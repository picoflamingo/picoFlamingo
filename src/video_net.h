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


#ifndef VIDEO_NET_H
#define VIDEO_NET_H

#include "pthread.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct video_stream_t
  {
    char              *id;
    pthread_mutex_t   mutex;
    pthread_t         tid;
    unsigned char     *_the_buffer; 
    int               _the_len; 
    int               _dirty_bit; 
    unsigned char     *_the_copy; 
    int               s; 
    int               connected;
    int               ref;
  } *VIDEO_STREAM;


  int video_net_init ();
  VIDEO_STREAM  connect_server (char *ip, int port);
  int           disconnect_server (VIDEO_STREAM vs);
  unsigned char *get_live_frame (VIDEO_STREAM vs, int *len);
#ifdef __cplusplus
  }
#endif

#endif
