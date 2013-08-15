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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "render_engine.h"
#include "shader.h"
#include "matrix.h"
#include "pf_mesh.h"
#include "pf_render_item.h"

#define VERTEX_ARRAY    0
#define COLOR_ARRAY     1


static PF_MESH     cube_mesh;

#define alpha 1.0

GLfloat pfColors[] = 
  {
    1.0f, 0.0f, 0.0f, alpha,  
    1.0f, 0.0f, 0.0f, alpha,  
    1.0f, 0.0f, 0.0f, alpha,
    1.0f, 0.0f, 0.0f, alpha,
    
    1.0f, 1.0f, 0.0f, alpha,  
    1.0f, 1.0f, 0.0f, alpha,  
    1.0f, 1.0f, 0.0f, alpha,
    1.0f, 1.0f, 0.0f, alpha,
    
    0.0f, 1.0f, 0.0f, alpha,  
    0.0f, 1.0f, 0.0f, alpha,  
    0.0f, 1.0f, 0.0f, alpha,
    0.0f, 1.0f, 0.0f, alpha,
    
    
    1.0f, 0.0f, 1.0f, alpha,  
    1.0f, 0.0f, 1.0f, alpha,  
    1.0f, 0.0f, 1.0f, alpha,
    1.0f, 0.0f, 1.0f, alpha,
    
    0.0f, 1.0f, 1.0f, alpha,  
    0.0f, 1.0f, 1.0f, alpha,  
    0.0f, 1.0f, 1.0f, alpha,
    0.0f, 1.0f, 1.0f, alpha,
    
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,
    1.0f, 1.0f, 1.0f, alpha
  };


GLfloat pfColors1[] = 
  {
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,
    1.0f, 1.0f, 1.0f, alpha,
    
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,
    1.0f, 1.0f, 1.0f, alpha,
    
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,
    1.0f, 1.0f, 1.0f, alpha,
    
    
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,
    1.0f, 1.0f, 1.0f, alpha,
    
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,
    1.0f, 1.0f, 1.0f, alpha,
    
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,  
    1.0f, 1.0f, 1.0f, alpha,
    1.0f, 1.0f, 1.0f, alpha
  };


GLfloat cubeVerts[] =
  {
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f, 
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
  };

GLfloat cubeNormals[] =
  {  
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
  };

   GLfloat cubeTex[] =
   {
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      1.0f, 0.0f,
      1.0f, 1.0f,
      0.0f, 1.0f,
      0.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
   };


int cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };




PF_RITEM
pf_cube_new (float x, float y, float z)
{
  float    cubeVerts1[24 * 3];
  PF_RITEM ri;
  int      prg, i;

  prg = get_shader ("per-pixel");

  ri = pf_ritem_new ();

  /* Update cube vertexs */
  for (i = 0; i < 24; i++)
    {
      cubeVerts1[i * 3 + 0] = cubeVerts[i * 3 + 0] * x;
      cubeVerts1[i * 3 + 1] = cubeVerts[i * 3 + 1] * y;
      cubeVerts1[i * 3 + 2] = cubeVerts[i * 3 + 2] * z;
    }

  cube_mesh = pf_mesh_new (24, 36);
  pf_mesh_set_vertex (cube_mesh, cubeVerts1);
  pf_mesh_set_index (cube_mesh, cubeIndices);
  pf_mesh_set_normal (cube_mesh, cubeNormals);
  pf_mesh_set_color (cube_mesh, pfColors1);

  pf_ritem_set_mesh (ri, cube_mesh);

  pf_ritem_set_prg (ri, prg);

  return ri;
}
