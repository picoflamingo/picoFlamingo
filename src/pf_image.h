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

#ifndef PF_IMAGE_H
#define PF_IMAGE_H

#include "pf_render_item.h"

#ifdef __cplusplus
extern "C" {
#endif

  PF_RITEM pf_image_new (char *fname, float scale, int center);
  int      pf_image_render (PF_RITEM pfri);

  int      pf_image_set_data (PF_RITEM pfri, unsigned char *data, int len);
  int      pf_image_set_data_from_file (PF_RITEM ri, char *file);
  int      pf_image_set_color (PF_RITEM pfri, 
			       float r, float g, float b, float alpha);

  int      pf_image_reconnect (PF_RITEM pfri, char *ip_str, int port);
  int      pf_image_disconnect (PF_RITEM pfri);
  int      pf_image_save (PF_RITEM pfri);
  int      pf_image_save_as (PF_RITEM pfri, char *fname);
  int      pf_image_set_auto (PF_RITEM pfri, int flag);
#ifdef __cplusplus
}
#endif

#endif
