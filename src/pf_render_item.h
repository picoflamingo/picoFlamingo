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

#ifndef PF_RENDER_ITEM_H
#define PF_RENDER_ITEM_H

#include <stdio.h>
#include "matrix.h"
#include "pf_mesh.h"

#define PFRI_DEFAULT  0
#define PFRI_IMAGE    1
#define PFRI_TEXT     2
#define PFRI_3D	      3
#define PFRI_QUAD     4
#define PFRI_STEXT    5
#define PFRI_GROUP    6

struct pf_ritem_t; /* Forward declaration */
typedef int (*PF_FUNC)(struct pf_ritem_t*);

typedef struct pf_ritem_t
{
  int           id;
  int           n_meshes;
  PF_MESH       *mesh;
  PF_MATRIX44   original_trans;
  PF_MATRIX44   current_trans;
  int           dirty;
  float         x, y, z;
  float         a, b, c;
  float         sx, sy, sz;
  int           program;
  int           depth;
  void          *priv;
  char          *name;
  int           show;
  int           dirty_bound;
  int           clickable;
  int           group;
  float         bb2D[4];
  PF_FUNC       render;
  PF_FUNC       free;
  PF_FUNC       auto_update;
  PF_FUNC       save;

} *PF_RITEM;

#ifdef __cplusplus
extern "C" {
#endif

  PF_RITEM pf_ritem_new ();
  int      pf_ritem_free (PF_RITEM pfri);

  int      pf_ritem_set_mesh (PF_RITEM pfri, PF_MESH m);
  int      pf_ritem_add_mesh (PF_RITEM pfri, PF_MESH m);
  PF_MESH  pf_ritem_get_mesh (PF_RITEM pfri);
  int      pf_ritem_set_pos (PF_RITEM pfri, float x, float y, float z);
  int      pf_ritem_set_rot (PF_RITEM pfri, float a, float y, float z);
  int      pf_ritem_set_render_func (PF_RITEM pfri, PF_FUNC func);
  int      pf_ritem_set_free_func (PF_RITEM pfri, PF_FUNC func);
  int      pf_ritem_set_save_func (PF_RITEM pfri, PF_FUNC func);
  int      pf_ritem_set_prg (PF_RITEM pfri, int program);

  int      pf_ritem_set_name (PF_RITEM pfri, char *name);
  char*    pf_ritem_get_name (PF_RITEM pfri);
  int      pf_ritem_show (PF_RITEM pfri, int flag);


  int      pf_ritem_set_priv_data (PF_RITEM pfri, void *data);
  void*    pf_ritem_get_priv_data (PF_RITEM pfri);

  int      pf_ritem_set_group (PF_RITEM pfri, int indx);
  int      pf_ritem_get_group (PF_RITEM pfri);


  int      pf_ritem_render (PF_RITEM pfri);

  int      pf_ritem_set_abs_rot (PF_RITEM pfri, float a, float b, float c);
  int      pf_ritem_get_abs_rot (PF_RITEM pfri, float *a, float *b, float *c);
  int      pf_ritem_set_abs_pos (PF_RITEM pfri, float a, float b, float c);
  int      pf_ritem_get_abs_pos (PF_RITEM pfri, float *a, float *b, float *c);
  int      pf_ritem_set_color (PF_RITEM pfri, 
			       float r, float g, float b, float alpha);
  int      pf_ritem_get_color (PF_RITEM ri, 
			       float *r, float *g, float *b, float *a);

  int      pf_ritem_set_depth_test (PF_RITEM pfri, int flag);
  int      pf_ritem_get_depth_test (PF_RITEM pfri);


  int      pf_ritem_calculate_bb (PF_RITEM pfri, PF_MATRIX44 modelview);
  int      pf_ritem_test_bb (PF_RITEM pfri, float x, float y);
#ifdef __cplusplus
}
#endif

#endif
