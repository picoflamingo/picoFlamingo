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


#ifndef PF_STEX_H
#define PF_STEX_H
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "pf_render_item.h"

typedef struct line_render_hint_t
{
  GLuint texid[1];
  float  vertex[12]; // 4 vertex (3 components) for a quad
  int    n_sublines;
  int    flag;
  float  *y_sublines;
} *LRH;


#ifdef __cplusplus
extern "C" {
#endif

  PF_RITEM pf_stext_new (char *font, char *text);
  int      pf_stext_render (PF_RITEM pfri);

  int      pf_stext_set_scale (PF_RITEM pfri, float scale_x, float scale_y);
  int      pf_stext_set_font (PF_RITEM pfri, char *font);
  int      pf_stext_set_color (PF_RITEM pfri, float r, float g, float b, float alpha);
  int      pf_stext_get_color (PF_RITEM pfri, float* r, float* g, float* b, float *alpha);
  int      pf_stext_add_text_line (PF_RITEM pfri, char *text);
  int      pf_stext_set_interline (PF_RITEM pfri, float il);
  int      pf_stext_set_width (PF_RITEM pfri, float w);

  int      pf_stex_set_current_line (PF_RITEM pfri, int cl);
  int      pf_stext_set_lines_up (PF_RITEM pfri, int cl);
  int      pf_stext_set_lines_down (PF_RITEM pfri, int cl);
  int      pf_stext_set_para (PF_RITEM pfri, float i1, float i2, float p);
  int      pf_stext_set_hl (PF_RITEM pfri, int flag);
  int      pf_stext_set_hl_color (PF_RITEM pfri, float r, float g, float b, float alpha);

  
  int      pf_stext_clear (PF_RITEM pfri);
  int      pf_stext_get_params (PF_RITEM pfri, float* scale_x, float *scale_y,
			       float *interline, float *width);

  int      pf_stext_add_lrh (PF_RITEM pfri);
  LRH lrh_new ();
  int lrh_free (LRH l);

#ifdef __cplusplus
}
#endif

#endif
