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

#include "bcm_host.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <render_engine.h>

// Constants
#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480


// EGL variables

  static EGL_DISPMANX_WINDOW_T nativewindow;

int 
pf_re_native_init (EGLNativeDisplayType *display_out, 
		   EGLNativeWindowType *window_out,
		   int w, int h)
{
bcm_host_init();

   DISPMANX_ELEMENT_HANDLE_T dispman_element;
   DISPMANX_DISPLAY_HANDLE_T dispman_display;
   DISPMANX_UPDATE_HANDLE_T dispman_update;
   VC_RECT_T dst_rect;
   VC_RECT_T src_rect;
int success;
int screen_width = 800, screen_height = 600;

#if 1
   // create an EGL window surface
   success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
   if ( success < 0 ) 	
	{
	fprintf (stderr, "Cannot get display size\n");
	exit (1);
}
#endif
   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = screen_width;
   dst_rect.height = screen_height;

   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = screen_width << 16;
   src_rect.height = screen_height << 16;

   dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   dispman_update = vc_dispmanx_update_start( 0 );

   dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

   nativewindow.element = dispman_element;
   nativewindow.width = screen_width;
   nativewindow.height = screen_height;
   vc_dispmanx_update_submit_sync( dispman_update );


  *display_out = EGL_DEFAULT_DISPLAY;
  *window_out = &nativewindow;
}


int
pf_re_native_end ()
{

  return 0;
}

void
pf_re_events (int *type, int *x, int *y, int *key, int *button)
{
	return;
}
