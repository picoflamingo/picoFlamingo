/*
  Simple v4l2 jpeg streamer 
  (C) 2010 David Martínez Oliveira <dmo AT papermint-designs DOT com)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/* This is a modification of the V4L2 video capture example
 * 
 * The included modifications are:
 * - Uses the v4lconvert_yuyv_to_rgb24 from libv4l library
 * - Uses libjpeg to compress frames
 * - Adds code to accept TCP connections and stream frames
 *   using those connections 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <signal.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>


/* Network Specific */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <fcntl.h>


#include "jpeg.h"
#include <math.h>
#define CLEAR(x) memset (&(x), 0, sizeof (x))

static int width = 640;
static int height = 480;
static int img_w = 640;
static int img_h = 480;
/* Características de la imagen*/

static int img_bpp = 3;
static int img_size;

#define PX_FMT_MJPEG 0
#define PX_FMT_YUYV 1
#define PX_FMT_LAST 2

typedef void (*PROC_IMG)(const void*, int);

void process_image1 (const void *, int);
void process_image2 (const void *, int);

PROC_IMG proc_img[PX_FMT_LAST] = {process_image2, process_image1};

static int px_fmt = 0;

typedef enum 
  {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
  } io_method;

struct buffer 
{
  void *                  start;
  size_t                  length;
};


static char             dev_name[1024];
static io_method        io              = IO_METHOD_MMAP;
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;


#define MAX_DISP 10
#define PUERTO   4000

#define BUF_SIZE 4096
#define MAX_CLIENTS 32


/* Contador para volcar imagenes*/
static int   cnt = 0;
static char *jp_buf = NULL;
static int   jp_size = 0;

static int   n_clients = 0;
static int   client[MAX_CLIENTS];


static int 
crea_socket (int puerto)
{
  int                  s, ops;
  struct sockaddr_in   server;
  int                  sa_len = sizeof(struct sockaddr_in);

  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_family = AF_INET;
  server.sin_port = htons(puerto);
  s = socket (PF_INET, SOCK_STREAM, 0);

  ops = 1;
  setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &ops, sizeof(ops));
  bind (s, (struct sockaddr *) &server, sa_len);
  listen (s, 10);

  return s;
}

static int
add_client (int s)
{
  int   i;

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (client[i] == -1) 
	{
	  client[i] = s;
	  n_clients++;
	  break;
	}
    }
  return 0;
}


static void
errno_exit (const char *s)
{
  fprintf (stderr, "%s error %d, %s\n",
	   s, errno, strerror (errno));
  
  exit (EXIT_FAILURE);
}

static int
xioctl (int  fd, int request, void *arg)
{
  int r;
  
  do r = ioctl (fd, request, arg);
  while (-1 == r && EINTR == errno);
  
  return r;
}

#define CLIP(color) (unsigned char)(((color)>0xFF)?0xff:(((color)<0)?0:(color)))

static void 
v4lconvert_yuyv_to_rgb24 (const unsigned char *src, unsigned char *dest,
			 int width, int height)
{
  int j;
  
  while (--height >= 0) 
    {
      for (j = 0; j < width; j += 2) 
	{
	  int u = src[1];
	  int v = src[3];
	  int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
	  int rg = (((u - 128) << 1) +  (u - 128) +
		    ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
	  int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;
	  
	  *dest++ = CLIP(src[0] + v1);
	  *dest++ = CLIP(src[0] - rg);
	  *dest++ = CLIP(src[0] + u1);
	  
	  *dest++ = CLIP(src[2] + v1);
	  *dest++ = CLIP(src[2] - rg);
	  *dest++ = CLIP(src[2] + u1);
	  src += 4;
	}
    }
}

//#define DUMP_FILE 1


void
process_image2 (const void *p1, int len)
{
  FILE *f;
  unsigned char *buf;

  if (jp_buf) free (jp_buf);
  jp_buf = malloc (len);
  memcpy (jp_buf, p1, len);
  jp_size = len;

#ifdef DUMP_FILE
  char fname[1024];
  snprintf (fname, 1024, "img-%04d.jpeg", cnt);
  if ((f = fopen (fname, "wb")))
    {
      fwrite (p1, len, 1, f);
      fclose (f);
    }
#endif
  cnt++;

  return;
}

void
process_image1 (const void *p1, int len)
{
  unsigned char *buf;

  buf = malloc (width * height * 3);
  v4lconvert_yuyv_to_rgb24 (p1, buf, width,height);

  jp_size = 0;
  if (jp_buf) free (jp_buf);
  jp_buf = 0;
  
  jp_size = write_jpeg_mem (&jp_buf, buf, img_w, img_h, img_bpp);
  // Uncomment to dump images into a file
#ifdef DUMP_FILE
  char fname[1024];
  snprintf (fname, 1024, "img-%04d.jpeg", cnt);
  write_jpeg_file( fname, buf, width, height, 3);
#endif
  free (buf);

  cnt++;
}




static int
read_frame (void)
{
  struct v4l2_buffer buf;
  unsigned int i;
  
  switch (io) 
    {
    case IO_METHOD_READ:
      if (-1 == read (fd, buffers[0].start, buffers[0].length)) 
	{
	  switch (errno) 
	    {
	    case EAGAIN:
	      return 0;
	      
	    case EIO:
	      /* Could ignore EIO, see spec. */
	      
	      /* fall through */
	      
	    default:
	      errno_exit ("read");
	    }
	}
      
#if 0
      if (px_fmt == PX_FMT_MJPEG)
	process_image2 (buffers[0].start, buffers[0].length);
      else
	process_image1 (buffers[0].start, buffers[0].length);
#endif
      proc_img[px_fmt] (buffers[0].start, buffers[0].length);
      break;
      
    case IO_METHOD_MMAP:
      CLEAR (buf);
      
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      
      if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) 
	{
	  switch (errno) 
	    {
	    case EAGAIN:
	      return 0;
	      
	    case EIO:
	      /* Could ignore EIO, see spec. */
	      
	      /* fall through */
	      
	    default:
	      errno_exit ("VIDIOC_DQBUF");
	    }
	}

      assert (buf.index < n_buffers);
#if 0      
      if (px_fmt == PX_FMT_MJPEG)
	process_image2 (buffers[buf.index].start, buffers[buf.index].length);
      else
	process_image1 (buffers[buf.index].start, buffers[buf.index].length);
#endif 
	proc_img[px_fmt] (buffers[buf.index].start, buffers[buf.index].length);
      if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
	errno_exit ("VIDIOC_QBUF");
      
      break;
      
    case IO_METHOD_USERPTR:
      CLEAR (buf);
      
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;
      
      if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) 
	{
	  switch (errno) 
	    {
	    case EAGAIN:
	      return 0;
	      
	    case EIO:
	      /* Could ignore EIO, see spec. */
	      
	      /* fall through */
	      
	    default:
	      errno_exit ("VIDIOC_DQBUF");
	    }
	}
      
      for (i = 0; i < n_buffers; ++i)
	if (buf.m.userptr == (unsigned long) buffers[i].start
	    && buf.length == buffers[i].length)
	  break;
      
      assert (i < n_buffers);
      
      //process_image ((void *) buf.m.userptr);
#if 0
      if (px_fmt == PX_FMT_MJPEG)
	process_image2 (buffers[0].start, buffers[0].length);      
      else
	process_image1 (buffers[0].start, buffers[0].length);      
#endif
      proc_img[px_fmt] (buffers[0].start, buffers[0].length);      

      if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
	errno_exit ("VIDIOC_QBUF");
      
      break;
    }
  
  return 1;
}


static void
stop_capturing (void)
{
  enum v4l2_buf_type type;

  switch (io) 
    {
    case IO_METHOD_READ:
      /* Nothing to do. */
      break;
      
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      
      if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
	errno_exit ("VIDIOC_STREAMOFF");
      
      break;
    }
}

static void
start_capturing (void)
{
  unsigned int i;
  enum v4l2_buf_type type;
  
  switch (io) 
    {
    case IO_METHOD_READ:
      /* Nothing to do. */
      break;
      
    case IO_METHOD_MMAP:
      for (i = 0; i < n_buffers; ++i) 
	{
	  struct v4l2_buffer buf;
	  
	  CLEAR (buf);
	  
	  buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	  buf.memory      = V4L2_MEMORY_MMAP;
	  buf.index       = i;
	  
	  if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
	    errno_exit ("VIDIOC_QBUF");
	}
      
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      
      if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
	errno_exit ("VIDIOC_STREAMON");
      
      break;
      
    case IO_METHOD_USERPTR:
      for (i = 0; i < n_buffers; ++i) 
	{
	  struct v4l2_buffer buf;
	  
	  CLEAR (buf);
	  
	  buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	  buf.memory      = V4L2_MEMORY_USERPTR;
	  buf.index       = i;
	  buf.m.userptr   = (unsigned long) buffers[i].start;
	  buf.length      = buffers[i].length;
	  
	  if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
	    errno_exit ("VIDIOC_QBUF");
	}
      
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      
      if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
	errno_exit ("VIDIOC_STREAMON");
      
      break;
    }
}

static void
uninit_device (void)
{
  unsigned int i;
  
  switch (io) 
    {
    case IO_METHOD_READ:
      free (buffers[0].start);
      break;
      
    case IO_METHOD_MMAP:
      for (i = 0; i < n_buffers; ++i)
	if (-1 == munmap (buffers[i].start, buffers[i].length))
	  errno_exit ("munmap");
      break;
      
    case IO_METHOD_USERPTR:
      for (i = 0; i < n_buffers; ++i)
	free (buffers[i].start);
      break;
    }
  
  free (buffers);
}

static void
init_read (unsigned int buffer_size)
{
  buffers = calloc (1, sizeof (*buffers));
  
  if (!buffers) 
    {
      fprintf (stderr, "Out of memory\n");
      exit (EXIT_FAILURE);
    }
  
  buffers[0].length = buffer_size;
  buffers[0].start = malloc (buffer_size);
  
  if (!buffers[0].start) 
    {
      fprintf (stderr, "Out of memory\n");
      exit (EXIT_FAILURE);
    }
}

static void
init_mmap (void)
{
  struct v4l2_requestbuffers req;
  
  CLEAR (req);
  
  req.count               = 4;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_MMAP;
  
  if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) 
    {
      if (EINVAL == errno) 
	{
	  fprintf (stderr, "%s does not support "
		   "memory mapping\n", dev_name);
	  exit (EXIT_FAILURE);
	} 
      else 
	{
	  errno_exit ("VIDIOC_REQBUFS");
	}
    }
  
  if (req.count < 2) 
    {
      fprintf (stderr, "Insufficient buffer memory on %s\n",
	       dev_name);
      exit (EXIT_FAILURE);
    }
  
  buffers = calloc (req.count, sizeof (*buffers));
  
  if (!buffers) 
    {
      fprintf (stderr, "Out of memory\n");
      exit (EXIT_FAILURE);
    }
  
  for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
    {
      struct v4l2_buffer buf;
      
      CLEAR (buf);
      
      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = n_buffers;
      
      if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
	errno_exit ("VIDIOC_QUERYBUF");
      
      buffers[n_buffers].length = buf.length;
      buffers[n_buffers].start =
	mmap (NULL /* start anywhere */,
	      buf.length,
	      PROT_READ | PROT_WRITE /* required */,
	      MAP_SHARED /* recommended */,
	      fd, buf.m.offset);
      
      if (MAP_FAILED == buffers[n_buffers].start)
	errno_exit ("mmap");
    }
}

static void
init_userp (unsigned int buffer_size)
{
  struct v4l2_requestbuffers req;
  unsigned int page_size;
  
  page_size = getpagesize ();
  buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);
  
  CLEAR (req);
  
  req.count               = 4;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_USERPTR;
  
  if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) 
    {
      if (EINVAL == errno) 
	{
	  fprintf (stderr, "%s does not support "
		   "user pointer i/o\n", dev_name);
	  exit (EXIT_FAILURE);
	} 
      else 
	{
	  errno_exit ("VIDIOC_REQBUFS");
	}
    }
  
  buffers = calloc (4, sizeof (*buffers));
  
  if (!buffers) 
    {
      fprintf (stderr, "Out of memory\n");
      exit (EXIT_FAILURE);
    }
  
  for (n_buffers = 0; n_buffers < 4; ++n_buffers) 
    {
      buffers[n_buffers].length = buffer_size;
      buffers[n_buffers].start = memalign (/* boundary */ page_size,
					   buffer_size);
      
      if (!buffers[n_buffers].start) 
	{
	  fprintf (stderr, "Out of memory\n");
	  exit (EXIT_FAILURE);
	}
    }
}

static void
init_device (void)
{
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;
  
  if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) 
    {
      if (EINVAL == errno) 
	{
	  fprintf (stderr, "%s is no V4L2 device\n",
		   dev_name);
	  exit (EXIT_FAILURE);
	} 
      else 
	{
	  errno_exit ("VIDIOC_QUERYCAP");
	}
    }
  
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
    {
      fprintf (stderr, "%s is no video capture device\n",
	       dev_name);
      exit (EXIT_FAILURE);
    }

  switch (io) 
    {
    case IO_METHOD_READ:
      if (!(cap.capabilities & V4L2_CAP_READWRITE)) 
	{
	  fprintf (stderr, "%s does not support read i/o\n",
		   dev_name);
	  exit (EXIT_FAILURE);
	}
      
      break;
      
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
      if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
	{
	  fprintf (stderr, "%s does not support streaming i/o\n",
		   dev_name);
	  exit (EXIT_FAILURE);
	}
      
      break;
    }
  
  
  /* Select video input, video standard and tune here. */
  
  
  CLEAR (cropcap);
  
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) 
    {
      crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      crop.c = cropcap.defrect; /* reset to default */
      
      if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) 
	{
	  switch (errno) 
	    {
	    case EINVAL:
	      /* Cropping not supported. */
	      break;
	    default:
	      /* Errors ignored. */
	      break;
	    }
	}
    } 
  else 
    {        
      /* Errors ignored. */
    }
  
  
  CLEAR (fmt);
  
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = width; 
  fmt.fmt.pix.height      = height;
  //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
  //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  if (px_fmt == PX_FMT_MJPEG)
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  else
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  //fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
  
  if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
    errno_exit ("VIDIOC_S_FMT");
  
  /* Note VIDIOC_S_FMT may change width and height. */
  
  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;
  
  switch (io) 
    {
    case IO_METHOD_READ:
      init_read (fmt.fmt.pix.sizeimage);
      break;
      
    case IO_METHOD_MMAP:
      init_mmap ();
      break;
      
    case IO_METHOD_USERPTR:
      init_userp (fmt.fmt.pix.sizeimage);
      break;
    }
}

static void
close_device (void)
{
  if (-1 == close (fd))
    errno_exit ("close");

  fd = -1;
}

static void
open_device (void)
{
  struct stat st; 
  
  if (-1 == stat (dev_name, &st)) 
    {
      fprintf (stderr, "Cannot identify '%s': %d, %s\n",
	       dev_name, errno, strerror (errno));
      exit (EXIT_FAILURE);
    }
  
  if (!S_ISCHR (st.st_mode)) 
    {
      fprintf (stderr, "%s is no device\n", dev_name);
      exit (EXIT_FAILURE);
    }
  
  fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
  
  if (-1 == fd) 
    {
      fprintf (stderr, "Cannot open '%s': %d, %s\n",
	       dev_name, errno, strerror (errno));
      exit (EXIT_FAILURE);
    }
}

static void
usage (FILE  *fp, int argc, char **argv)
{
  fprintf (fp,
	   "Usage: %s [options]\n\n"
	   "Options:\n"
	   "-d | --device name   Video device name [/dev/video]\n"
	   "-i | --info          Print this message\n"
	   "-w | --width w       Image width\n"
	   "-h | --height h      Image height\n"
	   "-p | --port h        TCP port\n"
	   "-f | --format n      Format 0:MJPEG 1:YUYV\n"
	   "-m | --mmap          Use memory mapped buffers\n"
	   "-r | --read          Use read() calls\n"
	   "-u | --userp         Use application allocated buffers\n"
	   "",
	   argv[0]);
}

static const char short_options [] = "d:w:h:p:f:imru";

static const struct option
long_options [] = 
  {
    { "device",     required_argument,      NULL,           'd' },
    { "width",      required_argument,      NULL,           'w' },
    { "height",     required_argument,      NULL,           'h' },
    { "port",       required_argument,      NULL,           'p' },
    { "format",     required_argument,      NULL,           'f' },
    { "info",       no_argument,            NULL,           'i' },
    { "mmap",       no_argument,            NULL,           'm' },
    { "read",       no_argument,            NULL,           'r' },
    { "userp",      no_argument,            NULL,           'u' },
    { 0, 0, 0, 0 }
  };



int
main   (int argc, char **argv)
{
  fd_set              rfds;
  int                 max, n_res, s1;
  struct timeval      tv;
  int                 s, i, fin = 0;
  struct sockaddr_in  cliente;
  socklen_t           sa_len = sizeof(struct sockaddr_in);
  int                 port = 4000;
  
  printf ("Simplistic v4l2 network streamer!!!\n");
  signal (SIGPIPE, SIG_IGN);
  strcpy (dev_name, "/dev/video0");

  printf ("Processing Arguments...\n");
  for (;;) 
    {
      int index;
      int c;
      
      c = getopt_long (argc, argv, short_options, long_options, &index);
    
    if (-1 == c)
      break;
    
    switch (c) 
      {
      case 0: /* getopt_long() flag */
	break;
	
      case 'd':
	strcpy (dev_name, optarg);
	break;

      case 'w':
	width = atoi (optarg);
	break;

      case 'h':
	height = atoi (optarg);
	break;

      case 'p':
	port = atoi (optarg);
	break;

      case 'f':
	px_fmt = atoi (optarg);
	break;

      case 'i':
	usage (stdout, argc, argv);
	exit (EXIT_SUCCESS);
	
      case 'm':
	io = IO_METHOD_MMAP;
	break;
	
      case 'r':
	io = IO_METHOD_READ;
	break;
	
      case 'u':
	io = IO_METHOD_USERPTR;
	break;
	
      default:
	usage (stderr, argc, argv);
	exit (EXIT_FAILURE);
      }
    }
  
  img_w = width;  
  img_h = height;

  printf ("Opening and initialising device\n");

  open_device ();
  init_device ();

  /* Start capturing*/
  start_capturing ();
  
  img_bpp = 3;
  img_size = img_w * img_h * img_bpp;
  printf ("Creating socket...\n");
  s = crea_socket (port);

  n_clients = 0;
  for (i = 0; i < MAX_CLIENTS; i++) client[i] = -1;
  max = -1;

  while (!fin)
    {
      FD_ZERO(&rfds);

      FD_SET (s, &rfds);
      max = s + 1;

      tv.tv_sec = 0;
      tv.tv_usec = 10000;

      if ((n_res = select (max, &rfds, NULL, NULL, &tv)) < 0)
	perror ("select:");
      else
	{
	  if (FD_ISSET (s, &rfds))
	    {
	      s1 =  accept (s, (struct sockaddr*) &cliente, &sa_len);
	      printf ("New Connection\n");
	      add_client (s1);
	      printf ("%d clients \n", n_clients);
	    }
	  
	  if (read_frame () != 1) continue;

	  for (i = 0; i < MAX_CLIENTS;i++)
	    {
	      if (client[i] == -1) continue;
	      if (send (client[i], "XXXXXXXX", 8, 0) < 0)
		{
		  perror ("send:");
		  client[i] = -1;
		  n_clients--;
		  printf ("%d clients \n", n_clients);
		}
	      send (client[i], &jp_size, sizeof(int), 0);
	      send (client[i], jp_buf, jp_size, 0);
	    }
	  //usleep (20000);
	}
    }

  stop_capturing ();  
  uninit_device ();
  close_device ();
  
  exit (EXIT_SUCCESS);
  
  return 0;
}
