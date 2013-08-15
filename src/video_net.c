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
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <unistd.h>

#include <pthread.h>

/* Network Specific */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <fcntl.h>

#include <math.h>
#include "log.h"

#include "video_net.h"


#define MAX_VIDEO_STREAMS 10
static VIDEO_STREAM _vstream[MAX_VIDEO_STREAMS];
static int          _n_vstreams = 0;

void*  _get_frames (void*);


int
video_net_init ()
{
  int i;
  for (i = 0; i < MAX_VIDEO_STREAMS; i++)
    _vstream[i] = NULL;

  _n_vstreams = 0;
  return 0;
}

VIDEO_STREAM
look4stream (char *name)
{
  int   i;
  for (i = 0; i < MAX_VIDEO_STREAMS; i++)
    {
      if (_vstream[i] &&strcasecmp (_vstream[i]->id, name) == 0) 
	return _vstream[i];
    }
  return NULL;
}

char *
get_stream_name (VIDEO_STREAM vs)
{
  if (vs) return vs->id;
  return NULL;
}

VIDEO_STREAM
connect_server (char *ip, int port)
{
  char               name[1024];
  struct sockaddr_in si_other;
  int                slen=sizeof(si_other);
  VIDEO_STREAM       vs;

  /* Look for video stream in current */
  snprintf (name, 1024, "%s@%d", ip, port);

  if ((vs = look4stream (name)) != NULL)
    {
      LOG ("Stream %s already intialised...\n", name);
      if (vs->connected)
	{
	  pthread_mutex_lock (&vs->mutex);
	  vs->ref++;
	  pthread_mutex_unlock (&vs->mutex);
	  return vs;
	}
      else
	goto connection;
    }

  /* Create video stream object */
  if ((vs = malloc (sizeof (struct video_stream_t))) == NULL)
    {
      fprintf (stderr, "Cannot allocate VIDEO_STREAM object\n");
      return NULL;
    }

  _vstream[_n_vstreams] = vs;
  _n_vstreams++;
  LOG ("%d video streams available\n", _n_vstreams);

  pthread_mutex_init (&vs->mutex, NULL);
  /* Initialise */
  vs->id = strdup (name);
  vs->_the_buffer = NULL;
  vs->_the_copy = NULL;
  vs->_the_len = 0;
  vs->_dirty_bit = 0;
  vs->connected = 0;
  vs->ref = 0;

 connection:  

  pthread_mutex_lock (&vs->mutex);
  if ((vs->s = socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
      perror ("socket:");
      pthread_mutex_unlock (&vs->mutex);
      exit (1);
    }


  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(port);

  if (inet_aton(ip, &si_other.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    pthread_mutex_unlock (&vs->mutex);
    exit(1);
  }


  LOG ("Connecting to server %s\n", vs->id);
  // FIXME: Timeouts and error check

  if ((connect (vs->s, (struct sockaddr*)&si_other, slen)) < 0)
    {
      perror ("connect:");
      vs->connected = 0;
      pthread_mutex_unlock (&vs->mutex);
      return NULL;
    }
  vs->connected = 1;
  vs->ref++;
  pthread_mutex_unlock (&vs->mutex);
  /* Create thread and mutex*/
  pthread_create (&vs->tid, NULL, _get_frames, vs);

  return vs;

}

#define DROP_FRAMES 1
static int drop = DROP_FRAMES;


/* Get Image */
unsigned char * 
get_live_frame (VIDEO_STREAM vs, int *len)
{
 pthread_mutex_lock (&vs->mutex);
 /* Make a copy*/
 if (!vs->_dirty_bit) 
   {
     pthread_mutex_unlock (&vs->mutex);
     *len = 0;
     return NULL;
     //goto unlock_and_return;
   }

 if (vs->_the_copy) free (vs->_the_copy);
 LOG ("%d: Frame requested. Current size: %d\n", time (NULL), vs->_the_len);
 vs->_the_copy = malloc (vs->_the_len);

 memcpy (vs->_the_copy, vs->_the_buffer, vs->_the_len);
 vs->_dirty_bit = 0;

 unlock_and_return:
 pthread_mutex_unlock (&vs->mutex);

 *len = vs->_the_len;
 return vs->_the_copy;
}


int
disconnect_server (VIDEO_STREAM vs)
{
 pthread_mutex_lock (&vs->mutex);
 if (vs->ref !=0)
   {
     vs->ref --;
     if (vs->ref <= 0)
       {
	 LOG ("No more references for stream %s. (%d)\n", 
	      vs->id, vs->ref);
	 vs->connected = vs->ref = 0;
	 // wait for thread to terminate
	 LOG ("Waiting for get_frame thread to finish)\n");
	 pthread_mutex_unlock (&vs->mutex);
	 pthread_join (vs->tid, NULL);
	 
	 close (vs->s);
	 return 0;
       }
   }
 /* XXX: Free buffers... when the object gets really destroyed*/
 pthread_mutex_unlock (&vs->mutex);

 return 0;
}

void* 
_get_frames (void*p)
{
  int                  size, bsize, cnt;
  int                  rsize, size1;
  unsigned char        *buffer;
  char                 c;
  VIDEO_STREAM         vs = (VIDEO_STREAM) p;
  int                  cont;
  
  drop = DROP_FRAMES;
  buffer = NULL;
  cont = 1;
  while (cont)
    {      
      /* Synchronise */
      bsize = 0;
      cnt ++;
      while (bsize != 8)
	{
	  read (vs->s, &c, 1);
	  if (c == 'X') bsize++; else bsize = 0;
	  cnt++;
	}
      
      read (vs->s, &size, sizeof(int));
      
      if (size == 0) return NULL;
      
      LOG ("%ld: New frame of size %d\n", time (NULL), size);
      
      buffer = malloc (size);
      
      rsize = 0;
      size1 = size;
      while (size1 > 0) 
	{
	  
	  bsize = read (vs->s, buffer + rsize, size1);
	  
	  if (bsize == -1) 
	    {
	      free (buffer);
	      return NULL;
	    }
	  rsize += bsize;
	  size1 -= bsize;
	}
	  pthread_mutex_lock (&vs->mutex);
	  if (vs->_the_buffer) free (vs->_the_buffer);
	  vs->_the_buffer = buffer;
	  vs->_the_len = size;
	  vs->_dirty_bit = 1;
	  cont = vs->ref;
	  pthread_mutex_unlock (&vs->mutex);


    }

  // Remove from list 
  return (char *)buffer;
}


