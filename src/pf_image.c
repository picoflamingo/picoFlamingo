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

#include "video_net.h"

#include "support/stb_image.h"

#include "log.h"


extern int    screen_w;
extern int    screen_h;
extern char   *pres_dir;

typedef struct ritem_img_priv_t
{
  GLuint  texid;
  char    *fname;
  char    *data;
  float   texcoord[4];
  int     w, h;
  int     w1, h1;
  int     update;
  VIDEO_STREAM s;
  int     save;
  int     center;
  float   scale;
  char    *shot_name;
  int     auto_adjust;
} *RITEM_IMG_PRIV;


extern float m_fAngle;
static int _photo_cnt = 1;


void
pf_image_calculate_quad (int center, int w, int h, 
			 float *x0, float *y0, float *x1, float *y1)
{
  float r1;

  r1 = ((float) h / (float) w);	  	  
  if (center)
    {      
      *x0 = -0.5;
      *y0 = -0.5 * r1;
      
      *x1 = 0.5;
      *y1 = 0.5 * r1;
      
    }
  else
    {
      
      *x0 = 0.0;
      *y0 = 0.0;
      
      *x1 = 1.0;
      *y1 = r1;	  
    }
  
}



#define MX_X 0.75
#define MN_X -0.75

void
pf_image_calculate_quad1 (int center, int w, int h, 
			 float *x0, float *y0, float *x1, float *y1)
{
  float r1, r2, r;


  r1 = ((float) h / (float) w);
  r2 = ((float) w / (float) h);
  if (center)
    {      
      if (w < h)
	{
	  *x0 = -0.5;
	  *y0 = -0.5 * r1;
	  
	  *x1 = 0.5;
	  *y1 = 0.5 * r1;
	}
      else
	{
	  /*
	  *y0 = -0.5;
	  *x0 = -0.5 * r2;
	  
	  *y1 = 0.5;
	  *x1 = 0.5 * r2;
	  */

	  *y0 = MN_X;
	  *x0 = MN_X * r2;
	  
	  *y1 = MX_X;
	  *x1 = MX_X * r2;

	}
      
    }
  else
    {
      
      *x0 = 0.0;
      *y0 = 0.0;
      
      *x1 = 1.0;
      *y1 = r1;	  
    }
  
}


int
_pf_image_update_from_file (PF_RITEM ri)
{
  RITEM_IMG_PRIV priv;
  int             stride1, stride2, y;
  float           x0, y0, x1, y1, texMaxX, texMaxY, quadX, quadY, r1;
  int             w, h, w2, h2 ,comp, i;
  unsigned char   *img;
  char            buffer[1025], *file;

  priv = pf_ritem_get_priv_data (ri);
  file = strdup (priv->fname);

  printf ("++ render thread update image '%s'\n", file);

  if (*file != '/')
    sprintf (buffer, "%s/%s", pres_dir, file);
  else
    sprintf (buffer, "%s", file);
  if (priv->fname) free (priv->fname);
  priv->fname = strdup (buffer);

  printf ("Inner Updating %s with file:%s (%s)\n", ri->name, file, buffer);

  priv->texid = pf_tmng_add_from_file (file);
  pf_tmng_get_size (file, &priv->w, &priv->h);
  pf_ritem_set_priv_data (ri, priv);
  printf ("Texid: %d (%dx%d) (auto:%d\n", priv->texid, priv->w, priv->h, priv->auto_adjust);

  /* Update Image */
  if (priv->auto_adjust)
    pf_image_calculate_quad1 (priv->center, priv->w, priv->h, &x0, &y0, &x1, &y1);
  else
    pf_image_calculate_quad (priv->center, priv->w, priv->h, &x0, &y0, &x1, &y1);

  GLfloat pfVertices[] = {-1.0,  -1.0, 0.0f, 
			   1.0,  -1.0, 0.0f, 
			  -1.0,  1.0, 0.0f,
                           1.0,  1.0, 0.0f};


  pfVertices[0]  = x0;
  pfVertices[1]  = y0;
  pfVertices[3]  = x1;
  pfVertices[4]  = y0;
  pfVertices[6]  = x0;
  pfVertices[7]  = y1;
  pfVertices[9]  = x1;
  pfVertices[10] = y1;

  
  printf("Image %s QUAD (%d,%d): %f, %f - %f, %f\n", 
	  priv->fname, priv->w, priv->h,
	  x0, y0, x1, y1);
  
  for (i = 0; i < 12; i ++) pfVertices[i] *= priv->scale;

  pf_mesh_set_vertex (pf_ritem_get_mesh (ri), pfVertices);

  pf_ritem_set_priv_data (ri, priv);
  free (file);
  return 0;

}


int
pf_image_render (PF_RITEM pfri)
{
  PF_MATRIX44    theMat;
  PF_MATRIX44    modelView;
  RITEM_IMG_PRIV rtp;
  int            i, n;
  float v[3], w[3], *a, *b;
  

  if (!pfri->show) return 0;
  glUseProgram(pfri->program);

  rtp = (RITEM_IMG_PRIV) pf_ritem_get_priv_data (pfri);

  pfmat_identity (modelView);
  pfmat_identity (theMat);
  
  pfmat_mul (modelView, theMat, pf_re_get_modelview());
  pfmat_mul (modelView, pfri->current_trans, modelView);

  pfmat_mul (modelView, modelView, pf_re_get_projection ());
  

  /* v and w are my bounding box in 2D */  

  if (pfri->depth)
    glEnable (GL_DEPTH_TEST);
  else
    glDisable (GL_DEPTH_TEST);

  glEnable (GL_BLEND);
  glBlendEquation (GL_FUNC_ADD);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  
  glBindTexture (GL_TEXTURE_2D, rtp->texid);

  if (rtp->update == 1)
    {
      LOG ("Updating image (%dx%d)\n", rtp->w1, rtp->h1);
      if (rtp->data)
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 
			 rtp->w1, rtp->h1, 
			 GL_RGBA, GL_UNSIGNED_BYTE, rtp->data);

      rtp->update = 0;
    }

  if (rtp->update == 2)
    {
      LOG ("Updating image From file (%s)\n", rtp->fname);
      _pf_image_update_from_file (pfri);
      rtp->update = 0;
    }




  glUseProgram(pf_mesh_get_program (pfri->mesh[0]));
  glUniform1i(glGetUniformLocation (pf_mesh_get_program (pfri->mesh[0]), 
				   "sampler2d"), 0);
  
  if (pfri->clickable && pfri->dirty_bound)
    pf_ritem_calculate_bb (pfri, modelView);

  pf_mesh_render (pfri->mesh[0], modelView);
  
  
  glBindTexture (GL_TEXTURE_2D, 0);
  glDisable (GL_BLEND);
  
  return 0;  
}


int
pf_image_ritem_free (PF_RITEM ri)
{
  RITEM_IMG_PRIV priv;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  //glDeleteTextures (1, &priv->texid);
  pf_tmng_unref_texid (priv->texid);
  if (priv->data) free (priv->data);
  free (priv);

  return 0;
}

#if 0
int
pf_image_set_data1 (PF_RITEM ri, char *data)
{
  RITEM_IMG_PRIV priv;
  int             stride1, stride2, y,x , loff;

  priv = pf_ritem_get_priv_data (ri);

  LOG ("Setting image data %p (%d, %d) (%d, %d)\n", data,
	  priv->w, priv->h, priv->w1, priv->h1);

  // Produce the texture
  if (priv->data) free (priv->data);
  priv->data = NULL;
  priv->data = malloc (priv->w1 * priv->h1 * 4);
  memset (priv->data, 0, priv->w1 * priv->h1 * 4);

  LOG ("Allocated %d bytes (%d Mb)\n", priv->w1 * priv->h1 * 4,
	  priv->w1 * priv->h1 * 4 /(1024 * 1024) );

  stride1 = priv->w * 3;
  stride2 = priv->w * 4;

  for (y = 0; y < priv->h; y++)
    {
      loff = y * stride2;
      for (x = 0; x < priv->w; x++)
	{

	  *(priv->data + loff + x * 4 + 0) = *(data + y * stride1 + x * 3 + 0);
	  *(priv->data + loff + x * 4 + 1) = *(data + y * stride1 + x * 3 + 1);
	  *(priv->data + loff + x * 4 + 2) = *(data + y * stride1 + x * 3 + 2);
	  *(priv->data + loff + x * 4 + 3) = 255;

      }
    }

  LOG ("Image produced with size: %d (texid:%d)\n", 
	  priv->w1 * priv->h1 * 4, priv->texid);

  priv->update = 1;

  free (data);
  return 0;
}

#endif

int
pf_image_set_data (PF_RITEM ri, unsigned char *data, int len)
{
  RITEM_IMG_PRIV priv;
  int             w, h, comp;
  unsigned char   *img;

  priv = pf_ritem_get_priv_data (ri);

  /* Check if images has already been loaded */


  /* Otherwise, create a new texture and assign to image */
  // Load Image
  comp = 4;
  img = stbi_load_from_memory ((unsigned char*)data, len, &w, &h, &comp, 4);

  if (!img) return -1;



  if (priv->data) free (priv->data);
  priv->data = (char *)img;
  priv->update = 1;

  return 0;

}

int
pf_image_save (PF_RITEM pfri)
{
  RITEM_IMG_PRIV priv;
  char           tmp[1024];

  if (!pfri) return -1;
  priv = pf_ritem_get_priv_data (pfri);
  snprintf (tmp, 1024, "%s/img-%05d.jpg", pres_dir, _photo_cnt);

  if (priv->shot_name) free (priv->shot_name);
  priv->shot_name = strdup (tmp);


  priv->save = 1;
  return 0;
}


int
pf_image_save_as (PF_RITEM pfri, char *fname)
{
  RITEM_IMG_PRIV priv;
  char           tmp[1024];

  if (!pfri) return -1;
  priv = pf_ritem_get_priv_data (pfri);
  snprintf (tmp, 1024, "%s/%s", pres_dir, fname);

  if (priv->shot_name) free (priv->shot_name);
  priv->shot_name = strdup (tmp);

  priv->save = 1;
  return 0;
}


int
pf_image_get_frame (PF_RITEM pfri)
{
  RITEM_IMG_PRIV  priv;
  unsigned char   *frame;
  int             len, _the_len;
  char            name[1024];
  FILE            *f;

  priv = pf_ritem_get_priv_data (pfri);

  /* Close previous connection if any */
  if (priv->s == NULL) return 0;
  
  frame = get_live_frame (priv->s, &len);
  _the_len = len;
  if (frame)
    {
      if (priv->save && priv->shot_name)
	{
#if 0
	  if (!priv->shot_name)
	    snprintf (name, 1024, "%s/img-%05d.jpg", pres_dir, _photo_cnt);
	  else
	    strncpy (name, 1024, priv->shot_name);
#endif
	  fprintf (stderr, "Saving Image '%s' (%d bytes)\n", name, _the_len);
	  if ((f = fopen (priv->shot_name, "wb")) != NULL)
	    {
	      fwrite (frame, _the_len, 1, f);
	      fclose (f);
	      _photo_cnt++;
	      priv->save = 0; // Reset save flag
	      fflush (f);
	    }
	}
      pf_image_set_data  (pfri, frame, len);

    }

  return 0;
}

int      
pf_image_reconnect (PF_RITEM pfri, char *ip_str, int port)
{
  RITEM_IMG_PRIV   priv;

  priv = pf_ritem_get_priv_data (pfri);

  /* Connect to new resource */
  LOG ("Reconnection Image '%s' to %s:%p\n",
	  pfri->name, ip_str, port);
  
  if (pfri->auto_update) disconnect_server (priv->s);
  priv->s = connect_server (ip_str, port);

  /* Update image callback */
  if (priv->s)
    pfri->auto_update = pf_image_get_frame;
  
  return 0;
}

int
pf_image_set_data_from_file (PF_RITEM ri, char *file)
{
  RITEM_IMG_PRIV priv;
  int             stride1, stride2, y;
  float           x0, y0, x1, y1, texMaxX, texMaxY, quadX, quadY, r1;
  int             w, h, w2, h2 ,comp, i;
  unsigned char   *img;
  char            buffer[1025];

  priv = pf_ritem_get_priv_data (ri);

  /* Unref old texture */
  pf_tmng_unref (priv->fname);

  if (priv->fname) free (priv->fname);

  priv->fname = strdup (file);
  priv->update = 2;
  printf ("Deferrer texture update for image: '%s'\n", priv->fname);
#if 0
  sprintf (buffer, "%s/%s", pres_dir, file);
  if (priv->fname) free (priv->fname);
  priv->fname = strdup (buffer);

  printf ("Updating %s with file:%s (%s)\n", ri->name, file, buffer);

  comp = 4;
  sprintf (buffer, "%s/%s", pres_dir, file);
  img = stbi_load (buffer, &w, &h, &comp, 4);

  priv->texid = pf_tmng_add (file, 1, w, h, img);

  //priv->texid = pf_tmng_add_from_file (file);
  priv->data =img;
  pf_tmng_get_size (file, &priv->w, &priv->h);
  pf_ritem_set_priv_data (ri, priv);
  printf ("Texid: %d (%dx%d) (%d)\n", priv->texid, priv->w, priv->h, priv->auto_adjust);


  priv->update = 1;

  /* Update Image */

  if (priv->auto_adjust)
    pf_image_calculate_quad1 (priv->center, priv->w, priv->h, &x0, &y0, &x1, &y1);
  else
    pf_image_calculate_quad (priv->center, priv->w, priv->h, &x0, &y0, &x1, &y1);

  GLfloat pfVertices[] = {-1.0,  -1.0, 0.0f, 
			   1.0,  -1.0, 0.0f, 
			  -1.0,  1.0, 0.0f,
                           1.0,  1.0, 0.0f};


  pfVertices[0]  = x0;
  pfVertices[1]  = y0;
  pfVertices[3]  = x1;
  pfVertices[4]  = y0;
  pfVertices[6]  = x0;
  pfVertices[7]  = y1;
  pfVertices[9]  = x1;
  pfVertices[10] = y1;

  
  printf("Image %s QUAD (%d,%d): %f, %f - %f, %f\n", 
	  priv->fname, priv->w, priv->h,
	  x0, y0, x1, y1);
  
  for (i = 0; i < 12; i ++) pfVertices[i] *= priv->scale;

  pf_mesh_set_vertex (pf_ritem_get_mesh (ri), pfVertices);
#endif
  pf_ritem_set_priv_data (ri, priv);
  return 0;

}

int
pf_image_disconnect (PF_RITEM pfri)
{
  RITEM_IMG_PRIV   priv;

  priv = pf_ritem_get_priv_data (pfri);

  /* Close previous connection if any */
  disconnect_server (priv->s);

  /* Set default image */
  pfri->auto_update = NULL;

  return 0;
}


PF_RITEM
pf_image_new (char *fname, float scale, int center)
{
  PF_RITEM        ri; 
  PF_MESH         the_mesh;
  RITEM_IMG_PRIV  priv; 
  float           x0, y0, x1, y1, texMaxX, texMaxY, quadX, quadY, r1;
  int             prg, w, h, w2, h2 ,comp, i, y;
  int             stride1, stride2;
  unsigned char   *img;
  char            buffer[1024];

  GLfloat pfNormals[] = { 0.0, 0.0, 1.0f, 
			  0.0, 0.0, 1.0f, 
			  0.0, 0.0, 1.0f,
                          0.0, 0.0, 1.0f};

  
  GLfloat pfColors[] = {1.0f, 1.0f, 1.0f, 1.0,  
			1.0f, 1.0f, 1.0f, 1.0,  
			1.0f, 1.0f, 1.0f, 1.0,
			1.0f, 1.0f, 1.0f, 1.0
  };
  
  float tc[] ={0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0 };

  if ((prg = get_shader ("basic-text")) < 0)
    {
      fprintf (stderr, "Cannot load shader 'basic-text\n");
      return NULL;
    }

  ri = pf_ritem_new ();
  ri->id = PFRI_IMAGE;

  pf_ritem_set_free_func (ri, (void*) pf_image_ritem_free);

  the_mesh = pf_mesh_new (4, 4);

  // Set item data
  priv = malloc (sizeof (struct ritem_img_priv_t));
  sprintf (buffer, "%s/%s", pres_dir, fname);
  priv->fname = strdup (buffer);
  priv->update = 0;
  priv->s = NULL;
  priv->auto_adjust = 0;

  priv->texid = pf_tmng_add_from_file (fname);
  pf_tmng_get_size (fname, &priv->w, &priv->h);

  w = priv->w;
  h = priv->h;
  priv->w1 = w;
  priv->h1 = h;
  priv->shot_name = NULL;
  // Produce the texture
  priv->save = 0;


  pf_ritem_set_priv_data (ri, priv);

  //Update mapping coordinates and vertex

  GLfloat pfVertices[] = {-1.0,  -1.0, 0.0f, 
			   1.0,  -1.0, 0.0f, 
			  -1.0,  1.0, 0.0f,
                           1.0,  1.0, 0.0f};


  priv->center = center;
  pf_image_calculate_quad (center, w, h, &x0, &y0, &x1, &y1);
  pfVertices[0]  = x0;
  pfVertices[1]  = y0;
  pfVertices[3]  = x1;
  pfVertices[4]  = y0;
  pfVertices[6]  = x0;
  pfVertices[7]  = y1;
  pfVertices[9]  = x1;
  pfVertices[10] = y1;

  LOG("Image %s QUAD (%f-%d,%f-%d): %f, %f - %f, %f\n", 
	  priv->fname, texMaxX, w, texMaxY, h,
	  x0, y0, x1, y1);

  priv->scale = scale;
  for (i = 0; i < 12; i ++) pfVertices[i] *= scale;

  pf_mesh_set_vertex (the_mesh, pfVertices);
  pf_mesh_set_normal (the_mesh, pfNormals);
  pf_mesh_set_color (the_mesh, pfColors);
  pf_mesh_set_texcoord (the_mesh, tc);


  pf_ritem_set_mesh (ri, the_mesh);

  // use the first text unit to pass font texture
  glUniform1i(glGetUniformLocation(prg, "sampler2d"), 0);

  pf_ritem_set_prg (ri, prg);
  pf_ritem_set_render_func (ri, pf_image_render);

  priv->data = NULL;

  return ri;
}




int      
pf_image_set_color (PF_RITEM pfri, float r, float g, float b, float alpha)
{
  float   *colors;
  int     i;
  if (!pfri) return -1;

  LOG ("Changing image color...\n");
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

  return 0;
}


int      
pf_image_set_auto (PF_RITEM pfri, int flag)
{
  int     i;
  RITEM_IMG_PRIV   priv;


  if (!pfri) return -1;
  priv = pf_ritem_get_priv_data (pfri);


  LOG ("Setting image autoadjust...\n");
  // build color array
  priv->auto_adjust = flag;


  return 0;
}
