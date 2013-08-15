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

#ifndef PF_MESH_H
#define PF_MESH_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "shader.h"
#include "matrix.h"

typedef struct pf_mesh_t
{
  int      n_vertex;
  int      n_index;
  GLfloat  *vertex;
  GLuint   *index;
  GLfloat  *normal;
  GLfloat  *color;
  GLfloat  *texcoord;
  GLuint   vert_shader;
  GLuint   frag_shader;
  GLuint   program;      
} *PF_MESH;

#ifdef __cplusplus
extern "C" {
#endif

  PF_MESH pf_mesh_new (int n_vertex, int n_index);
  int     pf_mesh_free (PF_MESH m);
  int     pf_mesh_set_vertex (PF_MESH m, float *vertex);
  int     pf_mesh_set_zc_vertex (PF_MESH m, float *vertex);
  int     pf_mesh_set_index  (PF_MESH m, int   *index);
  int     pf_mesh_set_normal (PF_MESH m, float *normal);
  int     pf_mesh_set_texcoord (PF_MESH m, float *texcoord);
  int     pf_mesh_set_zc_texcoord (PF_MESH m, float *texcoord);
  int     pf_mesh_set_color  (PF_MESH m, float *color);
  int     pf_mesh_set_rel_alpha  (PF_MESH m, float alpha);
  int     pf_mesh_set_program (PF_MESH m, char *vertex, char *fragment);
  int     pf_mesh_get_program (PF_MESH m);
  int      pf_mesh_set_prg (PF_MESH m, int prg);

  float* pf_mesh_get_vertex (PF_MESH m);
  float* pf_mesh_get_texcoord (PF_MESH m);

  int pf_mesh_render (PF_MESH m, PF_MATRIX44 modelView);
  int pf_mesh_render2 (PF_MESH m, PF_MATRIX44 modelView, PF_MATRIX44 normal);

#ifdef __cplusplus
}
#endif

#endif
