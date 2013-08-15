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

#ifndef PF_QUAD_H
#define PF_QUAD_H

#include "pf_render_item.h"

#ifdef __cplusplus
extern "C" {
#endif

  PF_RITEM pf_quad_new        (float w, float h);
  int      pf_quad_render     (PF_RITEM pfri);
  int      pf_quad_set_width  (PF_RITEM pfri, float w);
  int      pf_quad_set_height (PF_RITEM ri, float h);
  int      pf_quad_set_size   (PF_RITEM ri, float w, float h);
  int      pf_quad_get_size   (PF_RITEM ri, float *w, float *h);
  int      pf_quad_set_vertex (PF_RITEM ri, 
			       float x0, float y0, float x1, float y1);
  int      pf_quad_set_color  (PF_RITEM pfri, 
			       float r, float g, float b, float alpha);

#ifdef __cplusplus
}
#endif

#endif
