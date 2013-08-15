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

#include "log.h"
#include "render_engine.h"
#include "shader.h"
#include "matrix.h"
#include "pf_mesh.h"
#include "pf_quad.h"
#include "pf_render_item.h"

#include "pf_font.h"
#include "pf_stext.h"

#define VERTEX_ARRAY    0
#define COLOR_ARRAY     1


typedef struct ritem_stext_priv_t
{
  PF_FONT   *font;
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
  float     hl;
  // STEX specific
  LRH       *lrh;
  float     y_offset;
} *RITEM_STEXT_PRIV;

extern float m_fAngle;


int      
pf_stext_get_params (PF_RITEM pfri, float* scale_x, float *scale_y,
		    float *interline, float *width)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

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
should_wrap1 (RITEM_STEXT_PRIV rtp, float x0, char *txt)
{
  float x = x0;
  char *aux = txt;

  while ((*aux != 0) && !isblank(*aux))
    {
      if (rtp->w > 0.0 && x > (rtp->w)) return 1;
      x +=  rtp->font->glyphs[(int)*aux].advance / rtp->scale_x;      
      aux++;
    }
  return 0;
}


static int
should_wrap (RITEM_STEXT_PRIV rtp, int x0, int w, char *txt)
{
  int x = x0;
  char *aux = txt;

  while ((*aux != 0) && !isblank(*aux))
    {
      if (x > w) return 1;
      x +=  rtp->font->glyphs[(int)*aux].advance;      
      aux++;
    }
  return 0;
}


static int
_blit_chr (PF_RITEM pfri, unsigned char *tex, char ch, int x, int y, int w, int h, int max_ch_h)
{
  RITEM_STEXT_PRIV rtp;
  int              i, j, stride, stride1, off;
  char             *data;

  rtp = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  LOG("Blitting character : %c (%dx%d) base: %d\n", 
       ch, rtp->font->glyphs[(int)ch].maxy, 
       rtp->font->glyphs[(int)ch].maxx,
       rtp->font->glyphs[(int)ch].base
       );
  
  stride = 4 * w;
  stride1 = 4 * rtp->font->glyphs[(int)ch].tw;
  data = rtp->font->glyphs[(int)ch].data;
  off = 4 * x + y *stride;
  y += max_ch_h - rtp->font->glyphs[(int)ch].maxy - rtp->font->glyphs[(int)ch].base;

  for (i = 0; i < rtp->font->glyphs[(int)ch].maxy; i++)
    for (j = 0; j < rtp->font->glyphs[(int)ch].maxx; j++)
      {
	memcpy (tex  + (x + j) * 4 + (i + y) * stride, 
		data + j * 4 + i * stride1, 4);
      }
  return rtp->font->glyphs[(int)ch].advance;
}

int
lrh_calculate (PF_RITEM pfri, int the_line)
{
  RITEM_STEXT_PRIV rtp;
  LRH              lrh;
  int              line;
  int              w, h, x_tex, y_tex, max_ch_h, h1, w1, tmp, tmp1;
  float            w_src, h_src, x_src ,y_src;
  char             *aux;
  float            y_offset;
  GLubyte *expanded_data = NULL;
  int              max_h;

  rtp = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  lrh = rtp->lrh[the_line];
  aux  = rtp->text[the_line];

  LOG ("Calculating LRH for line %d (%f): '%s'\n", 
       the_line, rtp->w, aux);

  y_offset = 0.0;
  w = h = 0; 

  lrh->n_sublines = 1;

  w = h = 0;
  line = 0;
  x_tex = y_tex = 0;
  x_src = y_src = 0.0;
  h1 = max_ch_h = 0;
  w1 = 0.0;

  /*
  max_h = h1 = max_ch_h = rtp->font->glyphs['C'].maxy + rtp->font->glyphs['p'].maxy -
    rtp->font->glyphs['p'].base;
  */
  /* FIXME: This has to be calculated with the font */
  int mh = 0;
  mh = rtp->font->glyphs['C'].maxy;
  if (mh < rtp->font->glyphs['f'].maxy) mh = rtp->font->glyphs['f'].maxy;;
  //max_h = h1 = max_ch_h = rtp->font->glyphs['C'].maxy - rtp->font->glyphs['p'].base;
  max_h = h1 = max_ch_h = mh - rtp->font->glyphs['p'].base;

  //max_ch_h = rtp->font->glyphs['C'].maxy;

  
  if (*aux == '\\' && *(aux + 1) == 'i')
    {
      x_src = 0.0;
      x_tex = 0;
      aux +=2;
    }
  else
    {

      x_src = rtp->indent1;
      x_tex = (int) (rtp->indent1 * rtp->scale_x);
    }

  tmp = 0;
  h = h1;
  while (*aux)
    {
      if (*aux == '\\' && *(aux + 1) == 't')
	{
	  aux += 2;
	  x_tex = (int) (rtp->ts * rtp->scale_x);
	  x_src = rtp->ts;
	  continue;
	}
      if (!isblank (*aux) && should_wrap1 (rtp, x_src, aux))
	{
	  if (x_src > w_src) w_src = x_src;
	  if (w_src > w1) w1 = w_src;
	  if (x_tex > w) w = x_tex;
	  h += h1; // Add current line size
	  //h1 = 0;
	  //h1 = max_ch_h;
	  //h1 += max_ch_h;
	  x_src = rtp->indent2;
	  x_tex =(int)( rtp->indent2 * rtp->scale_x);
	  line++;
	  // skips whites for next line
	  while (*aux == ' ') aux++;
	  lrh->n_sublines++;
	  
	}
      x_tex += rtp->font->glyphs[(int)*aux].advance;
      x_src +=  rtp->font->glyphs[(int)*aux].advance / rtp->scale_x;
      aux++;
    }

  if (!line)
    {
      w_src = x_src;
    }
#if 0
  //line++;
  if (h == 0) 
    {
      h += h1;
    }
#endif
  // On Display size -> Quad Y coordinates
  h += h1 / 2.0;
  h += (2 * line);

  LOG ("Lines: %d\n", line);

  
  h_src = (float)h / rtp->scale_y; 
  rtp->y_offset = h_src / 2.0;

#if 0
  if (!line) rtp->hl_h = h_src / 4.0;
  else rtp->hl_h = h_src /2.0;
#endif
  if (x_tex > w) w = x_tex;


  LOG ("Device Coordinates: (%d) %f x %f\n", line, w_src, h_src);
  LOG ("Texture Coordinates: %d x %d Max Character Height: %d (%d)\n", w, h, max_ch_h, h1);
  lrh->n_sublines = line;
  lrh->vertex[0] = 0.0;
  lrh->vertex[1] = h_src;
  lrh->vertex[2] = 0.0;

  lrh->vertex[3] = w_src;
  lrh->vertex[4] = h_src;
  lrh->vertex[5] = 0.0;

  lrh->vertex[6] = 0.0;
  lrh->vertex[7] = 0.0;
  lrh->vertex[8] = 0.0;

  lrh->vertex[9] = w_src;
  lrh->vertex[10] = 0.0;
  lrh->vertex[11] = 0.0;
  // Create texture

  /* Allocate memory for the texture data.*/
  if ((expanded_data = 
       (GLubyte *) malloc ( sizeof(GLubyte) * 4 * w * (h))) == NULL)
    {
      fprintf (stderr, "ERROR: %s", "Cannot allocalte memory for "
	       "character texture\n");
      /* FIXME: Cleanup memory */
      return -1;
    }

  memset (expanded_data, 0, 4*w*h);
  // Just blit one character
  int  x, y;
  aux  = rtp->text[the_line];
  x = y = 0;

  x = y = 0.0;
  h1 = 0;
  x_src = 0.0;

  if (*aux == '\\' && *(aux + 1) == 'i')
    {
      x = x_src = 0.0;
      x_tex = 0;
      aux +=2;
    }
  else
    {
      x_src = rtp->indent1;
      x = x_tex = (int)(rtp->indent1 * rtp->scale_x);
    }
  while (*aux)
    {
      if (*aux == '\\' && *(aux + 1) == 't')
	{
	  aux += 2;
	  x = x_tex = (int) (rtp->ts * rtp->scale_x);
	  x_src = rtp->ts;
	  x_src += rtp->ts;
	  continue;
	}
      if (!isblank (*aux) && should_wrap (rtp, x_tex, w, aux))
	{
	  h1 += max_ch_h; // Add current line size
	  h1 += 2;
	  x = 0;
	  x_src = 0.0;
	  x_src = rtp->indent2;
	  x = x_tex =(int)( rtp->indent2 * rtp->scale_x);
	  line++;
	  // skips whites for next line
	  while (*aux == ' ') aux++;
	}
      x_tex += rtp->font->glyphs[(int)*aux].advance;
      x_src +=  rtp->font->glyphs[(int)*aux].advance / rtp->scale_x;
      x += _blit_chr (pfri, expanded_data , *aux, x, h1, w, h, max_ch_h);

      aux++;
    }

#if 0
  // XXX: Used for initial debug
  for(j = 0; j < h; j++) 
    for(i = 0; i < w; i++)
      {
	if ((i < 2) || (j < 2) || (i > w - 2) || (j > h - 2))
	  {
	    expanded_data[(4 * i + j * stride + 0)] = 0;
	    expanded_data[4 * (i + j * w) + 1] = 255;
	    expanded_data[4 * i + j * stride + 2] = 0;
	    
	    //if ((i == 0) || (j == 0) || (i == w) || (j == h))
	    expanded_data[4 * (i + j * w) + 3] = 255;	  
	  }
      }
#endif


  glGenTextures (1, (GLuint *)&lrh->texid[0]);
  LOG ("Created texture %d\n", lrh->texid[0]);
  lrh->flag = 1;
  glBindTexture   (GL_TEXTURE_2D, lrh->texid[0]);

  LOG ("Generating GL_RGBA texture of size %dx%d\n", w, h);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h,
		0, GL_RGBA, GL_UNSIGNED_BYTE, expanded_data );
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture   (GL_TEXTURE_2D, 0);
  LOG ("RLH done!\n");
  free (expanded_data);
  return 0;
}

// ********************************************************
int
pf_stext_render (PF_RITEM pfri)
{
  PF_MATRIX44 theMat;
  PF_MATRIX44 modelView;
  RITEM_STEXT_PRIV rtp;
  int         i, start, end, line, real_line, cnt;
  float       x, y, y_offset;
  float       *tc;
  float       *pf_v;

  if (!pfri->show) return 0;

  rtp = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  if (rtp->lines == 0) return 0;
  /************************************************************/
  /************************************************************/
  /************************************************************/
  start = 0;
  if (rtp->hl && rtp->highlight)
    {
      LRH lrh = rtp->lrh[rtp->current_line];
      float   w, h;
      if (lrh)
	{
	  if (rtp->w > 0)
	    w = rtp->w;
	  else
	    w = lrh->vertex[3];
	  h = lrh->vertex[4];

	  w *= 1.01;
#if 1
	  pf_quad_set_size (rtp->highlight, w, h);
	  pf_ritem_set_abs_pos (rtp->highlight,
				pfri->x + w / 2.0,
				//pfri->y - h / 4.0,
				pfri->y - h / 2.0,
				rtp->hl_z);
				//rtp->hl_z - 0.001);
#else
	  pf_quad_set_size (rtp->highlight, w, 0.01);
	  pf_ritem_set_abs_pos (rtp->highlight,
				pfri->x + w / 2.0,
				//pfri->y + h / 4.0,
				pfri->y - h,
				rtp->hl_z);
				//rtp->hl_z - 0.001);
#endif
	  pf_quad_render (rtp->highlight);


	}
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
  LOG ("Rendering from %d to %d (%d)\n", start, end, rtp->lines_down);
  x = 0.0;
  //y = -rtp->lrh[start]->vertex[1] / 2.0;
  //y = rtp->lrh[start]->vertex[4] / 2.0;

  //y = -rtp->lrh[rtp->current_line]->vertex[4];
  y = 0.0;
  //y = -rtp->lrh[start]->vertex[1] / (float)rtp->lrh[start]->n_sublines;

  //y = - rtp->interline / 2.0;
  cnt = 0;
  for (i = 0; i < end - start; i++)
    {     
      real_line = start + i;
      if (rtp->lrh[real_line]->flag == 0)
	{

	  lrh_calculate (pfri, real_line);
	  //y = -rtp->lrh[real_line]->vertex[1] / 2.0;
	  //y = -rtp->lrh[real_line]->vertex[1];
	  LOG ("Start y: %f\n", y);
	}
      y += -rtp->lrh[real_line]->vertex[1];

      LOG ("Line %d (%d) delta:%f y:%f\n", i, real_line,
	      rtp->lrh[real_line]->vertex[1], y);


      pfmat_translate (theMat, x, y, 0.0);

      pfmat_mul (modelView, theMat, pf_re_get_modelview());
      pfmat_mul (modelView, pfri->current_trans, modelView);
      pfmat_mul (modelView, modelView, pf_re_get_projection ());

      tc = pf_mesh_get_texcoord (pfri->mesh[0]);
      pf_v = pf_mesh_get_vertex (pfri->mesh[0]);
      
      tc[0] = 0.0;
      tc[1] = 0.0;
      
      tc[2] = 1.0;
      tc[3] = 0.0;
      
      tc[4] = 0.0;
      tc[5] = 1.0;
      
      tc[6] = 1.0;
      tc[7] = 1.0;
      
      // Update vertex on mesh
      memcpy (pf_v, rtp->lrh[real_line]->vertex, sizeof(float) * 12);
      
      glBindTexture (GL_TEXTURE_2D, rtp->lrh[real_line]->texid[0]);
      
      pf_mesh_render (pfri->mesh[0], modelView);

      //y += -rtp->lrh[real_line]->vertex[1] / 2.0;
      //y += -rtp->lrh[real_line]->vertex[1];
      //y += -rtp->interline;
      y += -rtp->interline;
    }

  glBindTexture (GL_TEXTURE_2D, 0);
  glDisable (GL_BLEND);

  glEnable (GL_DEPTH_TEST);
  return 0;  
}


int
pf_stext_free (PF_RITEM ri)
{
  RITEM_STEXT_PRIV priv; 

  if (!ri) return -1;

  priv = pf_ritem_get_priv_data (ri);
  pf_stext_clear (ri);
  pf_ritem_free (priv->highlight);
  free (priv);

  return 0;
}

PF_RITEM
pf_stext_new (char *font, char *text)
{
  PF_RITEM        ri; 
  PF_MESH         the_mesh;
  RITEM_STEXT_PRIV priv; 
  int             prg;
  GLfloat         pfVertices[12];
  GLfloat         pfNormals[12];
  GLfloat         pfColors[16];
  float           tc[8];

  prg = get_shader ("basic-text");

  ri = pf_ritem_new ();
  ri->id = PFRI_STEXT;
  pf_ritem_set_free_func (ri, pf_stext_free);

  the_mesh = pf_mesh_new (4, 4);

  pf_mesh_set_vertex (the_mesh, pfVertices);
  pf_mesh_set_normal (the_mesh, pfNormals);
  pf_mesh_set_color (the_mesh, pfColors);
  pf_mesh_set_texcoord (the_mesh, tc);


  pf_ritem_set_mesh (ri, the_mesh);

  // use the first text unit to pass font texture
  glUniform1i(glGetUniformLocation(prg, "sampler2d"), 0);

  pf_ritem_set_prg (ri, prg);
  pf_ritem_set_render_func (ri, pf_stext_render);

  // XXX: Depth test initially disabled for text rendering
  //      For non proportional fonts letter may overlap producing
  //      some artifacts
  pf_ritem_set_depth_test (ri, 0);

  // Set item data
  priv = malloc (sizeof (struct ritem_stext_priv_t));
  memset (priv, 0, sizeof (struct ritem_stext_priv_t));
  priv->font = pf_get_font (font);
  priv->lines = 0;
  priv->interline = -0.02;
  priv->indent1 = 0.0;
  priv->indent2 = 0.0;
  priv->para_space = 0.0;
  priv->w = -1;

  priv->ts = 0.4;
  
  priv->current_line = 0;
  priv->lines_up = 0;
  priv->lines_down = -1; /* Draw all the lines down*/

  priv->highlight = NULL;
  priv->lrh = NULL;
  if (text)
    {
      priv->lines = 1;
      priv->text = malloc (sizeof(char*));
      priv->text[0] = strdup (text);
 
    }
  priv->scale_x = priv->scale_y = 120.0;
  pf_ritem_set_priv_data (ri, priv);

  if (text)
    pf_stext_add_lrh (ri);

  

  return ri;
}

int      
pf_stext_set_scale (PF_RITEM pfri, float scale_x, float scale_y)
{
  RITEM_STEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  /* Only positive (non-zero) values alowed */
  if (scale_x > 0.0 )
    priv->scale_x = scale_x;
  if (scale_y > 0.0)
    priv->scale_y = scale_y;

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}


int      
pf_stext_set_width (PF_RITEM pfri, float w)
{
  RITEM_STEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  /* Only positive (non-zero) values alowed */
  if (w > 0.0 )
    priv->w = w;

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}


int      
pf_stext_set_font (PF_RITEM pfri, char *font)
{
  RITEM_STEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  priv->font = pf_get_font (font);

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}


int      
pf_stext_set_interline (PF_RITEM pfri, float il)
{
  RITEM_STEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->interline = il;
  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}

int      
pf_stext_add_text_line (PF_RITEM pfri, char *text)
{
  RITEM_STEXT_PRIV priv; 
  if (!pfri) return -1;
  
  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->lines++;
  priv->text = realloc (priv->text, sizeof(char*) * priv->lines);
  priv->text[priv->lines - 1] = strdup (text);

  pf_ritem_set_priv_data (pfri, priv);
  pf_stext_add_lrh (pfri);
  return 0;
}

int      
pf_stext_clear (PF_RITEM pfri)
{
  RITEM_STEXT_PRIV priv; 
  int             i;

  if (!pfri) return -1;
  
  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  /* Free text memory */
  for (i = 0; i < priv->lines; i++)
    {
      free (priv->text[i]);
      if (priv->lrh[i])
	lrh_free (priv->lrh[i]);
    }

  free (priv->text);
  if (priv->lrh)
    free (priv->lrh);
  priv->lrh = NULL;
  priv->text = NULL;
  priv->lines = 0;

  pf_ritem_set_priv_data (pfri, priv);
  return 0;
}

int
pf_stext_get_color (PF_RITEM pfri, float* r, float* g, float* b, float *alpha)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  *r = priv->r;
  *g = priv->g;
  *b = priv->b;
  *alpha = priv->alpha;



  return 0;
}

int      
pf_stext_set_color (PF_RITEM pfri, float r, float g, float b, float alpha)
{
  RITEM_STEXT_PRIV priv; 
  float   *colors;
  int     i;
  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
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
    pf_quad_set_color (priv->highlight,  0.3, 0.99, 0.3, alpha);
  return 0;
}



int      
pf_stext_set_current_line (PF_RITEM pfri, int cl)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->current_line = cl;

  return 0;
}

int      
pf_stext_set_lines_up (PF_RITEM pfri, int cl)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->lines_up = cl;
  
  return 0;
}

int      
pf_stext_set_lines_down (PF_RITEM pfri, int cl)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->lines_down = cl;

  return 0;
}


int      
pf_stext_set_para (PF_RITEM pfri, float indent1, float indent2, float para)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->indent1 = indent1;
  priv->indent2 = indent2;
  priv->para_space = para;

  LOG ("Stext Para updated: %f %f %f\n", 
       priv->indent1, priv->indent2, priv->para_space);
  return 0;
}

int      
pf_stext_set_hl (PF_RITEM pfri, int flag)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  priv->hl = flag;
  if (priv->highlight) return 0;
  LOG ("Creating Highlight Quad\n");
  priv->highlight = pf_quad_new (20.0, 2.0);
  pf_ritem_set_color (priv->highlight, 0.3, 0.99, 0.3, 0.99);
  priv->hl_x = -0.1;
  priv->hl_y = -1.95;
  priv->hl_z = -4.41;
  pf_ritem_set_pos (priv->highlight, pfri->x, pfri->y, pfri->z - 0.01);
  //pf_ritem_set_pos (priv->highlight, -0.1, -1.95, -4.41);
  return 0;
}

/* Line render hints functions */
LRH
lrh_new ()
{
  LRH aux;

  if ((aux = malloc (sizeof(struct line_render_hint_t))) == NULL)
    {
      fprintf (stderr, "Cannot allocate line render hint\n");
      return NULL;
    }
  aux->texid[0] = 0;
  aux->n_sublines = 0;
  aux->y_sublines = NULL;
  aux->flag = 0;
  return aux;
}

int
lrh_free (LRH l)
{
  if (!l) return -1;
  if (l->texid >= 0) 
    if (glIsTexture (l->texid[0]))
      {
	LOG ("Deletting texture %d\n", l->texid[0]);
	glDeleteTextures (1, (GLuint *)l->texid);
      }
  l->texid[0] = 0;

  free (l);

  return 0;
}


int
pf_stext_add_lrh (PF_RITEM pfri)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);

  LOG ("Add Line Rendering Hint (%d)\n", priv->lines);
  // Reallocate LRH array
  if ((priv->lrh = realloc (priv->lrh, sizeof (LRH) *priv->lines)) == NULL)
    {
      fprintf (stderr, "Cannot realloc line render hints array\n");
      exit (1);
    }
  // Add a new LRH object to the array
  priv->lrh[priv->lines - 1] = lrh_new ();
  return 0;
}

int      
pf_stext_set_hl_color (PF_RITEM pfri, float r, float g, float b, float alpha)
{
  RITEM_STEXT_PRIV priv; 

  if (!pfri) return -1;

  priv = (RITEM_STEXT_PRIV) pf_ritem_get_priv_data (pfri);
  if (!priv->highlight) return -1;

  pf_ritem_set_color (priv->highlight, r, g, b, alpha);
  return 0;
}

