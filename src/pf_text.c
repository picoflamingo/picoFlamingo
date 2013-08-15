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

#include <ctype.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>


#include "sys/time.h"

#include "render_engine.h"
#include "shader.h"
#include "matrix.h"
#include "pf_mesh.h"
#include "pf_quad.h"
#include "pf_render_item.h"

#include "pf_font.h"

#define VERTEX_ARRAY    0
#define COLOR_ARRAY     1

typedef struct ritem_text_priv_t
{
  PF_FONT   *font;
  //float     scale;
  float     scale_x;
  float     scale_y;
  float     w;
  int       lines;
  float     interline;
  float     para_space;
  float     indent1;
  float     indent2;
  float     ts;
  char      **text;  
  float     r, g, b, alpha;
  /* TBC: Additional fields */
  int       current_line;
  int       real_line;
  int       lines_up;
  int       lines_down;
  PF_RITEM  highlight;
  float     hl_h, hl_x, hl_y, hl_z, hl_off;
  int hl;
} *RITEM_TEXT_PRIV;

extern float m_fAngle;


int      
pf_text_get_params (PF_RITEM pfri, float* scale_x, float *scale_y,
		    float *interline, float *width)
{
  RITEM_TEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);

  *scale_x = priv->scale_x;
  *scale_y = priv->scale_y;
  *interline = priv->interline;
  *width = priv->w;

  return 0;
  
}


/* *****************************************************
 * ****************************
 * OPTIMISATIONS
 * --> Encode text vertex attributes in VBOs
 * --> Produce Fonts texture atlas (modifications on pf_font!!!)
 * ****************************
 */

static int
should_wrap (RITEM_TEXT_PRIV rtp, float x0, char *txt)
{
  float x = x0;
  char *aux = txt;

  while ((*aux != 0) && !isblank(*aux))
    {
      if (rtp->w > 0.0 && x > rtp->w) return 1;
      x +=  rtp->font->glyphs[(int)*aux].advance / rtp->scale_x;      
      aux++;
    }
  return 0;
}


// ********************************************************
int
pf_text_render (PF_RITEM pfri)
{
  PF_MATRIX44 theMat;
  PF_MATRIX44 modelView;
  RITEM_TEXT_PRIV rtp;
  int         i, start, end, line, real_line, cnt;
  char        *aux;
  float       x, y, y_offset;
  float       *tc;
  float       *pf_v;

  if (!pfri->show) return 0;


  rtp = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);


  if (rtp->highlight)
    {
      pf_quad_set_size (rtp->highlight, rtp->w + 0.5, rtp->hl_h);
#if 0
      pf_ritem_set_abs_pos (rtp->highlight,
			    rtp->hl_x,
			    rtp->hl_y - rtp->hl_off,
			    rtp->hl_z);
#endif
      pf_ritem_set_abs_pos (rtp->highlight,
			    0.0,
			    //pf_ri->y - rtp->hl_off,
			    pfri->y - rtp->hl_off,
			    rtp->hl_z - 0.1);

      pf_quad_render (rtp->highlight);
    }


  if (rtp->lines == 0) return 0;
  start = rtp->current_line;
  if (rtp->current_line >= rtp->lines) start = rtp->lines - 1;
  if (rtp->lines_down == -1) end = rtp->lines;
  else
    end = rtp->current_line + rtp->lines_down;

  if (end > rtp->lines) end = rtp->lines;

  start -= rtp->lines_up;
  if (start < 0) start = 0;
  y_offset = rtp->current_line - start;

  

  if (pfri->depth)
    glEnable (GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);

  glEnable (GL_BLEND);
  glBlendEquation (GL_FUNC_ADD);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glUseProgram(pf_mesh_get_program (pfri->mesh[0]));
  glUniform1i(glGetUniformLocation(pf_mesh_get_program (pfri->mesh[0]), 
				   "sampler2d"), 0);


  real_line = line = 0;

  cnt = 0;
  for (i = 0; i < end - start; i++)
    {     
      aux = rtp->text[start + i];
      if (*aux == '\\' && *(aux + 1) == 'i')
	{
	  x = 0.0;
	  aux +=2;
	}
      else
	x = rtp->indent1;

      real_line = start + i;
      while (*aux)
	{
	  if (*aux == '\\' && *(aux + 1) == 't')
	    {
	      aux += 2;
	      x = rtp->ts;
	      continue;
	    }
	  if (!isblank (*aux) && should_wrap (rtp, x, aux))
	    {
	      /* Wrap line */
	      x = rtp->indent2;
	      line++;
	      // skips whites for next line
	      while (*aux == ' ') aux++;
	      if (real_line == rtp->current_line)
		cnt++;
	    }

	  y = (rtp->font->glyphs[(int)*aux].base) / rtp->scale_y 
	    - ((i + line)  - y_offset) * rtp->interline 
	    - (real_line - 1 - start) * rtp->para_space;


	  pfmat_translate (theMat, x, y, 0.0);

	  pfmat_mul (modelView, theMat, pf_re_get_modelview());
	  pfmat_mul (modelView, pfri->current_trans, modelView);
	  pfmat_mul (modelView, modelView, pf_re_get_projection ());


	  tc = pf_mesh_get_texcoord (pfri->mesh[0]);
	  pf_v = pf_mesh_get_vertex (pfri->mesh[0]);

	  // Adjust Matrix
	  // Bind texture    
	  tc[0] = rtp->font->glyphs[(int)*aux].texcoord[0];
	  tc[1] = rtp->font->glyphs[(int)*aux].texcoord[3];
	  
	  tc[2] = rtp->font->glyphs[(int)*aux].texcoord[2];
	  tc[3] = rtp->font->glyphs[(int)*aux].texcoord[3];
	  
	  tc[4] = rtp->font->glyphs[(int)*aux].texcoord[0];
	  tc[5] = rtp->font->glyphs[(int)*aux].texcoord[1];
	  
	  tc[6] = rtp->font->glyphs[(int)*aux].texcoord[2];
	  tc[7] = rtp->font->glyphs[(int)*aux].texcoord[1];
	  
	  
	  pf_v[0]  = rtp->font->glyphs[(int)*aux].minx / rtp->scale_x;  
	  pf_v[1]  = rtp->font->glyphs[(int)*aux].miny / rtp->scale_y;  
	  pf_v[2]  = 0.0;  
	  
	  pf_v[3]  = rtp->font->glyphs[(int)*aux].maxx / rtp->scale_x;  
	  pf_v[4]  = rtp->font->glyphs[(int)*aux].miny / rtp->scale_y;  
	  pf_v[5]  = 0.0;  
	  
	  pf_v[6]  = rtp->font->glyphs[(int)*aux].minx / rtp->scale_x;  
	  pf_v[7]  = rtp->font->glyphs[(int)*aux].maxy / rtp->scale_y;  
	  pf_v[8]  = 0.0;  
	  
	  pf_v[9]  = rtp->font->glyphs[(int)*aux].maxx / rtp->scale_x;  
	  pf_v[10] = rtp->font->glyphs[(int)*aux].maxy / rtp->scale_y;  
	  pf_v[11] = 0.0;  
	  
	  glBindTexture (GL_TEXTURE_2D, rtp->font->glyphs[(int)*aux].texid);

	  pf_mesh_render (pfri->mesh[0], modelView);
	  

	  x +=  rtp->font->glyphs[(int)*aux].advance / rtp->scale_x;
	  aux++; /* Next character*/
	}
    }
  if (cnt)
    {
      rtp->hl_h = (cnt + 1) * rtp->interline + rtp->para_space;
      rtp->hl_off = rtp->hl_h / 2.0 - 1.5 * rtp->interline / 2.0 - rtp->para_space;
    }
  else
    {
      rtp->hl_h = (cnt + 1) * rtp->interline + rtp->para_space;
      rtp->hl_off = - rtp->para_space;
    }

  glBindTexture (GL_TEXTURE_2D, 0);
  glDisable (GL_BLEND);

  glEnable (GL_DEPTH_TEST);
  return 0;  
}


int
pf_text_free (PF_RITEM ri)
{
  int             i;
  RITEM_TEXT_PRIV priv; 

  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);
  for (i = 0; i < priv->lines; i++)
    if (priv->text[i]) free (priv->text[i]);
  if (priv->text) free (priv->text);
  pf_ritem_free (priv->highlight);
  free (priv);

  return 0;
}

PF_RITEM
pf_text_new (char *font, char *text)
{
  PF_RITEM        ri; 
  PF_MESH         the_mesh;
  RITEM_TEXT_PRIV priv; 
  int             prg;
  GLfloat         pfVertices[12];
  GLfloat         pfNormals[12];
  GLfloat         pfColors[16];
  float           tc[8];

  prg = get_shader ("basic-text");


  ri = pf_ritem_new ();
  ri->id = PFRI_TEXT;
  pf_ritem_set_free_func (ri, pf_text_free);

  the_mesh = pf_mesh_new (4, 4);

  pf_mesh_set_vertex (the_mesh, pfVertices);
  pf_mesh_set_normal (the_mesh, pfNormals);
  pf_mesh_set_color (the_mesh, pfColors);
  pf_mesh_set_texcoord (the_mesh, tc);


  pf_ritem_set_mesh (ri, the_mesh);

  // use the first text unit to pass font texture
  glUniform1i(glGetUniformLocation(prg, "sampler2d"), 0);

  pf_ritem_set_prg (ri, prg);
  pf_ritem_set_render_func (ri, pf_text_render);

  // Set item data
  priv = malloc (sizeof (struct ritem_text_priv_t));
  memset (priv, 0, sizeof (struct ritem_text_priv_t));
  priv->font = pf_get_font (font);
  priv->lines = 0;
  priv->interline = 0.5;
  priv->indent1 = 0.0;
  priv->indent2 = 0.0;
  priv->para_space = 0.0;
  priv->w = -1;

  priv->ts = 0.4;

  priv->current_line = 0;
  priv->lines_up = 0;
  priv->lines_down = -1; /* Draw all the lines down*/

  priv->highlight = NULL;

  if (text)
    {
      priv->lines = 1;
      priv->text = malloc (sizeof(char*));
      priv->text[0] = strdup (text);
    }
  priv->scale_x = priv->scale_y = 120.0;
  pf_ritem_set_priv_data (ri, priv);
  
  // Deactivate Depth test for text
  // XXX:
  // For non propertional fonts letter can overlap introducing
  // artifacts in the final result
  ri->depth = 0;

  return ri;
}

int      
pf_text_set_scale (PF_RITEM pfri, float scale_x, float scale_y)
{
  RITEM_TEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);

  /* Only positive (non-zero) values alowed */
  if (scale_x > 0.0 )
    priv->scale_x = scale_x;
  if (scale_y > 0.0)
    priv->scale_y = scale_y;

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}


int      
pf_text_set_width (PF_RITEM pfri, float w)
{
  RITEM_TEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);

  /* Only positive (non-zero) values alowed */
  if (w > 0.0 )
    priv->w = w;

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}


int      
pf_text_set_font (PF_RITEM pfri, char *font)
{
  RITEM_TEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);

  priv->font = pf_get_font (font);

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}


int      
pf_text_set_interline (PF_RITEM pfri, float il)
{
  RITEM_TEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->interline = il;
  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}

int      
pf_text_add_text_line (PF_RITEM pfri, char *text)
{
  RITEM_TEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->lines++;
  priv->text = realloc (priv->text, sizeof(char*) * priv->lines);
  priv->text[priv->lines - 1] = strdup (text);

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}

int      
pf_text_clear (PF_RITEM pfri)
{
  RITEM_TEXT_PRIV priv; 
  int             i;

  if (!pfri) return -1;
  
  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);

  /* Free text memory */
  for (i = 0; i < priv->lines; i++)
    free (priv->text[i]);

  free (priv->text);
  priv->text = NULL;
  priv->lines = 0;

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}

int
pf_text_get_color (PF_RITEM pfri, float* r, float* g, float* b, float *alpha)
{
  RITEM_TEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);

  *r = priv->r;
  *g = priv->g;
  *b = priv->b;
  *alpha = priv->alpha;

  return 0;
}

int      
pf_text_set_color (PF_RITEM pfri, float r, float g, float b, float alpha)
{
  RITEM_TEXT_PRIV priv; 
  float   *colors;
  int     i;
  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->r = r;
  priv->g = g;
  priv->b = b;
  priv->alpha = alpha;
  
  // build color array
  colors = malloc (sizeof(float) * 16);
  for (i = 0; i < 16; i += 4)
    {
      colors [i] = r;
      colors [i + 1] = g;
      colors [i + 2] = b;
      colors [i + 3] = alpha;
    }

  pf_mesh_set_color (pfri->mesh[0], colors);  
  free (colors);
  pf_ritem_set_priv_data (pfri, priv);
  if (priv->highlight)
    pf_quad_set_color (priv->highlight, 0.3, 0.99, 0.3, alpha);
  return 0;
}



int      
pf_text_set_current_line (PF_RITEM pfri, int cl)
{
  RITEM_TEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->current_line = cl;

  return 0;
}

int      
pf_text_set_lines_up (PF_RITEM pfri, int cl)
{
  RITEM_TEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->lines_up = cl;
  
  return 0;
}

int      
pf_text_set_lines_down (PF_RITEM pfri, int cl)
{
  RITEM_TEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->lines_down = cl;

  return 0;
}


int      
pf_text_set_para (PF_RITEM pfri, float indent1, float indent2, float para)
{
  RITEM_TEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->indent1 = indent1;
  priv->indent2 = indent2;
  priv->para_space = para;

  printf ("Text Para updated: %f %f %f\n", 
	  priv->indent1, priv->indent2, priv->para_space);
  return 0;
}

int      
pf_text_set_hl (PF_RITEM pfri, int flag)
{
  RITEM_TEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_TEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->hl = flag;
  if (priv->highlight) return 0;
  printf ("Creating Highlight Quad\n");
  priv->highlight = pf_quad_new (6.0, 2.0);
  pf_ritem_set_color (priv->highlight, 0.3, 0.99, 0.3, 0.99);
  priv->hl_x = -0.1;
  priv->hl_y = -1.95;
  priv->hl_z = -4.41;
  //pf_ritem_set_pos (priv->highlight, pfri->x, pfri->y, pfri->z - 0.05);
  pf_ritem_set_pos (priv->highlight, -0.1, -1.95, -4.41);
  return 0;
}
