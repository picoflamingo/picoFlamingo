/* 
 * picoFlamingo: 3D Portable Presentation System
 * Copyright (c) 2010, 2011, 2013 David Martínez Oliveira
 *
 * This file is part of picoFlamingo
 *
 * picoFlamingo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * picoFlamingo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with picoFlamingo.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>

/* Unix Specific */
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

/* Network Specific */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/* Bluetooth */
#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH    31
#define PF_BLUETOOTH    AF_BLUETOOTH
#endif
#define BTPROTO_RFCOMM  3

typedef struct {
        uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

#define BDADDR_ANY   (&(bdaddr_t) {{0, 0, 0, 0, 0, 0}})

static inline void bacpy(bdaddr_t *dst, const bdaddr_t *src) {
  memcpy(dst, src, sizeof(bdaddr_t));
}

/* RFCOMM socket address */
struct sockaddr_rc {
  sa_family_t     rc_family;
  bdaddr_t        rc_bdaddr;
  uint8_t         rc_channel;
};

#include "log.h"

#include "pfs_files.h"
#include "pfs_cmd.h"

extern int    transition;
extern int    slide_show;
extern float  rotx, roty, rotz;

extern int      n_ri_list;
extern char     *pres_dir;
extern int      current_item;
extern char     *pf_identity;

extern int      tcp_port;
extern int      bt_channel;

static pthread_t tid;

static int tcp_s = -1;
static int bt_s = -1;

static int bt_enable = 1;
static int tcp_enable = 1;

typedef struct client_t
{
  int    s;
} CLIENT_DATA;

#define MAX_CLIENTS 32
static CLIENT_DATA cdata[MAX_CLIENTS];
int current_client;

static int
_add_client (int s)
{
  int  i;
  for (i = 0; i < MAX_CLIENTS; i++)
    if (cdata[i].s < 0 )
      {
	cdata[i].s = s;
	return i;
      }
  fprintf (stderr, "Cannot accept more conenctions\n");
  return -1;
}

static int
_remove_client (int s)
{
  int  i;
  for (i = 0; i < MAX_CLIENTS; i++)
    if (cdata[i].s == s )
      {
	cdata[i].s = -1;
	return i;
      }
  fprintf (stderr, "Cannot accept more conenctions\n");
  return -1;
}


#define IBUF_SIZE  4096
#define SBUF_SIZE  1024

int
net_printf (char *fmt, ...)
{
  char    buf[IBUF_SIZE];
  int     len;
  va_list arg;


  if (!fmt) return -1;
  va_start (arg, fmt);

  len = vsnprintf (buf, IBUF_SIZE, fmt, arg);
  if (len >= IBUF_SIZE)
    {
      fprintf (stderr, "%s: Output truncated!!!\n", __FUNCTION__);
      buf[IBUF_SIZE - 1] = 0;
    }

  if (current_client == -2)
    write (1, buf, len);
  else
    write (cdata[current_client].s, buf, len);

  va_end (arg);
  return len;
}

int
bc_printf (char *fmt, ...)
{
  char    buf[IBUF_SIZE];
  int     len;
  va_list arg;
  int     i;

  if (!fmt) return -1;
  va_start (arg, fmt);

  len = vsnprintf (buf, IBUF_SIZE, fmt, arg);
  if (len >= IBUF_SIZE)
    {
      fprintf (stderr, "%s: Output truncated!!!\n", __FUNCTION__);
      buf[IBUF_SIZE - 1] = 0;
    }
  write (1, buf, len);
  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (cdata[i].s < 0) continue;
      write (cdata[i].s, buf, len);
    }

  va_end (arg);
  return len;
}


int
broadcast_msg (char *msg)
{
  int  i;

  write (1, msg, strlen(msg));
  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (cdata[i].s < 0 ) continue;
      write (cdata[i].s, msg, strlen(msg));
    }
  return -1;

}

static int
_process_cmd (char *buffer)
{
  char *aux, id[SBUF_SIZE], name[SBUF_SIZE], temp[SBUF_SIZE];
  char *aux1;
  int  i, l;

  buffer[strlen(buffer) - 1] = 0;
  fprintf (stderr, "processing command '%s\n", buffer);

  /* Extract id*/
  memset(id, 0, SBUF_SIZE);
  memset(temp, 0, SBUF_SIZE);
  if ((aux = strchr (buffer, ' ')))
    {
      *aux = 0;
      strcpy (temp, buffer);
      *aux = 32;
    }
  else
    strcpy (temp, buffer);

  l = 0;
  /* parse command*/
  if ((aux1 = strchr (temp, ':')) != NULL)
    {
      /* We have received a named command... process name */
      /* Check name */
      *aux1 = 0;
      strcpy (name, temp);
      strcpy (id, aux1 + 1);
      fprintf (stderr, "Command '%s' to '%s'\n", id, name);
      l = strlen (name) + 1;
      /* Process name */
      if (name[0] == '!')
	{
	  // Negation ... message for everybody but us
	  if (strcasecmp (name, pf_identity) == 0)
	    return 0; // Message not for us
	}
      else // Direct message not for us
	if (strcasecmp (name, pf_identity) != 0)
	  return 0; // Message not for us


    }
  else
    strcpy (id, temp);

  i = pfs_cmd_find (id);
  LOG ("Looking for command '%s' index is %d\n", id, i);
  if (i < 0)
    {
      fprintf (stderr, "Unknown command %s\n", buffer);
      return 0;
    }
  LOG ("Runing command '%s' on buffer (%d)'%s'\n", id, l, buffer + l);
  pfs_cmd_run (i, buffer + l, NULL);

  return 0;
}

/* TODO: Add buffer and process multi-line messages */
static int
_read_line (int s, char *buffer, int len)
{
  char  c;
  int   i;

  /* read line */
  memset (buffer, 0, len);
  c = 0;
  i = 0;
  /* Read character by character until the multi-line buffer is in place*/
  while (c != '\n')
    {
      if ((read (s, &c, 1)) <=0)
	{
	  close (s);
	  _remove_client (s);
	  return 0;
	}
      buffer[i] = c;
      i++;
    }
  return 0;
}

static int
_accept_client (int sa)
{
  int                s, i;
  struct sockaddr_in client;
  socklen_t          sa_len = sizeof(struct sockaddr_in);

  s = accept (sa, (struct sockaddr*) &client, 
	      &sa_len);
  if ((i = _add_client (s)) < 0)
    {
      close (s);
    }
  printf ("Client %d added to list at position %d\n", s, i);
  return 0;
}

/* TODO: Add netkitty main function below */
void*
_process_input_thread (void *p)
{
  int            loop4ever = 1;
  fd_set         rfds;
  struct timeval tv;
  int            retval, max, s, i;
  char           buffer[SBUF_SIZE];


  while (loop4ever)
    {
      max = 0;
      /* Watch stdin (fd 0) to see when it has input. */
      FD_ZERO(&rfds);
      FD_SET(0, &rfds);
      
      /* Add bt and tcp */
      if (bt_enable)
	FD_SET(bt_s, &rfds);
      if (tcp_enable)
	FD_SET(tcp_s, &rfds);

      if (tcp_s > bt_s) max = tcp_s + 1; else max = bt_s + 1;

      /* Then add clients... */
      for (i = 0; i < MAX_CLIENTS; i++)
	{
	  if (cdata[i].s < 0) continue;

	  FD_SET(cdata[i].s, &rfds);
	  if (cdata[i].s > max) max = cdata[i].s + 1;
	}
      /* Wait up to five seconds. */
      tv.tv_sec = 5;
      tv.tv_usec = 0;
      max++;
      if ((retval = select(max, &rfds, NULL, NULL, &tv)) < 0)
	perror ("select():");
      if (retval)
	{
	  /* Check connections */
	  if (FD_ISSET(bt_s, &rfds))
	    {
	      /* Accept connection and add client to list*/
	      _accept_client (bt_s);
	    }
	  if (FD_ISSET(tcp_s, &rfds))
	    {
	      /* Accept connection and add client to list*/
	      _accept_client (tcp_s);
	    }
	  /* Check clients */

	  for (i = 0; i < MAX_CLIENTS; i++)
	    {
	      current_client = -1;
	      if (cdata[i].s < 0) continue;
	      if (FD_ISSET(cdata[i].s,&rfds))
		{
		  current_client = i;

		  _read_line (cdata[i].s, buffer, SBUF_SIZE);
		  _process_cmd (buffer);
		}
	    }
	  if (FD_ISSET(0, &rfds))
	    {
	      current_client = -2;
	      _read_line (0, buffer, SBUF_SIZE);
	      _process_cmd (buffer);
	    }
	}
    }

  return NULL;
}

static int
_create_sockets ()
{
  int                  i, ops = 1;
  struct sockaddr_in   server;
  struct sockaddr_rc   addr;
  void                 *g_addr;
  int                   g_len, family, proto, type;


  /* Init client data structure */
  for (i = 0; i < MAX_CLIENTS; i++) cdata[i].s = -1;

  fprintf (stderr, "+ Starting Bluetooth RFCOMM Server. Channel: %d\n", 
	   bt_channel);
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = bt_channel;
  g_addr = &addr;
  g_len = sizeof(struct sockaddr_rc);
  bacpy(&addr.rc_bdaddr, BDADDR_ANY);

  if ((bt_s = socket (AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
    {
      perror ("BT_socket:");
      bt_enable = 0;
    }
  else
    {

      setsockopt (bt_s, SOL_SOCKET, SO_REUSEADDR, &ops, sizeof(ops));
      if ((bind (bt_s, (struct sockaddr *) g_addr, g_len)) < 0)
	{
	  perror ("BT_bind:");
	  bt_enable = 0;
	  close (bt_s);
	  bt_s = -1;
	}
    }

  fprintf (stderr, "+ Starting TCP Server. Port: %d\n", 
	   tcp_port);

  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_family = AF_INET;
  server.sin_port = htons(tcp_port);
  g_addr = &server;
  g_len = sizeof(struct sockaddr_in);

  if ((tcp_s = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("TCP_socket:");
      tcp_enable = 0;
    }
  else
    {
      setsockopt (tcp_s, SOL_SOCKET, SO_REUSEADDR, &ops, sizeof(ops));
      if ((bind (tcp_s, (struct sockaddr *) g_addr, g_len)) < 0)
	{
	  tcp_enable = 0;
	  close (tcp_s);
	  tcp_s = -1;
	}
    }
  listen (bt_s, 10);
  listen (tcp_s, 10);
  printf ("BT_S: %d  TCP_S:%d\n", bt_s, tcp_s);
  return 0;
}

int 
init_user_input_thread ()
{
  /* Create network sockets and start thread */
  _create_sockets ();
  pthread_create (&tid, NULL, _process_input_thread, NULL);

  return 0;
}


