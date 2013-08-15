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

#ifndef PF_GROUP_H
#define PF_GROUP_H

#include "pf_render_item.h"

#ifdef __cplusplus
extern "C" {
#endif


  PF_RITEM pf_group_new        ();
  int      pf_group_render     (PF_RITEM pfri);
  int      pf_group_add_child  (PF_RITEM pfri, PF_RITEM child);

#ifdef __cplusplus
}
#endif

#endif
