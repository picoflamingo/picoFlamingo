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


#ifndef PF_RENDER_ENGINE_H
#define PF_RENDER_ENGINE_H

#include "matrix.h"

#define PF_EV_NONE           0
#define PF_EV_MOUSE_CLICK    1
#define PF_EV_KEY            2
#define PF_EV_MOUSE_RELEASE  3

typedef int (*RENDER_SCENE_FUNC) (void);

#ifdef __cplusplus
extern "C" {
#endif

  int pf_re_init (int w, int h, int bpp);
  int pf_re_end ();
  int pf_re_render (RENDER_SCENE_FUNC func);


  int       pf_re_projection (PF_MATRIX proj);
  PF_MATRIX pf_re_get_projection ();

  PF_MATRIX pf_re_reset_modelview ();
  int       pf_re_set_modelview (PF_MATRIX proj);
  PF_MATRIX pf_re_get_modelview ();

  int    pf_re_light_position (float *p);
  float* pf_re_get_light_position ();
  int    pf_re_set_clear_color (float r, float g, float b, float a);

  void pf_re_events (int *type, int *x, int *y, int *key, int *button);
  int  pf_re_set_post_proc_shader (char *name);

  int  TestEGLError(const char* location);

#ifdef __cplusplus
}
#endif

#endif
