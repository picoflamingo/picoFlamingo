/* 
 * picoFlamingo: 3D Portable Presentation System
 * Copyright (c) 2010, 2011, 2013 David Mart�nez Oliveira
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

#ifndef PFS_FILES_H
#define PFS_FILES_H

#ifdef __cplusplus
extern "C" {
#endif

  int clean_scene ();
  int load_slide (char *name, int indx);
  int include_slide (char *fname);

#ifdef __cplusplus
}
#endif

#endif
