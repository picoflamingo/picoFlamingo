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

#ifndef PF_FONT_H
#define PF_FONT_H


#define PF_FONT_STYLE_NORMAL 0

typedef struct
{
  int   minx, maxx, miny, maxy, advance, base;
  int   tw, th;
  float texcoord[4];
  int   texid;
  char  *data;
} PF_GLYPH;

typedef struct
{
  int       style, pointsize, height, ascent, descent, lineskip;
  PF_GLYPH  glyphs[256];
} PF_FONT;

#ifdef __cplusplus
extern "C" {
#endif

  int      pf_font_mng_init ();
  int      pf_font_new (char *name, PF_FONT *font);
  PF_FONT* pf_get_font (char *name);

#ifdef __cplusplus
}
#endif

#endif
