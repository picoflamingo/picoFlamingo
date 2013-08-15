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

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <render_engine.h>

// Constants
#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480


// EGL variables

int 
pf_re_native_init (EGLNativeDisplayType *display_out, 
		   EGLNativeWindowType *window_out,
		   int w, int h)
{
  display_out = 0;
  window_out = 0;
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
