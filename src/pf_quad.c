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

#include <unistd.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "render_engine.h"
#include "shader.h"
#include "matrix.h"
#include "pf_mesh.h"
#include "pf_render_item.h"

#include "pf_quad.h"
#include "log.h"


typedef struct ritem_quad_priv_t
{
  GLuint  texid;
  float   texcoord[4];
  float     w, h;
} *RITEM_QUAD_PRIV;


// ********************************************************
int
pf_quad_render (PF_RITEM pfri)
{
  PF_MATRIX44 theMat;
  PF_MATRIX44 modelView;
  RITEM_QUAD_PRIV rtp;

  if (!pfri->show) return 0;
  glUseProgram(pfri->program);

  rtp = (RITEM_QUAD_PRIV) pf_ritem_get_priv_data (pfri);
  
  pfmat_identity (modelView);
  pfmat_identity (theMat);
  
  pfmat_mul (modelView, theMat, modelView);
  pfmat_mul (modelView, pfri->current_trans, modelView);
  pfmat_mul (modelView, modelView, pf_re_get_projection ());
  
  
  glEnable (GL_BLEND);
  if (pfri->depth)
    glEnable (GL_DEPTH_TEST);
  else
    glDisable (GL_DEPTH_TEST);

  glBlendEquation (GL_FUNC_ADD);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  if (rtp->texid >= 0)
    glBindTexture (GL_TEXTURE_2D, rtp->texid);

  glUseProgram(pf_mesh_get_program (pfri->mesh[0]));
  glUniform1i(glGetUniformLocation (pf_mesh_get_program (pfri->mesh[0]), 
				    "sampler2d"), 0);
  
  
  pf_mesh_render (pfri->mesh[0], modelView);
  
  if (rtp->texid >= 0)
    glBindTexture (GL_TEXTURE_2D, 0);

  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  
  return 0;  
}


int
pf_quad_ritem_free (PF_RITEM ri)
{
  RITEM_QUAD_PRIV priv;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  free (priv);

  return 0;
}


PF_RITEM
pf_quad_new (float w, float h)
{
  PF_RITEM        ri; 
  PF_MESH         the_mesh;
  RITEM_QUAD_PRIV  priv; 
  float           x0, y0, x1, y1;
  float           w2, h2;
  int             prg;

  GLfloat pfNormals[] = { 0.0, 0.0, 1.0f, 
			  0.0, 0.0, 1.0f, 
			  0.0, 0.0, 1.0f,
                          0.0, 0.0, 1.0f};

  
  GLfloat pfColors[] = {1.0f, 1.0f, 1.0f, 1.0,  
			1.0f, 1.0f, 1.0f, 1.0,  
			1.0f, 1.0f, 1.0f, 1.0,
			1.0f, 1.0f, 1.0f, 1.0
  };
  

  if ((prg = get_shader ("basic")) < 0)
    {
      fprintf (stderr, "Cannot load shader 'basic-text\n");
      return NULL;
    }

  ri = pf_ritem_new ();
  ri->id = PFRI_QUAD;

  pf_ritem_set_free_func (ri, (void*) pf_quad_ritem_free);

  the_mesh = pf_mesh_new (4, 4);

  // Set item data
  priv = malloc (sizeof (struct ritem_quad_priv_t));

  priv->texid = -1;
  priv->w = w;
  priv->h = h;

  pf_ritem_set_priv_data (ri, priv);

  //Update mapping coordinates and vertex

  GLfloat pfVertices[] = {-1.0,  -1.0, 0.0f, 
			   1.0,  -1.0, 0.0f, 
			  -1.0,  1.0, 0.0f,
                           1.0,  1.0, 0.0f};

  w2 = w / 2.0f;
  h2 = h / 2.0f;

  x0 = -w2;
  y0 = -h2;
  x1 = w2;
  y1 = h2;

  pfVertices[0]  = x0;
  pfVertices[1]  = y0;
  pfVertices[3]  = x1;
  pfVertices[4]  = y0;
  pfVertices[6]  = x0;
  pfVertices[7]  = y1;
  pfVertices[9]  = x1;
  pfVertices[10] = y1;

  LOG ("Quad  (%f,%f): %f, %f - %f, %f\n", w, h ,
       x0, y0, x1, y1);

  pf_mesh_set_vertex (the_mesh, pfVertices);
  pf_mesh_set_normal (the_mesh, pfNormals);
  pf_mesh_set_color (the_mesh, pfColors);


  pf_quad_set_color (ri, 1.0, 1.0, 1.0, 1.0);
  pf_ritem_set_mesh (ri, the_mesh);

  pf_ritem_set_prg (ri, prg);
  pf_ritem_set_render_func (ri, pf_quad_render);

  return ri;
}

/*
#define CLAMP_L(x) (x < 0.0) ? 0.0 : x
#define CLAMP_H(x) (x > 1.0) ? 1.0 : x
*/

#define CLAMP_L(x) x
#define CLAMP_H(x) x

int      
pf_quad_set_color (PF_RITEM pfri, float r, float g, float b, float alpha)
{
  float   *colors;

  if (!pfri) return -1;

  LOG ("Changing quad color...\n");
  // build color array
  colors = malloc (sizeof(float) * 16);

#ifdef PLAIN_COLOR
  for (i = 0; i < 16; i += 4)
    {
      colors [i] = r;
      colors [i + 1] = g;
      colors [i + 2] = b;
      colors [i + 3] = alpha;
    }
#else
  colors [0 * 4 + 0] = r;
  colors [0 * 4 + 1] = g;
  colors [0 * 4 + 2] = b;
  colors [0 * 4 + 3] = alpha;


  colors [1 * 4 + 0] = CLAMP_L(r - 0.2);
  colors [1 * 4 + 1] = CLAMP_L(g - 0.2);
  colors [1 * 4 + 2] = CLAMP_L(b - 0.2);
  colors [1 * 4 + 3] = alpha;


  colors [2 * 4 + 0] = CLAMP_H(r + 0.2);
  colors [2 * 4 + 1] = CLAMP_H(g + 0.2);
  colors [2 * 4 + 2] = CLAMP_H(b + 0.2);
  colors [2 * 4 + 3] = alpha;

  colors [3 * 4 + 0] = CLAMP_L(r - 0.2);
  colors [3 * 4 + 1] = CLAMP_L(g - 0.2);
  colors [3 * 4 + 2] = CLAMP_L(b - 0.2);
  colors [3 * 4 + 3] = alpha;
#endif


  pf_mesh_set_color (pfri->mesh[0], colors);  
  free (colors);

  return 0;
}

int      
pf_quad_set_vertex (PF_RITEM ri, float x0, float y0, float x1, float y1)
{
  float *pfVertices;
  RITEM_QUAD_PRIV priv;

  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  pfVertices = pf_mesh_get_vertex (ri->mesh[0]);

  if (pfVertices)
    {
      pfVertices[0]  = x0;
      pfVertices[1]  = y0;
      pfVertices[3]  = x1;
      pfVertices[4]  = y0;
      pfVertices[6]  = x0;
      pfVertices[7]  = y1;
      pfVertices[9]  = x1;
      pfVertices[10] = y1;
    }

  LOG ("Quad  %f, %f - %f, %f\n", 
	 x0, y0, x1, y1);

  return 0;
}

int      
pf_quad_set_size (PF_RITEM ri, float w, float h)
{
  RITEM_QUAD_PRIV priv;
  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  priv->w = w;
  priv->h = h;
  pf_quad_set_vertex (ri, 
		      -w / 2.0, -h / 2.0,
		      w / 2.0, h / 2.0);

  return 0;
}

int      
pf_quad_get_size (PF_RITEM ri, float *w, float *h)
{
  RITEM_QUAD_PRIV priv;
  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  *w = priv->w;
  *h = priv->h;

  return 0;
}

int      
pf_quad_set_width (PF_RITEM ri, float w)
{
  RITEM_QUAD_PRIV priv;
  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  priv->w = w;
  pf_ritem_set_priv_data (ri, priv);

  return 0;
}

int      
pf_quad_set_height (PF_RITEM ri, float h)
{
  RITEM_QUAD_PRIV priv;
  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  priv->h = h;
  pf_ritem_set_priv_data (ri, priv);

  return 0;
}
