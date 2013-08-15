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
#ifndef PF_TEX_MNG
#define PF_TEX_MNG

#include <GLES2/gl2.h>

typedef struct pf_tex {
  char    *id;
  int     w, h;
  int     ref;
  int     mutable;
  GLuint  texid;
} PF_TEX;

#ifdef __cplusplus
extern "C" {
#endif

  int pf_tmng_init ();
  int pf_tmng_end ();
  int pf_tmng_add (char *name, int flag, int w, int h, char *data);
  int pf_tmng_add_from_file (char *name);
  int pf_tmng_update (char *name, int x, int y, int w, int h, char *data);
  int pf_tmng_unref (char *name);
  int pf_tmng_unref_texid (GLuint texid);
  int pf_tmng_get (char *name);
  int pf_tmng_get_size (char *name, int *w, int *h);

#ifdef __cplusplus
}
#endif

#endif
