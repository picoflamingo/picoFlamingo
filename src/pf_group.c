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

#include "pf_group.h"
#include "log.h"

/* Need access to render list */
//extern PF_RITEM ri_list[MAX_RITEMS];
//extern int      n_ri_list;

typedef struct ritem_group_priv_t
{
  PF_MATRIX44 old_modelview;
  int         n_children;
  PF_RITEM    *child;
} *RITEM_GROUP_PRIV;


int
pf_group_ritem_free (PF_RITEM ri)
{
  RITEM_GROUP_PRIV priv;

  priv = pf_ritem_get_priv_data (ri);
  if (!priv) return -1;

  if (priv->child) free (priv->child);
  free (priv);

  return 0;
}

int      
pf_group_render     (PF_RITEM pfri)
{
  RITEM_GROUP_PRIV   priv;
  PF_MATRIX44        theMat;
  PF_MATRIX44        modelView;
  int                i;

  if (!pfri) return -1;
  if (!pfri->show) return -1;
  priv = (RITEM_GROUP_PRIV) pf_ritem_get_priv_data (pfri);

  if (priv->n_children == 0) return 0;
  /* Store old modelview matrix */
  memcpy (priv->old_modelview, pf_re_get_modelview(), sizeof(PF_MATRIX44));
  /* Calculate group matrix transform */
  pfmat_identity (modelView);
  pfmat_identity (theMat);
  
  pfmat_mul (modelView, theMat, pf_re_get_modelview());
  pfmat_mul (modelView, pfri->current_trans, modelView);

  pf_ritem_set_priv_data (pfri, priv);
  /* Update system modelview matrix with group update transform */
  pf_re_set_modelview (modelView);
  /* Render childs */
  for (i = 0; i < priv->n_children; i++)
    priv->child[i]->render (priv->child[i]);
  /* Retore old modelview matrix */
  pf_re_set_modelview (priv->old_modelview);
}

PF_RITEM 
pf_group_new        ()
{
  PF_RITEM           ri;
  RITEM_GROUP_PRIV   priv;

  printf ("Creating Group...\n");
  ri = pf_ritem_new ();
  ri->id = PFRI_GROUP;

  priv = malloc (sizeof (struct ritem_group_priv_t));
  priv->n_children = 0;
  priv->child = NULL;
  pf_ritem_set_priv_data (ri, priv);
  pf_ritem_set_render_func (ri, pf_group_render);
  pf_ritem_set_free_func (ri, pf_group_ritem_free);

  printf ("Group Created...\n");
  return ri;
}

int      
pf_group_add_child  (PF_RITEM pfri, PF_RITEM child)
{
  RITEM_GROUP_PRIV   priv;
  int                indx;

  if (!pfri) return -1;
  if (!child) return -1;

  printf ("Adding Child %p (%s) to group %s...\n", child, child->name, pfri->name);
  priv = (RITEM_GROUP_PRIV) pf_ritem_get_priv_data (pfri);

  /* Add child */
  indx = priv->n_children;
  priv->n_children++;
  priv->child = realloc (priv->child, sizeof(PF_RITEM) * priv->n_children);
  priv->child[indx] = child;

  pf_ritem_set_group (child, 1);

  pf_ritem_set_priv_data (pfri, priv);
  printf ("Item added\n");
  return 0;
}
