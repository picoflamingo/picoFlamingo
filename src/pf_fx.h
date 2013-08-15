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

#ifndef PF_FX_H
#define PF_FX_H

#define PF_START   0
#define PF_END     1
#define PF_CURRENT 2
#define PF_INC     3

#define PF_FX_FRAME 1
#define PF_FX_TIME  2

typedef struct pf_fx_t
{
  int active;
  PF_RITEM item;
  float    pos[4][3]; // Animation parameters
  float    rot[4][3]; // Animation parameters
  float    col[4][4]; // Animation parameters
  int      use_colors;
  int      remove_me;
  int      loop;
  /* Timing parameters */
  int      type;
  int      n_frames;
  int      iter[4];
  float    duration;  // ms
  float    t_iter[4];
} *PF_FX;


#ifdef __cplusplus
extern "C" {
#endif

  PF_FX  pf_fx_new (int type);
  int    pf_fx_free (PF_FX fx);

  int    pf_fx_use_colors (PF_FX fx, int flag);
  int    pf_fx_set_item (PF_FX, char *name);
  int    pf_fx_set_pos (PF_FX fx, int indx, float x, float y, float z);
  int    pf_fx_set_rot (PF_FX fx, int indx, float a, float b, float c);
  int    pf_fx_set_color (PF_FX fx, int indx, 
			  float r, float g, float b, float alpha);
  int    pf_fx_configure (PF_FX fx, int frame0, int n_frames, 
			  int n_cycle, int loop);
  int    pf_fx_tconfigure (PF_FX fx, float t0, float duration, 
			   float n_cycle, int loop);
  int    pf_fx_step (PF_FX fx);

  int    pf_fx_mng_init ();
  int    pf_fx_mng_end ();
  int    pf_fx_mng_add_fx (PF_FX fx);
  int    pf_fx_mng_run ();
  int pf_fx_mng_clear ();
#ifdef __cplusplus
}
#endif


#endif
