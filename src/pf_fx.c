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

#include <sys/time.h>
#include <time.h>

#include <unistd.h>
#include <math.h>

#include "pf_render_item.h"
#include "pfs_cmd.h" // Functions to find items!!!!
#include "pf_fx.h"
#include "pf_text.h"
#include "pf_stext.h"
#include "pf_image.h"

#include "const.h"
#include "log.h"

#define MAX_FXS 1024

extern PF_RITEM ri_list[MAX_RITEMS];

static PF_FX  fx_list[MAX_FXS];
static int    n_fx = 0;
static int    _init_timer = 1;

PF_FX  
pf_fx_new (int type)
{
  PF_FX  fx;
  int    i;
  
  if ((fx = malloc (sizeof(struct pf_fx_t))) == NULL)
    {
      fprintf (stderr, "Cannot allocate memory for PF_FX item\n");
      return NULL;
    }
  fx->item = NULL;
  fx->type = PF_FX_FRAME;
  fx->n_frames = 100;
  fx->remove_me = 0;
  fx->loop = 0;
  fx->use_colors = 1;
  fx->active = 0;

  for (i = 0; i < 4; i++)
    {
      fx->iter[i] = 0;
      fx->t_iter[i] = 0.0;
    }

  for (i = 0; i < 4; i++)
    fx->pos[i][0] = fx->pos[i][1] = fx->pos[i][2] =
      fx->rot[i][0] = fx->rot[i][1] = fx->rot[i][2] =
      fx->col[i][0] = fx->col[i][1] = fx->col[i][2] = fx->col[i][3] = 0.0;
      
  return fx;
}

int    
pf_fx_free (PF_FX fx)
{
  if (fx) free (fx);
  return 0;
}

int    
pf_fx_set_item (PF_FX fx, char *name)
{
  if (!fx) return -1;
  if (!name) return -1;
  
  fx->item = pfs_item_find (name);

  return 0;
}

int    
pf_fx_set_pos (PF_FX fx, int indx, float x, float y, float z)
{
  if (!fx) return -1;
  if (indx < 0 ) return -1;

  fx->pos[indx][0] = x;
  fx->pos[indx][1] = y;
  fx->pos[indx][2] = z;

  return 0;
}

int    
pf_fx_set_rot (PF_FX fx, int indx, float a, float b, float c)
{
  if (!fx) return -1;
  if (indx < 0 ) return -1;

  fx->rot[indx][0] = a;
  fx->rot[indx][1] = b;
  fx->rot[indx][2] = c;

  return 0;

}

int    
pf_fx_set_color (PF_FX fx, int indx, float r, float g, float b, float alpha)
{
  if (!fx) return -1;
  if (indx < 0 ) return -1;

  fx->col[indx][0] = r;
  fx->col[indx][1] = g;
  fx->col[indx][2] = b;
  fx->col[indx][3] = alpha;

  return 0;

}

int    
pf_fx_use_colors (PF_FX fx, int flag)
{
  if (!fx) return -1;
  fx->use_colors = flag;

  return 0;
}


int    
pf_fx_configure (PF_FX fx, int frame0, int n_frames, int n_cycle, int loop)
{
  int i;

  if (!fx) return -1;


  if (!fx->active)
    {
      fx->type = PF_FX_FRAME;
      if (n_frames < 0) n_frames = 0;
      fx->iter[PF_START] = frame0;
      fx->iter[PF_CURRENT] = 0;
      fx->iter[PF_END] = frame0 + n_cycle;
      fx->iter[PF_INC] = 1;

      fx->loop = loop;

      fx->n_frames = n_frames;
    }
  // Calculate increments
  // Position
  fx->pos[PF_INC][0] = (fx->pos[PF_END][0] - fx->pos[PF_START][0] )/ n_frames;
  fx->pos[PF_INC][1] = (fx->pos[PF_END][1] - fx->pos[PF_START][1] )/ n_frames;
  fx->pos[PF_INC][2] = (fx->pos[PF_END][2] - fx->pos[PF_START][2] )/ n_frames;

  fx->rot[PF_INC][0] = (fx->rot[PF_END][0] - fx->rot[PF_START][0] )/ n_frames;
  fx->rot[PF_INC][1] = (fx->rot[PF_END][1] - fx->rot[PF_START][1] )/ n_frames;
  fx->rot[PF_INC][2] = (fx->rot[PF_END][2] - fx->rot[PF_START][2] )/ n_frames;

  fx->col[PF_INC][0] = (fx->col[PF_END][0] - fx->col[PF_START][0] )/ n_frames;
  fx->col[PF_INC][1] = (fx->col[PF_END][1] - fx->col[PF_START][1] )/ n_frames;
  fx->col[PF_INC][2] = (fx->col[PF_END][2] - fx->col[PF_START][2] )/ n_frames;
  fx->col[PF_INC][3] = (fx->col[PF_END][3] - fx->col[PF_START][3] )/ n_frames;

  for (i = 0; i < 3; i++)
    {
      fx->pos[PF_CURRENT][i] = fx->pos[PF_START][i];
      fx->rot[PF_CURRENT][i] = fx->rot[PF_START][i];
      fx->col[PF_CURRENT][i] = fx->col[PF_START][i];
    }
  fx->col[PF_CURRENT][3] = fx->col[PF_START][3];

  LOG ("FX Configured!! (n_frames: %d)\n", n_frames);
  LOG ("Color: %f %f %f %f\n", 
       fx->col[PF_INC][0],
       fx->col[PF_INC][1],
       fx->col[PF_INC][2],
       fx->col[PF_INC][3]
       );
  LOG ("Color (START): %f %f %f %f\n", 
       fx->col[PF_START][0],
       fx->col[PF_START][1],
       fx->col[PF_START][2],
       fx->col[PF_START][3]
       );

  LOG ("Color (END): %f %f %f %f\n", 
       fx->col[PF_END][0],
       fx->col[PF_END][1],
       fx->col[PF_END][2],
       fx->col[PF_END][3]
       );

  return 0;
}


int    
pf_fx_tconfigure (PF_FX fx, float t0, float duration, float n_cycle, int loop)
{
  int i;

  if (!fx) return -1;


  if (!fx->active)
    {
      fx->type = PF_FX_TIME;
      if (duration < 0) return -1;

      fx->t_iter[PF_START] = t0;
      fx->t_iter[PF_CURRENT] = 0;
      fx->t_iter[PF_END] = t0 + n_cycle;
      fx->t_iter[PF_INC] = 0.1;

      fx->loop = loop;

      fx->duration = duration;
    }
  // Calculate increments
  // Position
  fx->pos[PF_INC][0] = (fx->pos[PF_END][0] - fx->pos[PF_START][0] )/ duration;
  fx->pos[PF_INC][1] = (fx->pos[PF_END][1] - fx->pos[PF_START][1] )/ duration;
  fx->pos[PF_INC][2] = (fx->pos[PF_END][2] - fx->pos[PF_START][2] )/ duration;

  fx->rot[PF_INC][0] = (fx->rot[PF_END][0] - fx->rot[PF_START][0] )/ duration;
  fx->rot[PF_INC][1] = (fx->rot[PF_END][1] - fx->rot[PF_START][1] )/ duration;
  fx->rot[PF_INC][2] = (fx->rot[PF_END][2] - fx->rot[PF_START][2] )/ duration;

  fx->col[PF_INC][0] = (fx->col[PF_END][0] - fx->col[PF_START][0] )/ duration;
  fx->col[PF_INC][1] = (fx->col[PF_END][1] - fx->col[PF_START][1] )/ duration;
  fx->col[PF_INC][2] = (fx->col[PF_END][2] - fx->col[PF_START][2] )/ duration;
  fx->col[PF_INC][3] = (fx->col[PF_END][3] - fx->col[PF_START][3] )/ duration;

  for (i = 0; i < 3; i++)
    {
      fx->pos[PF_CURRENT][i] = fx->pos[PF_START][i];
      fx->rot[PF_CURRENT][i] = fx->rot[PF_START][i];
      fx->col[PF_CURRENT][i] = fx->col[PF_START][i];
    }
  fx->col[PF_CURRENT][3] = fx->col[PF_START][3];

  LOG ("FX Configured!! (%f ms)\n", duration);
  LOG ("Color: %f %f %f %f\n", 
       fx->col[PF_INC][0],
       fx->col[PF_INC][1],
       fx->col[PF_INC][2],
       fx->col[PF_INC][3]
       );
  LOG ("Color (START): %f %f %f %f\n", 
       fx->col[PF_START][0],
       fx->col[PF_START][1],
       fx->col[PF_START][2],
       fx->col[PF_START][3]
       );

  LOG ("Color (END): %f %f %f %f\n", 
       fx->col[PF_END][0],
       fx->col[PF_END][1],
       fx->col[PF_END][2],
       fx->col[PF_END][3]
       );

  return 0;
}


static struct timeval _last_sample;
static float  _tdiff = 0.0;

int    
pf_fx_step (PF_FX fx)
{
  int  i, cnt;

  if (fx->remove_me) return -1;

  /* Update timing for animation. FRAMES and TIME based*/
  fx->iter[PF_CURRENT] ++;

  fx->t_iter[PF_CURRENT] += _tdiff;
  LOG ("Elapsed ms from last frame: %lf (total: %lf)\n", 
       _tdiff, fx->t_iter[PF_CURRENT]);


  if ((fx->type == PF_FX_FRAME && fx->iter[PF_CURRENT] < fx->iter[PF_START]) ||
      (fx->type == PF_FX_TIME  && fx->t_iter[PF_CURRENT] < fx->t_iter[PF_START]))
    {
      LOG ("Still waiting for effect on item'%s' (%d of %d\n",
	      fx->item->name, fx->iter[PF_CURRENT], fx->iter[PF_START]);
      return 0;
    }

  if (!fx->active)
    {
      
    }
  cnt = 0;
  if (!fx->use_colors)
    {
      for (i = 0; i < 3; i++)
	{
	  if (fabs (fx->pos[PF_END][i] - fx->pos[PF_CURRENT][i]) < 0.0001)
	    cnt ++;
	  else
	    {
	      if (fx->type == PF_FX_FRAME)
		{
		  fx->pos[PF_CURRENT][i] = fx->pos[PF_START][i] + 
		    (float)fx->iter[PF_CURRENT] * fx->pos[PF_INC][i];
		}
	      else
		{
		  fx->pos[PF_CURRENT][i] = fx->pos[PF_START][i] + 
		    (fx->t_iter[PF_CURRENT] - fx->t_iter[PF_START]) * 
		    fx->pos[PF_INC][i];
		}
	    }
	  
	  if (fabs (fx->rot[PF_END][i] - fx->rot[PF_CURRENT][i]) < 0.0001)
	    cnt++;
	  else
	    {
	      if (fx->type == PF_FX_FRAME)
		{
		  fx->rot[PF_CURRENT][i] = fx->rot[PF_START][i] + 
		    (float)fx->iter[PF_CURRENT] * fx->rot[PF_INC][i];
		}
	      else
		{
		  fx->rot[PF_CURRENT][i] = fx->rot[PF_START][i] + 
		    (fx->t_iter[PF_CURRENT] - fx->t_iter[PF_START]) * 
		    fx->rot[PF_INC][i];
		}
	    }

	}
      pf_ritem_set_abs_rot (fx->item, 
			    fx->rot[PF_CURRENT][0],
			    fx->rot[PF_CURRENT][1],
			    fx->rot[PF_CURRENT][2]);
      
      
      pf_ritem_set_abs_pos (fx->item, 
			    fx->pos[PF_CURRENT][0],
			    fx->pos[PF_CURRENT][1],
			    fx->pos[PF_CURRENT][2]);
      

    }

  if (fx->use_colors)
    {
      for (i = 0; i < 4; i++)
	{
	  if (fabs (fx->col[PF_END][i] - fx->col[PF_CURRENT][i]) < 0.0001)
	    cnt++;
	  else
	    {
	      if (fx->type == PF_FX_FRAME)
		{
		  //fx->col[PF_CURRENT][i] += fx->col[PF_INC][i];
		  fx->col[PF_CURRENT][i] = fx->col[PF_START][i] + 
		    (float)fx->iter[PF_CURRENT] * fx->col[PF_INC][i];
		}
	      else
		{
		  fx->col[PF_CURRENT][i] = fx->col[PF_START][i] + 
		    (fx->t_iter[PF_CURRENT] - fx->t_iter[PF_START]) * 
		    fx->col[PF_INC][i];

		}
	    }
	}
      if (fx->item->id == PFRI_TEXT)
	pf_text_set_color (fx->item, 
			   fx->col[PF_CURRENT][0],
			   fx->col[PF_CURRENT][1],
			   fx->col[PF_CURRENT][2],
			   fx->col[PF_CURRENT][3]);

      if (fx->item->id == PFRI_STEXT)
	pf_stext_set_color (fx->item, 
			   fx->col[PF_CURRENT][0],
			   fx->col[PF_CURRENT][1],
			   fx->col[PF_CURRENT][2],
			   fx->col[PF_CURRENT][3]);

      
      if (fx->item->id == PFRI_IMAGE)
	pf_image_set_color (fx->item, 
			    fx->col[PF_CURRENT][0],
			    fx->col[PF_CURRENT][1],
			    fx->col[PF_CURRENT][2],
			    fx->col[PF_CURRENT][3]);

      LOG ("(%d) Setting color (%f,%f,%f,%f)\n",
	   fx->item->id,
	   fx->col[PF_CURRENT][0],
	   fx->col[PF_CURRENT][1],
	   fx->col[PF_CURRENT][2],
	   fx->col[PF_CURRENT][3]);


    }
  
  


  LOG ("cnt:(%d)%d (%f,%f,%f) (%f,%f,%f) \n", 
       cnt, fx->iter[PF_CURRENT],
       fx->pos[PF_CURRENT][0],
       fx->pos[PF_CURRENT][1],
       fx->pos[PF_CURRENT][2],
       fx->pos[PF_END][0],
       fx->pos[PF_END][1],
       fx->pos[PF_END][2]
       );
  
  if ((fx->use_colors == 1 && cnt == 4) 
      || (fx->use_colors == 0 && cnt == 6)
      || (fx->type == PF_FX_FRAME && fx->iter[PF_CURRENT]   > 
	  fx->iter[PF_START] +  fx->n_frames)
      || (fx->type == PF_FX_TIME  && fx->t_iter[PF_CURRENT] > 
	  fx->t_iter[PF_START] +  fx->duration)
      )
      
    {
      if (!fx->loop)
	{
	  fx->remove_me = 1;
	  // Adjust final position
	  pf_ritem_set_abs_rot (fx->item, 
				fx->rot[PF_END][0],
				fx->rot[PF_END][1],
				fx->rot[PF_END][2]);
	  
	  
	  pf_ritem_set_abs_pos (fx->item, 
				fx->pos[PF_END][0],
				fx->pos[PF_END][1],
				fx->pos[PF_END][2]);
	}
      else
	{
	  //printf ("Restarting...");
	  // Restart 
	  // only if we reached the end of the cycle
	  if (fx->iter[PF_CURRENT] >= fx->iter[PF_START] + fx->iter[PF_END])
	    {
	      for (i = 0; i < 3; i++)
		{
		  fx->pos[PF_CURRENT][i] = fx->pos[PF_START][i];
		  fx->rot[PF_CURRENT][i] = fx->rot[PF_START][i];
		  fx->col[PF_CURRENT][i] = fx->col[PF_START][i];
		}
	      fx->col[PF_CURRENT][3] = fx->col[PF_START][3];
	      fx->iter[PF_CURRENT] = 0;
	    }
	}
    }

  return 0;

}

int    
pf_fx_mng_init ()
{
  int i;

  n_fx = 0;

  for (i = 0; i < MAX_FXS; i++)
    fx_list[i] = NULL;



  return 0;
}

int    
pf_fx_mng_end ()
{
  // Remove FXs
  return 0;
}

int    
pf_fx_mng_add_fx (PF_FX fx)
{
  int   i, j;

  LOG ("Adding FX %p\n", fx);
  // Look for a hole in the list
  j = 0;

  for (i = 0; i < MAX_FXS; i++)
    {
      LOG ("Testing for hole at %d\n", i);
      if (fx_list[i] == NULL)
	{
	  LOG ("Adding fx @ %d\n", i);
	  fx_list[i] = fx;
	  n_fx++;
	  break;
	}
    }
  return 0;
}

int
pf_fx_mng_clear ()
{
  int    i;
  for (i = 0; i < MAX_FXS; i++)
    {
      if (fx_list[i]) fx_list[i]->remove_me = 1;
    }

  return 0;
}

int    
pf_fx_mng_run ()
{
  int i, j, gc_step;
  struct timeval _current, _temp;

  if (_init_timer)
    {
      gettimeofday (&_last_sample, NULL);
      _tdiff = 0.01;
      _init_timer = 0;
    }
  else
    {
      gettimeofday (&_current, NULL);
      timersub (&_current, &_last_sample, &_temp);
      memcpy (&_last_sample, &_current, sizeof(struct timeval));
      _tdiff = _temp.tv_sec * 100.0 + _temp.tv_usec / 1000.0;
    }


  j = 0;
  gc_step = 0;
  for (i = 0; j < n_fx && i < MAX_FXS; i++)
    {
      if (fx_list[i] == NULL) continue; // Skip holes

      pf_fx_step (fx_list[i]);
      if (fx_list[i]->remove_me) 
	{
	  pf_fx_free (fx_list[i]);
	  fx_list[i] = NULL;
	  gc_step++;
	}
      j++;
    }

  // Update number of fxs
  n_fx -= gc_step;
  

  return 0;
}
