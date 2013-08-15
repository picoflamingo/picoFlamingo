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


#ifndef PF_TEXT_H
#define PF_TEXT_H

#include "pf_render_item.h"

#ifdef __cplusplus
extern "C" {
#endif

  PF_RITEM pf_text_new (char *font, char *text);
  int      pf_text_render (PF_RITEM pfri);

  int      pf_text_set_scale (PF_RITEM pfri, float scale_x, float scale_y);
  int      pf_text_set_font (PF_RITEM pfri, char *font);
  int      pf_text_set_color (PF_RITEM pfri, float r, float g, float b, float alpha);
  int      pf_text_get_color (PF_RITEM pfri, float* r, float* g, float* b, float *alpha);
  int      pf_text_add_text_line (PF_RITEM pfri, char *text);
  int      pf_text_set_interline (PF_RITEM pfri, float il);
  int      pf_text_set_width (PF_RITEM pfri, float w);

  int      pf_text_set_current_line (PF_RITEM pfri, int cl);
  int      pf_text_set_lines_up (PF_RITEM pfri, int cl);
  int      pf_text_set_lines_down (PF_RITEM pfri, int cl);
  int      pf_text_set_para (PF_RITEM pfri, float i1, float i2, float p);
  int      pf_text_set_hl (PF_RITEM pfri, int flag);
  
  int      pf_text_clear (PF_RITEM pfri);
  int      pf_text_get_params (PF_RITEM pfri, float* scale_x, float *scale_y,
			       float *interline, float *width);


#ifdef __cplusplus
}
#endif

#endif
