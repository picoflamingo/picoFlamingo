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

#include "support/stb_image.h"

#include "log.h"
#include "pf_tex_mng.h"


extern char   *pres_dir;

typedef struct pf_tmng_t
{
  int    n;
  int    size;
  PF_TEX *tex;
} *PF_TMNG;

static PF_TMNG _pf_tmng;

int 
pf_tmng_init ()
{
  printf ("+ pifcoFlamingo Texture Manager initialised\n");
  _pf_tmng = malloc (sizeof (struct pf_tmng_t));
  _pf_tmng->size = _pf_tmng->n = 0;
  _pf_tmng->tex = NULL;

  return 0;
}

int pf_tmng_end ()
{
  int  i;
  /* Delete textures */
  for (i = 0; i < _pf_tmng->size; i++)
    {
      if (_pf_tmng->tex[i].texid < 0) continue;
      glDeleteTextures  (1, &(_pf_tmng->tex[i].texid));
    }
  free (_pf_tmng->tex);
  _pf_tmng->tex = NULL;
  _pf_tmng->size = _pf_tmng->n = 0;
  return 0;
}

int
_pf_tmng_find_tex (char *name)
{
  int  i;

  for (i = 0; i < _pf_tmng->size; i++)
    {
      if (_pf_tmng->tex[i].texid == -1) continue;
      if (!strcmp (name, _pf_tmng->tex[i].id))
	return i;
    }
  return -1;


}

int 
pf_tmng_add (char *name, int flag,  int w, int h, char *data)
{
  int    i, indx, tmp;

  /* First check if the name already exist */
  if ((indx = _pf_tmng_find_tex (name)) >= 0)
    {
      _pf_tmng->tex[indx].ref++;
      printf ("Texture '%s' already exists... returning id (%d)\n", 
	      name, _pf_tmng->tex[indx].texid);
      return _pf_tmng->tex[indx].texid;
    }

  /* Locate hole and realloc if needed */
  if (_pf_tmng->size == _pf_tmng->n)
    {
      /* Buffer is full, realloc */
      _pf_tmng->size++;
      _pf_tmng->tex = realloc (_pf_tmng->tex, sizeof (PF_TEX) * _pf_tmng->size);
      indx = _pf_tmng->n;
    }
  else
    {
      /* Find a hole */
      for (i = 0; i < _pf_tmng->size; i++)
	if (_pf_tmng->tex[i].texid == -1)
	  break;
      indx = i;
    }

  /* Update new texture entry */
  _pf_tmng->tex[indx].id = strdup (name);
  _pf_tmng->tex[indx].w = w;
  _pf_tmng->tex[indx].h = h;
  _pf_tmng->tex[indx].mutable = flag;
  _pf_tmng->tex[indx].ref = 1;
  _pf_tmng->n++;

  glBindTexture   (GL_TEXTURE_2D, 0);

  /* Create OpenGL texture and return it*/
  glGenTextures (1, &(_pf_tmng->tex[indx].texid));
  GLenum errCode;
  const GLubyte *errString;
  
  if ((errCode = glGetError()) != GL_NO_ERROR) {
    //errString = gluErrorString(errCode);
    //fprintf (stderr, "OpenGL Error: %s\n", errString);
    fprintf (stderr, "OpenGL Error: %d\n", errCode);
  }

  printf ("Produced tex id (%d/%d - %d): %d (%s) (%dx%d)\n", 
	  _pf_tmng->n, _pf_tmng->size, indx,
	  _pf_tmng->tex[indx].texid, _pf_tmng->tex[indx].id, w, h);

  glBindTexture   (GL_TEXTURE_2D, _pf_tmng->tex[indx].texid);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h,
		0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return _pf_tmng->tex[indx].texid;
}

int
pf_tmng_add_from_file (char *name)
{
  unsigned char *img, *data;
  char          buffer[1024];
  int           w, h, comp, stride1, y, texid;

  comp = 4;
  if (*name != '/')
    sprintf (buffer, "%s/%s", pres_dir, name);
  else
    sprintf (buffer, "%s", name);

  img = stbi_load (buffer, &w, &h, &comp, 4);

  texid = pf_tmng_add (name, 1, w, h, img);
  

  free (img);
  return texid;

}

int 
pf_tmng_update (char *name, int x, int y, int w, int h, char *data)
{
  fprintf (stderr, "Not yet implemented\n");
  return 0;
}

int pf_tmng_unref (char *name)
{
  int i;

  if (!name) return -1;
  if ((i = _pf_tmng_find_tex (name)) >= 0)
    {
      _pf_tmng->tex[i].ref--;
      if (_pf_tmng->tex[i].ref == 0)
	{
	  printf ("Deleting texture by name %d (%s) (GL id: %d)\n", 
		  i, _pf_tmng->tex[i].id, _pf_tmng->tex[i].texid);
	  if (_pf_tmng->tex[i].texid != -1)
	    {
	      glDeleteTextures (1, &(_pf_tmng->tex[i].texid));
	      _pf_tmng->tex[i].texid = -1;
	    }
	  else
	    fprintf (stderr, "Oops. Invalid texture (value -1)\n");
	  _pf_tmng->n--;
	}
      return 0;
    }

  return -1;
}


int 
pf_tmng_unref_texid (GLuint texid)
{
  int   i;

  for (i = 0; i < _pf_tmng->size; i++)
    {
      if (_pf_tmng->tex[i].texid < 0) continue;
      if (_pf_tmng->tex[i].texid == texid)
	{
	  _pf_tmng->tex[i].ref--;
	  if (_pf_tmng->tex[i].ref == 0)
	    {
	      printf ("Deleting texture by id %d (%s) (GL id: %d)\n", 
		      i, _pf_tmng->tex[i].id, _pf_tmng->tex[i].texid);
	      glDeleteTextures (1, &(_pf_tmng->tex[i].texid));
	      _pf_tmng->tex[i].texid = -1;
	      _pf_tmng->n--;
	    }
	  return 0;
	  
	}
    }
}

int pf_tmng_get (char *name)
{
  int  i;

  if (!name) return -1;
  if ((i = _pf_tmng_find_tex (name)) >= 0)
    {
      _pf_tmng->tex[i].ref++;
      return _pf_tmng->tex[i].texid;
    }
  return 0;
}

int 
pf_tmng_get_size (char *name, int *w, int *h)
{
  int  i;

  if (!name) return -1;
  if ((i = _pf_tmng_find_tex (name)) >= 0)
    {

      *w = _pf_tmng->tex[i].w;
      *h = _pf_tmng->tex[i].h;
      return 0;
    }
  return -1;
}
