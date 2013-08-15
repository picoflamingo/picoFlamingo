/* 
 * picoFlamingo: 3D Portable Presentation System
 * Copyright (c) 2010, 2011 David Martínez Oliveira
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

#ifndef RENDER_ENGINE_SYS_H
#define RENDER_ENGINE_SYS_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>


int pf_re_native_init (EGLNativeDisplayType *x11Display_out, 
		       EGLNativeWindowType *x11Window_out,
		       int w, int h);
int pf_re_native_end ();

#endif
