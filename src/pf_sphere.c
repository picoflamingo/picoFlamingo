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
static PF_MATRIX44 theMat;
static PF_MATRIX44 rotMat;
static PF_MATRIX44 trasMat;
static PF_MATRIX44 modelView;
static PF_MATRIX44   projection;
static float m_fAngle = 1.5;

static GLuint uiProgramObject;             // Used to hold the program handle (made out of the two previous shaders

//  float alpha = 1.0;
#define alpha 1.0

GLfloat sColors[] = {1.0f, 0.0f, 0.0f, alpha,  

};




int esGenSphere ( int numSlices, float radius, 
		  GLfloat **vertices, GLfloat **normals,
		  GLfloat **texCoords, GLuint **indices )
{
   int i;
   int j;
   int numParallels = numSlices / 2;
   int numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
   int numIndices = numParallels * numSlices * 6;
   float angleStep = (2.0f * M_PI) / ((float) numSlices);

   // Allocate memory for buffers
   if ( vertices != NULL )
      *vertices = malloc ( sizeof(GLfloat) * 3 * numVertices );

   if ( normals != NULL )
      *normals = malloc ( sizeof(GLfloat) * 3 * numVertices );

   if ( texCoords != NULL )
      *texCoords = malloc ( sizeof(GLfloat) * 2 * numVertices );

   if ( indices != NULL )
      *indices = malloc ( sizeof(GLuint) * numIndices );

   for ( i = 0; i < numParallels + 1; i++ )
   {
      for ( j = 0; j < numSlices + 1; j++ )
      {
         int vertex = ( i * (numSlices + 1) + j ) * 3;

         if ( vertices )
         {
            (*vertices)[vertex + 0] = radius * sinf ( angleStep * (float)i ) *
                                               sinf ( angleStep * (float)j );
            (*vertices)[vertex + 1] = radius * cosf ( angleStep * (float)i );
            (*vertices)[vertex + 2] = radius * sinf ( angleStep * (float)i ) *
                                               cosf ( angleStep * (float)j );
         }

         if ( normals )
         {
            (*normals)[vertex + 0] = (*vertices)[vertex + 0] / radius;
            (*normals)[vertex + 1] = (*vertices)[vertex + 1] / radius;
            (*normals)[vertex + 2] = (*vertices)[vertex + 2] / radius;
         }

         if ( texCoords )
         {
            int texIndex = ( i * (numSlices + 1) + j ) * 2;
            (*texCoords)[texIndex + 0] = (float) j / (float) numSlices;
            (*texCoords)[texIndex + 1] = ( 1.0f - (float) i ) / (float) (numParallels - 1 );
         }
      }
   }
   // Generate the indices
   if ( indices != NULL )
   {
      GLuint *indexBuf = (*indices);
      for ( i = 0; i < numParallels ; i++ )
      {
         for ( j = 0; j < numSlices; j++ )
         {
            *indexBuf++  = i * ( numSlices + 1 ) + j;
            *indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + j;
            *indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );
            
            *indexBuf++ = i * ( numSlices + 1 ) + j;
            *indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );
            *indexBuf++ = i * ( numSlices + 1 ) + ( j + 1 );
         }
      }
   }
      
   //return numIndices;
   return numVertices;
}


PF_RITEM
pf_sphere_render_item (float radius)
{
  PF_RITEM ri;
  int      prg;
  char*    shader_attribs[] = {"myVertex", "myColor", "myNormal", NULL};
  float    *svertex, *snormal, *stex;
  GLuint      *sindex;
  PF_MESH  mesh;
  int      n, slices;

  prg = shader_program ("per-pixel", shader_attribs);
  ri = pf_ritem_new ();

  slices = 20;
  n = esGenSphere ( slices, radius, &svertex, &snormal, NULL, &sindex);

  mesh = pf_mesh_new (n, slices / 2 * slices * 6);
  pf_mesh_set_vertex (mesh, svertex);
  pf_mesh_set_index (mesh, sindex);
  pf_mesh_set_normal (mesh, snormal);
  pf_mesh_set_color (cube_mesh, sColors);
  //pf_mesh_set_program (cube_mesh, "simple.vert", "simple.frag");

  pf_ritem_set_mesh (ri, mesh);
  // pf_ritem_set_pos (ri, 0.0, 0.0, -2.0);
  //pf_ritem_set_rot (ri, 1.5, 0.0, 0.0);
  pf_ritem_set_prg (ri, prg);

  //pfmat_identity (rotMat);
  return ri;
}
