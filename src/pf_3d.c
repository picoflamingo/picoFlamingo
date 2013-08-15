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


/* 
*************************************************
FIXME: This module is not fully implemented. 
       This is an alpha version for demonstration purposes
       Memory is not properly freed on destruction
*************************************************
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

#include "pf_font.h"

#include "3ds.h"

#include "log.h"

#define VERTEX_ARRAY    0
#define COLOR_ARRAY     1

extern char *pres_dir;

typedef struct ritem_3d_priv_t
{
  OBJ3D    obj;
} *RITEM_3D_PRIV;



/* FIXME: Implement prover destruction of 3D model
 *        We need to add this callback to the render item
 *        when finally implemented
 */
int
pf_3d_free (PF_RITEM ri)
{
  RITEM_3D_PRIV priv; 

  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);

  // Delete what ever we need 

  free (priv);

  return 0;
}

int
pf_3d_render (PF_RITEM pfri)
{
  if (!pfri) return -1;

  if (pfri->depth)
    glEnable (GL_DEPTH_TEST);
  else
    glDisable (GL_DEPTH_TEST);

  glEnable (GL_BLEND);

  pf_ritem_render (pfri);

  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  return 0;
}

PF_RITEM
pf_3d_new (char *fname)
{
  PF_RITEM        ri; 
  PF_MESH         the_mesh;
  RITEM_3D_PRIV priv; 
  int             j, prg;
  long            i;
  float           r, g, b;
  char            buffer[1024];

  /* XXX: By default diffuse illumination */
  prg = get_shader ("diffuse");

  ri = pf_ritem_new ();
  ri->id = PFRI_3D;

  LOG ("Adding 3d model '%s'\n", fname);
  // Set item data
  priv = malloc (sizeof (struct ritem_3d_priv_t));

  sprintf (buffer, "%s/%s", pres_dir, fname);
  LOG  ("Loading model... '%s'\n", buffer);
  loader_3ds (&priv->obj, buffer);

  LOG ("Num Meshes: %d\n", priv->obj.n_meshes);

  for (i = 0; i < priv->obj.n_meshes; i++)
    {
      int n_vertex = 0;


      LOG ("Adding mesh %d \n", i);
      LOG ("Producing mesh (%d): %d vertexs, %d polygons\n", i, 
	      priv->obj.mesh[i].n_vertex, 
	      priv->obj.mesh[i].n_poly
	      );

      n_vertex =  priv->obj.mesh[i].n_poly * 3;
      the_mesh = pf_mesh_new (n_vertex, 
			      n_vertex);

      /* Allocate buffers */
      float  * _vertex = malloc (sizeof(float) * n_vertex * 3);
      float  * _normal = malloc (sizeof(float) * n_vertex * 3);
      int    * _index = malloc (sizeof(int) * n_vertex);
      float  * _color = malloc (sizeof(float) * n_vertex * 4);

      // Get Color for this mesh
      if (priv->obj.mesh[i].mat < priv->obj.n_mats)
	{
	  r = (float) priv->obj.mat_list[priv->obj.mesh[i].mat].color.r / 255.0;
	  g = (float) priv->obj.mat_list[priv->obj.mesh[i].mat].color.g / 255.0;
	  b = (float) priv->obj.mat_list[priv->obj.mesh[i].mat].color.b / 255.0;
	}
      else
	r = g = b = 0.5;

      for (j = 0; j < n_vertex; j++)
	{
	  _color[j * 4 + 0] = r;
	  _color[j * 4 + 1] = g;
	  _color[j * 4 + 2] = b;
	  _color[j * 4 + 3] = 1.0;

	}

      LOG ("Producing buffers for mesh %d (color: %f,%f,%f) \n", 
	   i, r, g, b);
      /* Build data */
      for (j = 0; j < priv->obj.mesh[i].n_poly; j++)
	{
	  int a, b, c;
	  /* Get face vertex index */
	  a = priv->obj.mesh[i].poly[j].a;
	  b = priv->obj.mesh[i].poly[j].b;
	  c = priv->obj.mesh[i].poly[j].c;

	  /* Add vertex to vetex array. For each face add 3 vertex  */
	  *(_vertex + j * 9 + 0) = priv->obj.mesh[i].vertex[a].x;
	  *(_vertex + j * 9 + 1) = priv->obj.mesh[i].vertex[a].y;
	  *(_vertex + j * 9 + 2) = priv->obj.mesh[i].vertex[a].z;  

	  *(_vertex + j * 9 + 3) = priv->obj.mesh[i].vertex[b].x;
	  *(_vertex + j * 9 + 4) = priv->obj.mesh[i].vertex[b].y;
	  *(_vertex + j * 9 + 5) = priv->obj.mesh[i].vertex[b].z;  

	  *(_vertex + j * 9 + 6) = priv->obj.mesh[i].vertex[c].x;
	  *(_vertex + j * 9 + 7) = priv->obj.mesh[i].vertex[c].y;
	  *(_vertex + j * 9 + 8) = priv->obj.mesh[i].vertex[c].z;  
	  /* Add normal vector. For each face vertex add the same normal */

 	  *(_normal + j * 9 + 0) = priv->obj.mesh[i].normal[j].x;
	  *(_normal + j * 9 + 1) = priv->obj.mesh[i].normal[j].y;
	  *(_normal + j * 9 + 2) = priv->obj.mesh[i].normal[j].z;

 	  *(_normal + j * 9 + 3) = priv->obj.mesh[i].normal[j].x;
	  *(_normal + j * 9 + 4) = priv->obj.mesh[i].normal[j].y;
	  *(_normal + j * 9 + 5) = priv->obj.mesh[i].normal[j].z;

 	  *(_normal + j * 9 + 6) = priv->obj.mesh[i].normal[j].x;
	  *(_normal + j * 9 + 7) = priv->obj.mesh[i].normal[j].y;
	  *(_normal + j * 9 + 8) = priv->obj.mesh[i].normal[j].z;
	  
	  /* Build index array */
	  *(_index + j * 3 + 0) = j * 3 + 0;
	  *(_index + j * 3 + 1) = j * 3 + 1;
	  *(_index + j * 3 + 2) = j * 3 + 2;	  
	}
 
      pf_mesh_set_vertex (the_mesh, _vertex);
      pf_mesh_set_index (the_mesh, _index);
      pf_mesh_set_normal (the_mesh, _normal);
      pf_mesh_set_color (the_mesh, _color);

      free (_normal);
      free (_index);
      free (_vertex);
      free (_color);
      
      pf_ritem_add_mesh (ri, the_mesh);

    }

  loader_3ds_free (&priv->obj);
  pf_ritem_set_priv_data (ri, priv);
  pf_ritem_set_render_func (ri, pf_3d_render);
  pf_ritem_set_prg (ri, prg);
  /* Now Free 3D object */

  return ri;
}
