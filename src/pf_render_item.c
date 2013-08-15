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

/* Render Items Implementation
 * ---------------------------------------------------------------------
 * Abstract object representing any slide item in the presentation
 * All slide items (text, images, 3D objects) are managed as render items
 * from the main application
 */

/* Generic Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* OpenGL ES includes */
#include <EGL/egl.h>
#include <GLES2/gl2.h>

/* Application specific includes */
#include "render_engine.h"
#include "shader.h"
#include "matrix.h"
#include "pf_mesh.h"

#include "pf_render_item.h"

/* Application generic includes */
#include "log.h"


/* Constructor/Destructor */
/* **************************************** */
PF_RITEM
pf_ritem_new ()
{
  PF_RITEM p;
  int      i;

  if ((p = malloc (sizeof(struct pf_ritem_t))) == NULL)
    {
      fprintf (stderr, "Cannot allocate Render item\n");
      return NULL;
    }
  memset (p, 0, sizeof (struct pf_ritem_t));
  p->id = PFRI_DEFAULT;
  p->mesh = NULL;
  p->priv = NULL;
  pfmat_identity (p->original_trans);
  pfmat_identity (p->current_trans);

  p->name = NULL;
  p->show = 1;

  p->render = pf_ritem_render;
  p->free = NULL;
  p->auto_update = NULL;
  p->save = NULL;

  p->n_meshes = 0;
  p->depth = 1;

  p->dirty_bound = 0;
  p->clickable = 0;
  p->group = -1;
  for (i = 0; i < 4; i++) p->bb2D[i] = 0.0;

  /* XXX: Create default mesh for Image and Text Items */

  /* Required to support old code during migration to the 
   * pf_mesh multimesh object 
   */
  pf_ritem_add_mesh (p, NULL);

  return p;
}


int      
pf_ritem_free (PF_RITEM pfri)
{
  int   i;

  if (!pfri) return -1;
  if (pfri->free) pfri->free (pfri);

  for (i = 0; i < pfri->n_meshes; i++)  
    pf_mesh_free (pfri->mesh[i]);

  free (pfri->mesh);

  /* XXX: Private data shall be freed in the upper layers */
  free (pfri);
  return 0;
}


/* Accessor functions */
/* **************************************** */
int      
pf_ritem_set_name (PF_RITEM pfri, char *name)
{
  if (!pfri) return -1;
  if (!name) return -1;

  if (pfri->name) free (pfri->name);
  pfri->name = strdup (name);

  return 0;
}


char*    
pf_ritem_get_name (PF_RITEM pfri)
{
  if (!pfri) return NULL;

  return pfri->name;
}


int      
pf_ritem_set_depth_test (PF_RITEM pfri, int flag)
{
  if (!pfri) return -1;

  pfri->depth = flag;

  return 0;
}


int
pf_ritem_get_depth_test (PF_RITEM pfri)
{
  if (!pfri) return -1;

  return pfri->depth;
}


int      
pf_ritem_set_pos (PF_RITEM pfri, float x, float y, float z)
{
  PF_MATRIX44 t;

  if (!pfri) return -1;

  pfri->x = x;
  pfri->y = y;
  pfri->z = z;

  pfmat_translate (t, x, y, z);
  pfmat_mul (pfri->current_trans, t, pfri->current_trans);

  return 0;
}


int      
pf_ritem_set_rot (PF_RITEM pfri, float a, float b, float c)
{
  PF_MATRIX44 r;

  if (!pfri) return -1;

  pfri->a = a;
  pfri->b = b;
  pfri->c = c;

  pfmat_rotation (r, a, b, c);
  pfmat_mul (pfri->current_trans, r, pfri->current_trans);

  return 0;
}


int      
pf_ritem_set_priv_data (PF_RITEM pfri, void *data)
{
  if (!pfri) return -1;

  pfri->priv = data;

  return 0;
}


void*    
pf_ritem_get_priv_data (PF_RITEM pfri)
{
  if (!pfri) return NULL;

  return pfri->priv;
}


int    
pf_ritem_set_group (PF_RITEM pfri, int indx)
{
  if (!pfri) return -1;

  pfri->group = indx;

  return 0;
}


int    
pf_ritem_get_group (PF_RITEM pfri)
{
  if (!pfri) return -1;

  return pfri->group;
}


int
pf_ritem_set_render_func (PF_RITEM pfri, PF_FUNC func)
{
  if (!pfri) return -1;
  if (!func) return -1;

  pfri->render = func;

  return 0;
}

 
int
pf_ritem_set_free_func (PF_RITEM pfri, PF_FUNC func)
{
  if (!pfri) return -1;
  if (!func) return -1;

  pfri->free = func;

  return 0;
} 

 
int      
pf_ritem_set_save_func (PF_RITEM pfri, PF_FUNC func)
{
  if (!pfri) return -1;
  if (!func) return -1;

  pfri->save = func;

  return 0;
}


int      
pf_ritem_set_prg (PF_RITEM pfri, int program)
{
  int   i;
	
  if (!pfri) return -1;

  pfri->program = program;

  for (i = 0; i < pfri->n_meshes; i++)
    pf_mesh_set_prg (pfri->mesh[i], program);

  return 0;
}

int
pf_ritem_add_mesh (PF_RITEM p, PF_MESH m)
{
  if (!p) return -1;

  if (p->mesh && p->mesh[0] == NULL)
    {
      printf ("First mesh update!!!\n");
      p->mesh[0] = m;

      return 0;
    }

  p->n_meshes++;
  if ((p->mesh = realloc (p->mesh, sizeof(PF_MESH) * p->n_meshes)) == NULL)
    {
      fprintf (stderr, "Cannot reasize mesh table\n");
      return -1;
    }
  p->mesh[(p->n_meshes - 1)] = m;

  return 0;
}


/* Legacy function. Uses mesh 0 for Image and Text */
int      
pf_ritem_set_mesh (PF_RITEM pfri, PF_MESH m)
{
  if (!pfri) return -1;
  if (!m) return -1;

  pfri->mesh[0] = m;

  return 0;
}


PF_MESH
pf_ritem_get_mesh (PF_RITEM pfri)
{
  if (!pfri) return NULL;

  return pfri->mesh[0];
}




/* Methods -- */
/* **************************************** */
int      
pf_ritem_render (PF_RITEM pfri)
{
  PF_MATRIX44 MVP;
  int         i, n;

  if (!pfri->show) return -1;
  pfmat_identity (MVP);

  /* Calculate Movel/View/Projection matrix */
  pfmat_mul (MVP, MVP, pf_re_get_modelview());
  pfmat_mul (MVP, pfri->current_trans, MVP);
  pfmat_mul (MVP, MVP, pf_re_get_projection ());

  glUseProgram(pfri->program);
  
  n = pfri->n_meshes;

  for (i = 0; i < n; i++)
    pf_mesh_render2 (pfri->mesh[i], MVP, pfri->current_trans);

  return 0;
}


int    
pf_ritem_show (PF_RITEM pfri, int flag)
{
  if (!pfri) return -1;

  pfri->show = flag;

  return 0;
}


/* Absolute positioning of items */
static int
_pf_update_trans (PF_RITEM pfri)
{
  PF_MATRIX44 t, r;

  pfmat_identity (pfri->current_trans);

  pfmat_translate (t, pfri->x, pfri->y, pfri->z);
  pfmat_mul (pfri->current_trans, t, pfri->current_trans);

  pfmat_rotation (r, pfri->a, pfri->b, pfri->c);
  pfmat_mul (pfri->current_trans, r, pfri->current_trans);

  return 0;
}


int      
pf_ritem_set_abs_pos (PF_RITEM pfri, float x, float y, float z)
{
  if (!pfri) return -1;

  pfri->x = x;
  pfri->y = y;
  pfri->z = z;

  _pf_update_trans (pfri);

  return 0;
}


int      
pf_ritem_get_abs_pos (PF_RITEM pfri, float *a, float *b, float *c)
{
  if (!pfri) return -1;

  *a = pfri->x;
  *b = pfri->y;
  *c = pfri->z;

  return 0;
}

/* XXX: Rotations as Euler Angles */
int      
pf_ritem_set_abs_rot (PF_RITEM pfri, float a, float b, float c)
{

  if (!pfri) return -1;

  pfri->a = a;
  pfri->b = b;
  pfri->c = c;

  _pf_update_trans (pfri);

  return 0;
}


int      
pf_ritem_get_abs_rot (PF_RITEM pfri, float *a, float *b, float *c)
{
  if (!pfri) return -1;

  *a = pfri->a;
  *b = pfri->b;
  *c = pfri->c;

  return 0;
}

/* set/get color on simple 2D objects (images, text, rectangles,...) */
int      
pf_ritem_set_color (PF_RITEM pfri, float r, float g, float b, float alpha)
{
  int     i;
  float   colors[16];

  if (!pfri) return -1;

  for (i = 0; i < 16; i += 4)
    {
      colors [i] = r;
      colors [i + 1] = g;
      colors [i + 2] = b;
      colors [i + 3] = alpha;
    }

  for (i = 0; i < pfri->n_meshes; i++)
    pf_mesh_set_color (pfri->mesh[i], colors);


  return 0;
}


int
pf_ritem_get_color (PF_RITEM ri, float *r, float *g, float *b, float *a)
{
  if (!ri) return -1;
  
  /* XXX: Should go through accessor function */
  *r = ri->mesh[0]->color[0];
  *g = ri->mesh[0]->color[1];
  *b = ri->mesh[0]->color[2];
  *a = ri->mesh[0]->color[3];

  return 0;
}


/* Bounding box for simple 2D objects */
int      
pf_ritem_calculate_bb (PF_RITEM pfri, PF_MATRIX44 model)
{
  int     i, j, n;
  float   maxx, maxy, minx, miny;
  float   v[3], *a;

  pfri->dirty_bound = 0;
  maxx = maxy = -1e6;
  minx = miny = 1e6;

  for (i = 0; i < pfri->n_meshes; i++)
    {
      n = pfri->mesh[i]->n_vertex;
      for (j = 0; j < n; j++)
	{
	  a = pfri->mesh[i]->vertex + (3 * j);
	  pfmat_vmul (v, model, a);

	  /* Update bounding box */
	  if (v[0] > maxx) maxx = v[0];
	  if (v[0] < minx) minx = v[0];
	  if (v[1] > maxy) maxy = v[1];
	  if (v[1] < miny) miny = v[1];
	}
    }

  /* Store BB together with render item */
  pfri->bb2D[0] = minx;
  pfri->bb2D[1] = maxx;
  pfri->bb2D[2] = miny;
  pfri->bb2D[3] = maxy;

  LOG("BB for item %s is X:(%f,%f) Y(%f,%f)\n",
	  pfri->name,
	  pfri->bb2D[0],
	  pfri->bb2D[1],
	  pfri->bb2D[2],
	  pfri->bb2D[3]
	  );

  return 0;
}


int      
pf_ritem_test_bb (PF_RITEM pfri, float x, float y)
{
  if (!pfri) return -1;
  if (!pfri->clickable) return 0;

  LOG ("Testing (%f,%f) -> (%f, %f) - (%f,%f)\n",
	  x, y,
	  pfri->bb2D[0],
	  pfri->bb2D[1],
	  pfri->bb2D[2],
	  pfri->bb2D[3]
	  );
  if ((x > pfri->bb2D[0]) && (x < pfri->bb2D[1]) && 
      (y > pfri->bb2D[2]) && (y < pfri->bb2D[3]))
    {
      return 1;
    }

  return 0;
}
