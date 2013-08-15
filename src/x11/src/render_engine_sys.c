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

#include "X11/Xlib.h"
#include "X11/Xutil.h"

#include "X11/Xatom.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <render_engine.h>

// Constants
#if 1
#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480
#else

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768
#endif
static  Window				x11Window	= 0;
static  Display*			x11Display	= 0;
static  Colormap			x11Colormap	= 0;

typedef struct
{
  unsigned long   flags;
  unsigned long   functions;
  unsigned long   decorations;
  long            inputMode;
  unsigned long   status;
} Hints;

int 
pf_re_native_init (EGLNativeDisplayType *X11Display_out, 
		   EGLNativeWindowType *X11Window_out, 
		   int w, int h)
{
  long	                x11Screen = 0;
  XVisualInfo*	        x11Visual = 0;  
  Hints                 hints;
  Atom                  property;
  /* Create XWindow window  */
  Window                sRootWindow;
  XSetWindowAttributes  win_attr;
  unsigned int          win_mask;
  int                   color_depth;

  // Initializes the display and screen
  x11Display = XOpenDisplay( 0 );
  if (!x11Display)
    {
      printf("Error: Unable to open X display\n");
      goto cleanup;
    }
  x11Screen = XDefaultScreen( x11Display );
  
  // Gets the window parameters
  sRootWindow = RootWindow(x11Display, x11Screen);
  color_depth = DefaultDepth(x11Display, x11Screen);
  x11Visual = malloc (sizeof(XVisualInfo));
  XMatchVisualInfo( x11Display, x11Screen, color_depth, TrueColor, x11Visual);
  if (!x11Visual)
    {
      printf("Error: Unable to acquire visual\n");
      goto cleanup;
    }
  x11Colormap = XCreateColormap( x11Display, sRootWindow, x11Visual->visual, AllocNone );
  win_attr.colormap = x11Colormap;
  
  // Configure Event Mask
  win_attr.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | ButtonMotionMask | PointerMotionMask;

  win_mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;
  
  x11Window = XCreateWindow( x11Display, RootWindow(x11Display, x11Screen), 
			     0, 0, w, h,
			     0, CopyFromParent, InputOutput, 
			     CopyFromParent, win_mask, &win_attr);

  /* Make window borderless */
  /* ------------------------------------------- */
  hints.flags = 2;        // Specify that we're changing the window decorations.
  hints.decorations = 0;  // 0 (false) means that window decorations should go bye-bye.

  property = XInternAtom (x11Display,"_MOTIF_WM_HINTS",True);

  XChangeProperty (x11Display, x11Window, property, property, 32,
		   PropModeReplace, (unsigned char *) &hints, 5);

  /* ------------------------------------------- */

  XMapWindow(x11Display, x11Window);
  XFlush(x11Display);

  *X11Display_out = (EGLNativeDisplayType) x11Display;
  *X11Window_out = (EGLNativeWindowType) x11Window;

  
  return 0;

 cleanup:
  if (x11Window) XDestroyWindow(x11Display, x11Window);
  if (x11Colormap) XFreeColormap( x11Display, x11Colormap );
  if (x11Display) XCloseDisplay(x11Display);
  
  return -1;
}

int
pf_re_native_end ()
{
  if (x11Window) XDestroyWindow(x11Display, x11Window);
  if (x11Colormap) XFreeColormap( x11Display, x11Colormap );
  if (x11Display) XCloseDisplay(x11Display);

  return 0;
}



void pf_re_events (int *type, int *x, int *y, int *key, int *button)
{
  int     numMessages, i;
  XEvent  event;
  
  *type = PF_EV_NONE;

  // Are there messages waiting, maybe this should be a while loop
  numMessages = XPending( x11Display );
  for(i = 0; i < numMessages; i++ )
    {
      XNextEvent( x11Display, &event );
      
      switch( event.type )
	{
	 
	case MotionNotify:
	  {
	    XMotionEvent *bevent = ((XMotionEvent *) &event);
	    *type = PF_EV_MOUSE_CLICK;
	    *x = bevent->x;
	    *y = bevent->y;
	    *button = (bevent->state >> 8);
	    break;
	  }
	  // exit on mouse click
	case ButtonRelease:
	  {
	    XButtonEvent *bevent = ((XButtonEvent *) &event);
	    *type = PF_EV_MOUSE_RELEASE;
	    *x = bevent->x;
	    *y = bevent->y;
	    *button = bevent->button;
	    break;
	  }

	case ButtonPress:
	  {
	    XButtonEvent *bevent = ((XButtonEvent *) &event);
	    *type = PF_EV_MOUSE_CLICK;
	    *x = bevent->x;
	    *y = bevent->y;
	    *button = bevent->button;
	    break;
	  }
	  
	case MapNotify:
	case UnmapNotify:
	  break;
	  
	case KeyPress:
	  {
	    XKeyEvent *key_event = ((XKeyEvent *) &event);
	    *type = PF_EV_KEY;
	    *key = key_event->keycode;
	  }

	  break;
	  
	  
	default:
	  break;
	}
    }
}


