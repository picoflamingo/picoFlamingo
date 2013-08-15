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

#include "log.h"

#include "pf_render_item.h"

#include "pf_text.h"
#include "pf_stext.h"
#include "pf_image.h"
#include "pf_3d.h"
#include "pf_quad.h"
#include "pf_group.h"

#include "pfs_files.h"
#include "pfs_cmd.h"

#include "pf_fx.h"
#include "shader.h"
#include "const.h"

extern PF_RITEM ri_list[MAX_RITEMS];
extern int      n_ri_list;
extern char     *pres_dir;
extern int      current_item;

extern int    generic_events;
extern int    transition;
extern int    current_slide;
extern int    slide_show;
extern float  rotx, roty, rotz;

int (*cmd) () = NULL;


PF_RITEM pf_cube_new (float, float, float);


static   char style[1024];

typedef struct pfs_cmd_t 
{
   char *name;
  int (*func) (char *in, char *out);
} PFS_CMD;

static PFS_CMD pfs_cmd[] = {
  { "NEXT",                  pfs_cmd_next},
  { "PREV",                  pfs_cmd_prev},
  { "SLIDE_SHOW",            pfs_cmd_slide_show},
  { "ROT",                   pfs_cmd_rot},
  { "RELOAD",                pfs_cmd_default},
  { "GOTO",                  pfs_cmd_goto_slide},

  { "INCLUDE",               pfs_cmd_include},
  { "BACKGROUND",            pfs_cmd_background},
  { "ADD_IMAGE",             pfs_cmd_add_image},
  { "ADD_CENTERED_IMAGE",    pfs_cmd_add_centered_image},
  { "ADD_TEXT",              pfs_cmd_add_text},
  { "ADD_STEXT",             pfs_cmd_add_stext},
  { "SET_TEXT_INTERLINE",    pfs_cmd_set_text_interline},
  { "UPDATE_TEXT_INTERLINE", pfs_cmd_update_text_interline},
  { "UPDATE_FONT",           pfs_cmd_update_font},
  { "UPDATE_TEXT_SCALE",     pfs_cmd_update_text_scale},
  { "UPDATE_TEXT_WIDTH",     pfs_cmd_update_text_width},
  { "ADD_CUBE",              pfs_cmd_add_cube},

  { "ADD_MODEL",             pfs_cmd_add_model},
  { "ADD_QUAD",              pfs_cmd_add_quad},
  { "ADD_GRAPH",             pfs_cmd_default},
  { "ADD_BOX",               pfs_cmd_default},
  { "ADD_GROUP",             pfs_cmd_add_group},

  { "ADD_TO_GROUP",          pfs_cmd_add_to_group},
  { "UPDATE_QUAD",           pfs_cmd_update_quad},
  { "SET_QUAD",              pfs_cmd_set_quad},

  { "ADD_DATA",              pfs_cmd_add_data},
  { "POSITION",              pfs_cmd_position},
  { "ROTATION",              pfs_cmd_rotation},
  { "SCALE",                 pfs_cmd_scale},
  { "COLOR",                 pfs_cmd_color},
  { "DEPTH_TEST",            pfs_cmd_depth_test},
  { "CLICKABLE",             pfs_cmd_clickable},

  { "SET_CURRENT_ITEM",      pfs_cmd_default},
  
  { "SET_DEPTH_TEST",        pfs_cmd_set_depth_test},  
  { "SHADER",                pfs_cmd_shader},
  { "SET_SHADER",            pfs_cmd_set_shader},
  { "SET_FOCUS",             pfs_cmd_set_focus},
  { "NEXT_FOCUS",            pfs_cmd_default},
  { "PREV_FOCUS",            pfs_cmd_default},

  { "NAME",                  pfs_cmd_name},
  { "UPDATE_TEXT",           pfs_cmd_update_text},
  { "SHOW",                  pfs_cmd_show},
  { "SET_COLOR",             pfs_cmd_set_color},
  { "SET_POS",               pfs_cmd_set_pos},
  { "SET_ROT",               pfs_cmd_set_rot},
  { "SET_XFORM",             pfs_cmd_set_xform},
  { "GET_XFORM",             pfs_cmd_get_xform},
  { "SET_IMAGE",             pfs_cmd_set_image},
  { "SET_AUTO",              pfs_cmd_set_auto},
  // FXs
  { "FX_START",              pfs_cmd_fx_start},
  { "FX_END",                pfs_cmd_fx_end},
  { "SPIN",                  pfs_cmd_spin},
  { "FLASH_ITEM",            pfs_cmd_flash_item},
  { "FX_COLOR",              pfs_cmd_fx_color},
  { "FX_TCOLOR",              pfs_cmd_fx_tcolor},
  { "FX_POS",                pfs_cmd_fx_pos},
  { "FX_TPOS",               pfs_cmd_fx_tpos},
  { "FX_ROT",                pfs_cmd_fx_rot},
  { "FX_TROT",                pfs_cmd_fx_trot},
  { "FX_MOVE_TO",            pfs_cmd_fx_move_to}, 
  { "FX_TMOVE_TO",            pfs_cmd_fx_tmove_to}, 
  { "FX_ROT_TO",             pfs_cmd_fx_rot_to},
  { "FX_TROT_TO",             pfs_cmd_fx_trot_to},
  { "FX_COLOR_TO",           pfs_cmd_fx_color_to},
  { "FX_TCOLOR_TO",           pfs_cmd_fx_tcolor_to},
  { "FX_SWAP_FADE",          pfs_cmd_fx_swap_fade},

  /* Video Commands */
  { "CONNECT",               pfs_cmd_connect},
  { "DISCONNECT",            pfs_cmd_disconnect},
  { "SHOT_WITH_NAME",        pfs_cmd_shot_with_name},
  { "SHOT",                  pfs_cmd_shot},


  /* Extra text commands */

  {"TXT_SET_LINE",           pfs_cmd_txt_set_line},
  {"TXT_SET_LINES_UP",       pfs_cmd_txt_set_lines_up},
  {"TXT_SET_LINES_DOWN",     pfs_cmd_txt_set_lines_down},
  {"TXT_CLEAR",              pfs_cmd_txt_clear},
  {"TXT_ADD_LINE",           pfs_cmd_txt_add_line},
  {"TXT_SET_PARA",           pfs_cmd_txt_set_para},
  {"TXT_SET_HL",             pfs_cmd_txt_set_hl},
  {"TXT_SET_HL_COLOR",       pfs_cmd_txt_set_hl_color},

  /* Query commands */
  {"GET_POS",                 pfs_cmd_get_pos},
  {"GET_COLOR",               pfs_cmd_get_color},
  {"GET_TEXT_PARAMS",         pfs_cmd_get_text_params},
  {"GET_QUAD_PARAMS",         pfs_cmd_get_quad_params},
  /* Misc */
  {"GENERIC_EVENTS",           pfs_cmd_generic_events},
  {"LIST",                     pfs_cmd_list},
  {"SAVE",                     pfs_cmd_save},
  { NULL, NULL}
};

int
pfs_cmd_find (char *name)
{
  int i, l, l1, l2;

  for (i = 0; pfs_cmd[i].name != NULL; i++)
    {
      l1 = strlen (name);
      l2 = strlen (pfs_cmd[i].name);
      l = l1 > l2 ? l1 : l2;
      if (strncasecmp (name, pfs_cmd[i].name, l) == 0)
	return i;
    }
  return -1;
}


PF_RITEM
pfs_item_find (char *name)
{
  char *item_name;
  int   i, l, l1, l2;

  for (i = 0; i < n_ri_list; i++)
    {
      item_name = pf_ritem_get_name (ri_list[i]);
      if (item_name == NULL) continue;
      l1 = strlen (name);
      l2 = strlen (item_name);
      LOG ("-- Testing '%s' against '%s'\n", name, item_name);
      l = l1 > l2 ? l1 : l2;
      if (strncasecmp (name, item_name, l) == 0)
	{
	  /* If found we are going to do something. Set the dirty bound falg to 1*/
	  ri_list[i]->dirty_bound = 1;
	  return ri_list[i];
	}
    }
  return NULL;
}

int
pfs_item_find_indx (char *name)
{
  char *item_name;
  int   i, l, l1, l2;

  for (i = 0; i < n_ri_list; i++)
    {
      item_name = pf_ritem_get_name (ri_list[i]);
      if (item_name == NULL) continue;
      l1 = strlen (name);
      l2 = strlen (item_name);
      l = l1 > l2 ? l1 : l2;
      if (strncasecmp (name, item_name, l) == 0)
	return i;
    }
  return -1;
}


static int
cmd_next_slide ()
{
  LOG ("Running command NEXT. Setting transition\n");
  transition = 1;
  
  return 0;
}

static int
cmd_prev_slide ()
{
  transition = -1;

  return 0;
}



static int
cmd_slide_show ()
{
  if (!slide_show) 
    {
      slide_show = 1; 
    }
  else 
    {
      slide_show = 0;
      transition = 0;
    }

  LOG ("SLIDESHOW %d\n", slide_show);

  return 0;
}


int
pfs_cmd_next ()
{
  LOG ("Running command NEXT\n");
  cmd = cmd_next_slide;

  return 0;
}

int
pfs_cmd_prev ()
{
  cmd = cmd_prev_slide;

  return 0;
}

int
pfs_cmd_slide_show ()
{
  cmd = cmd_slide_show;

  return 0;
}




int
pfs_cmd_run (int index, char *in, char *out)
{

  if (pfs_cmd[index].func)
    return pfs_cmd[index].func (in + strlen (pfs_cmd[index].name) + 1, out);
  else
    {
      fprintf (stderr, "Command '%s' not implemented\n", pfs_cmd[index].name);
      return -1;
    }

  return 0;
}

/* Command implementation */

int 
pfs_cmd_default (char *in, char *out)
{
  fprintf (stderr, "Command not yet implemented\n");

  return 0;
}

int
pfs_cmd_include (char *in, char *out)
{
  char data[1024];

  sscanf (in, "%s", data);
  include_slide (data);
  return 0;
}


int
pfs_cmd_background (char *in, char *out)
{
  char data[1024];

  n_ri_list++;
  sscanf (in, "%s", data);
  LOG ("ADDING BACKGROUND ON ITEM %d\n", n_ri_list);

  ri_list[n_ri_list] = pf_image_new (data, 20.0, 1);
  //pf_ritem_set_pos (ri_list[n_ri_list], 0.0, 0.0, -10.0);
  pf_ritem_set_pos (ri_list[n_ri_list], 0.0, 0.0, -13.0);

  return 0;
}

int
pfs_cmd_add_cube (char *in, char *out)
{
  float  x, y, z;

  n_ri_list++;
  sscanf (in, "%f %f %f", &x, &y, &z);
  ri_list[n_ri_list] = pf_cube_new (x, y, z);

  return 0;
}

int 
pfs_cmd_add_group (char *in, char *out)
{
  char name[1024];

  n_ri_list++;
  ri_list[n_ri_list] = pf_group_new ();

  return 0;
}



int
pfs_cmd_add_image (char *in, char *out)
{
  char  data[1024];
  float scale;

  n_ri_list++;
  sscanf (in, "%f %s", &scale, data);
  ri_list[n_ri_list] = pf_image_new (data, scale, 0);

  return 0;
}


int
pfs_cmd_add_quad (char *in, char *out)
{
  float w, h;

  n_ri_list++;
  sscanf (in, "%f %f", &w, &h);
  LOG ("Adding QUAD %fx%f\n", w, h);
  ri_list[n_ri_list] = pf_quad_new (w, h);

  return 0;
}


int
pfs_cmd_add_centered_image (char *in, char *out)
{
  char  data[1024];
  float scale;

  n_ri_list++;
  sscanf (in, "%f %s", &scale, data);
  ri_list[n_ri_list] = pf_image_new (data, scale, 1);

  return 0;
}


int
pfs_cmd_add_model (char *in, char *out)
{
  char data[1024];

  n_ri_list++;
  sscanf (in, "%s", data);
  ri_list[n_ri_list] = pf_3d_new (data);

  return 0;
}


int
pfs_cmd_add_text (char *in, char *out)
{
  sscanf (in, "%s", style);

  n_ri_list++;
  ri_list[n_ri_list] = pf_text_new (style, NULL);

  return 0;
}


int
pfs_cmd_add_stext (char *in, char *out)
{
  sscanf (in, "%s", style);

  n_ri_list++;
  ri_list[n_ri_list] = pf_stext_new (style, NULL);

  return 0;
}

int
pfs_cmd_add_data (char *in, char *out)
{
  char data[1024];

  sscanf (in, "%s", data);

  if (ri_list[n_ri_list]->id == PFRI_TEXT)
    pf_text_add_text_line (ri_list[n_ri_list], in);
  else
    pf_stext_add_text_line (ri_list[n_ri_list], in);

  return 0;
}

// General
int
pfs_cmd_position (char *in, char *out)
{
  float  x, y, z;

  sscanf (in, "%f %f %f", &x, &y, &z);
  pf_ritem_set_pos (ri_list[n_ri_list], x, y, z);

  return 0;
}

int
pfs_cmd_rotation (char *in, char *out)
{
  float  x, y, z;

  sscanf (in, "%f %f %f", &x, &y, &z);
  pf_ritem_set_rot (ri_list[n_ri_list], x, y, z);

  return 0;
}

int
pfs_cmd_scale (char *in, char *out)
{
  float scale;

  sscanf (in, "%f", &scale);
  pf_text_set_scale (ri_list[n_ri_list], scale, scale);	    

  return 0;
}


int
pfs_cmd_set_text_interline (char *in, char *out)
{
  float scale;

  sscanf (in, "%f", &scale);
  if (ri_list[n_ri_list]->id == PFRI_STEXT)  
    pf_stext_set_interline (ri_list[n_ri_list], scale);	    
  if (ri_list[n_ri_list]->id == PFRI_TEXT)  
    pf_text_set_interline (ri_list[n_ri_list], scale);	    

  return 0;
}

int
pfs_cmd_update_text_interline (char *in, char *out)
{
  float scale;
  PF_RITEM  item;  
  char      name[1024];

  sscanf (in, "%s %f", name, &scale);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);

  if (item->id == PFRI_STEXT)  
    pf_stext_set_interline (item, scale);	
  else if (item->id == PFRI_TEXT)  
    pf_text_set_interline (item, scale);    
  else
    return -1;

  return 0;
}


int
pfs_cmd_update_font (char *in, char *out)
{
  PF_RITEM  item;  
  char      name[1024], font[1024];

  sscanf (in, "%s %s", name, font);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);

  LOG ("Upating font for item %s to %s\n", name, font);
  if (item->id == PFRI_TEXT)  
    pf_text_set_font (item, font);	    
  else
    return -1;

  return 0;
}

int
pfs_cmd_update_text_scale (char *in, char *out)
{
  float     scale_x, scale_y;
  PF_RITEM  item;  
  char      name[1024];

  sscanf (in, "%s %f %f", name, &scale_x, &scale_y);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);

  if (item->id == PFRI_TEXT)  
    pf_text_set_scale (item, scale_x, scale_y);	    
  else
    return -1;

  return 0;
}


int
pfs_cmd_update_quad (char *in, char *out)
{
  float     scale_x, scale_y;
  PF_RITEM  item;  
  char      name[1024];

  sscanf (in, "%s %f %f", name, &scale_x, &scale_y);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);


  pf_quad_set_size (item, scale_x, scale_y);	    

  return 0;
}

int
pfs_cmd_set_quad (char *in, char *out)
{
  float     x0, y0, x1, y1;
  PF_RITEM  item;  
  char      name[1024];

  sscanf (in, "%s %f %f %f %f", name, &x0, &y0, &x1, &y1);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);


  pf_quad_set_vertex (item, x0, y0, x1, y1);

  return 0;
}


int
pfs_cmd_update_text_width (char *in, char *out)
{
  float scale;
  PF_RITEM  item;  
  char      name[1024];

  sscanf (in, "%s %f", name, &scale);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);

  if (item->id == PFRI_TEXT || item->id == PFRI_STEXT)  
    pf_text_set_width (item, scale);	    
  else
    return -1;

  return 0;
}


int
pfs_cmd_color (char *in, char *out)
{
  float  r, g, b, alpha;

  sscanf (in, "%f %f %f %f", &r, &g, &b, &alpha);
  if (ri_list[n_ri_list]->id == PFRI_TEXT)
    pf_text_set_color (ri_list[n_ri_list], r, g, b, alpha);	    
  else if (ri_list[n_ri_list]->id == PFRI_STEXT)
    pf_stext_set_color (ri_list[n_ri_list], r, g, b, alpha);	    
  else if (ri_list[n_ri_list]->id == PFRI_IMAGE)
    pf_image_set_color (ri_list[n_ri_list], r, g, b, alpha);	    
  else if (ri_list[n_ri_list]->id == PFRI_QUAD)
    pf_quad_set_color (ri_list[n_ri_list], r, g, b, alpha);	    
  else if (ri_list[n_ri_list]->id == PFRI_3D)
    pf_ritem_set_color (ri_list[n_ri_list], r, g, b, alpha);	    


  return 0;
}

int
pfs_cmd_depth_test (char *in, char *out)
{
  int flag;

  sscanf (in, "%d", &flag);
  //pf_ritem_set_depth_test (ri_list[n_ri_list], flag);
  ri_list[n_ri_list]->depth  = flag;

  return 0;
}

int
pfs_cmd_clickable (char *in, char *out)
{
  int flag;

  sscanf (in, "%d", &flag);
  printf ("Current item '%s' is clickable!\n", ri_list[n_ri_list]->name);
  ri_list[n_ri_list]->clickable  = flag;
  if (flag)
    ri_list[n_ri_list]->dirty_bound = 1;

  return 0;
}


int
pfs_cmd_set_color (char *in, char *out)
{
  PF_RITEM  item;
  float     r, g, b, alpha;
  char      name[1024];


  sscanf (in, "%s %f %f %f %f", name, &r, &g, &b, &alpha);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  if (item->id == PFRI_TEXT)
    pf_text_set_color (item, r, g, b, alpha);	    
  else if (item->id == PFRI_STEXT)
    pf_stext_set_color (item, r, g, b, alpha);	    

  else if (item->id == PFRI_IMAGE)
    pf_image_set_color (item, r, g, b, alpha);	    
  else if (item->id == PFRI_QUAD)
    pf_quad_set_color (item, r, g, b, alpha);	    
  else if (item->id == PFRI_3D)
    pf_ritem_set_color (item, r, g, b, alpha);	    


  return 0;
}

int
pfs_cmd_set_depth_test (char *in, char *out)
{
  PF_RITEM  item;
  int       flag;
  char      name[1024];


  sscanf (in, "%s %d", name, &flag);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  pf_ritem_set_depth_test (item, flag);	    


  return 0;
}


int
pfs_cmd_set_image (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024], file[1024];


  sscanf (in, "%s %s", name, file);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  if (item->id == PFRI_IMAGE)
    pf_image_set_data_from_file (item, file);	    


  return 0;
}

int
pfs_cmd_set_auto (char *in, char *out)
{
  PF_RITEM  item;
  int       flag;
  char   name[1024], file[1024];


  sscanf (in, "%s %d", name, &flag);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  printf ("Setting auto to:%d\n", flag);
  if (item->id == PFRI_IMAGE)
    pf_image_set_auto (item, flag);	    
  

  return 0;
}





int
pfs_cmd_set_pos (char *in, char *out)
{
  PF_RITEM  item;
  float     x, y, z;
  char      name[1024];


  sscanf (in, "%s %f %f %f", name, &x, &y, &z);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  pf_ritem_set_abs_pos (item, x, y, z);

  return 0;
}

int
pfs_cmd_get_pos (char *in, char *out)
{
  PF_RITEM  item;
  char      name[1024];

  sscanf (in, "%s", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  printf ("GET_POS %s %f %f %f\n", name, item->x, item->y, item->z);
  fflush (0);
  return 0;
}


int 
pfs_cmd_get_color (char *in, char *out)
{
  PF_RITEM  item;
  float     r, g, b, a;
  char      name[1024];


  sscanf (in, "%s", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  pf_ritem_get_color (item, &r, &g, &b, &a);
  printf ("GET_COLOR %s %f %f %f %f\n", name, r, g, b, a);
  fflush (0);

  return 0;
}

int 
pfs_cmd_get_text_params (char *in, char *out)
{
  PF_RITEM  item;
  float     x, y, il, w;
  char      name[1024];


  sscanf (in, "%s", name);

  x = y = il = w = 0.0;
  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  if (item->id == PFRI_TEXT)
    pf_text_get_params (item, &x, &y, &il, &w);

  printf ("GET_TEXT_PARAMS %s %f %f %f %f\n", name, x, y, il, w);
  fflush (0);

  return 0;
}

int 
pfs_cmd_get_quad_params (char *in, char *out)
{
  PF_RITEM  item;
  float     x, y;
  char      name[1024];


  sscanf (in, "%s", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  x = y = 0.0;

  if (item->id == PFRI_QUAD)
    pf_quad_get_size (item, &x, &y);

  printf ("GET_QUAD_PARAMS %s %f %f\n", name, x, y);
  fflush (0);
  return 0;

}



int
pfs_cmd_set_rot (char *in, char *out)
{
  PF_RITEM  item;
  float     x, y, z;
  char      name[1024];


  sscanf (in, "%s %f %f %f", name, &x, &y, &z);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  pf_ritem_set_abs_rot (item, x, y, z);

  return 0;
}




int
pfs_cmd_set_focus (char *in, char *out)
{
  char      name[1024];
  int       i;

  sscanf (in, "%s", name);

  if ((i = pfs_item_find_indx (name)) < 0)
    {
      current_item = n_ri_list;
      return 0;
    }
  LOG ("Found item '%s' (%d)\n", name, i);

  current_item = i;
  return 0;
}

int
pfs_cmd_name (char *in, char *out)
{
  char   name[1024];

  sscanf (in, "%s", name);

  pf_ritem_set_name (ri_list[n_ri_list], name);

  return 0;
}

int
pfs_cmd_update_text (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  char   text[1024], *aux;

  sscanf (in, "%s %s", name, text);
  LOG ("Looking for '%s'\n", name);

  /* Find text */
  aux = strchr (in, ' ');
  aux++;
  aux = strchr (in, ' ');
  aux++;

  LOG ("Name: '%s' Text:'%s'\n", name, aux);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);
  pf_text_clear (item);
  LOG ("Setting text");
  pf_text_add_text_line (item, aux);
  return 0;
}

int
pfs_cmd_txt_add_line (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  char   text[1024], *aux;

  sscanf (in, "%s %s", name, text);
  LOG ("Looking for '%s'\n", name);

  /* Find text */
  aux = strchr (in, ' ');
  aux++;
  aux = strchr (in, ' ');
  aux++;

  LOG ("Name: '%s' Text:'%s'\n", name, aux);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);
  LOG ("Setting text");
  if (item->id == PFRI_TEXT)
    pf_text_add_text_line (item, aux);
  else
    pf_stext_add_text_line (item, aux);
  return 0;
}

int
pfs_cmd_txt_clear (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  char   text[1024], *aux;

  //sscanf (in, "%s %s", name, text);
  sscanf (in, "%s", name);
  LOG ("Looking for '%s'\n", name);

  /* Find text */
  aux = strchr (in, ' ');
  aux++;
  aux = strchr (in, ' ');
  aux++;

  //LOG ("Name: '%s' Text:'%s'\n", name, aux);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);
  if (item->id == PFRI_TEXT)
    pf_text_clear (item);
  else if (item->id == PFRI_STEXT)
    pf_stext_clear (item);
  return 0;
}


int
pfs_cmd_rot (char *in, char *out)
{
  sscanf (in, "%f %f %f\n", &rotx, &roty, &rotz);
  LOG ("Rotating current item: %f %f %f\n", rotx, roty, rotz);

  return 0;
}

int
pfs_cmd_show (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  int    flag;

  sscanf (in, "%s %d", name, &flag);
  LOG ("Changing '%s' visibility to:%d \n", name, flag);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);
  pf_ritem_show (item, flag);

  return 0;
}




int
pfs_cmd_connect (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024], ip[1024];
  int    flag;

  sscanf (in, "%s %s %d", name, ip, &flag);
  LOG ("Changing '%s' visibility to:%d \n", name, flag);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);

  /* Setting the thing up */
  pf_image_reconnect (item, ip, flag);


  return 0;
}

int
pfs_cmd_shot (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];

  sscanf (in, "%s", name);
  LOG ("Saving Photo from image '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);

  /* Setting the thing up */
  pf_image_save (item);


  return 0;
}

int
pfs_cmd_shot_with_name (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  char   fname[1024];

  sscanf (in, "%s %s", name, fname);
  LOG ("Saving Photo from image '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);

  /* Setting the thing up */
  pf_image_save_as (item, fname);


  return 0;
}


int
pfs_cmd_generic_events (char *in, char *out)
{
  int    flag;

  sscanf (in, "%d", &flag);

  generic_events = flag;

  return 0;
}


int  
pfs_cmd_txt_set_line (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  int    line;

  sscanf (in, "%s %d", name, &line);
  LOG ("Changing '%s' current line to:%d \n", name, line);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);
  pf_text_set_current_line (item, line);
  return 0;
  
}
int  
pfs_cmd_txt_set_lines_up (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  int    line;

  sscanf (in, "%s %d", name, &line);
  LOG ("Changing '%s' lines up :%d \n", name, line);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);
  pf_text_set_lines_up (item, line);
  return 0;

}

int  pfs_cmd_txt_set_lines_down (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  int    line;

  sscanf (in, "%s %d", name, &line);
  LOG ("Changing '%s' lines down :%d \n", name, line);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);
  if (item->id == PFRI_TEXT)
    pf_text_set_lines_down (item, line);
  else if (item->id == PFRI_STEXT)
    pf_stext_set_lines_down (item, line);

  return 0;
}




int
pfs_cmd_disconnect (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];

  sscanf (in, "%s", name);
  LOG ("Disconnection stream '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item at: %p\n", item);

  /* Setting the thing up */
  pf_image_disconnect (item);


  return 0;
}


int
pfs_cmd_spin (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  a, b, c;
  int    frames;
  PF_FX    my_fx;

  sscanf (in, "%s %d %f %f %f", name, &frames, &a, &b, &c);
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;


  my_fx = pf_fx_new (0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  LOG ("Current attitude: (%f,%f,%f)\n", item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a + a, item->b + b, item->c + c);
  LOG ("Target attitude: (%f,%f,%f)\n", item->a + a, item->b + b, item->c + c);

  pf_fx_configure (my_fx, 0, frames, frames, 0);

  pf_fx_mng_add_fx (my_fx);

  return 0;
}



int
pfs_cmd_flash_item (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  r, g, b, alpha;
  int    frames, duration, loop;
  PF_FX    my_fx_in, my_fx_out;

  sscanf (in, "%s %d %d %d", name, &frames, &duration, &loop);
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  my_fx_in = pf_fx_new (0);
  pf_fx_set_item (my_fx_in, name);

  pf_fx_set_pos (my_fx_in, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx_in, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx_in, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx_in, PF_END, item->a, item->b, item->c);

  if (item->id == PFRI_TEXT)
    {
      pf_text_get_color (item, &r, &g, &b, &alpha);
      LOG ("Flashing text with color: %f, %f, %f\n", r, g, b);
      pf_fx_set_color (my_fx_in, PF_START, r, g, b, 0.0);
      pf_fx_set_color (my_fx_in, PF_END, r, g, b, 1.0);
    }
  else
    {
      pf_fx_set_color (my_fx_in, PF_START, 1.0, 1.0, 1.0, 0.0);
      pf_fx_set_color (my_fx_in, PF_END, 1.0, 1.0, 1.0, 1.0);
    }

  pf_fx_configure (my_fx_in, 0, frames, frames + duration, loop);


  my_fx_out = pf_fx_new (0);
  pf_fx_set_item (my_fx_out, name);

  pf_fx_set_pos (my_fx_out, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx_out, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx_out, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx_out, PF_END, item->a, item->b, item->c);

  if (item->id == PFRI_TEXT)
    {
      LOG ("Flashing text with color: %f, %f, %f\n", r, g, b);
      pf_fx_set_color (my_fx_out, PF_START, r, g, b, 1.0);
      pf_fx_set_color (my_fx_out, PF_END, r, g, b, 0.0);
    }
  else
    {
      pf_fx_set_color (my_fx_out, PF_START, 1.0, 1.0, 1.0, 1.0);
      pf_fx_set_color (my_fx_out, PF_END, 1.0, 1.0, 1.0, 0.0);
    }

  pf_fx_configure (my_fx_out, frames + duration, frames, frames+duration, loop);


  pf_fx_mng_add_fx (my_fx_in);
  pf_fx_mng_add_fx (my_fx_out);

  return 0;
}




int
pfs_cmd_fx_color (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  r0, g0, b0, a0, r1, g1, b1, a1;
  int    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %d %d %f %f %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &r0, &g0, &b0, &a0,
	  &r1, &g1, &b1, &a1
	  );
  LOG ("FX Color item '%s' (from %d duration: %d\n", name, frames, duration);
  LOG ("    (%f,%f,%f,%f) -> (%f,%f,%f,%f)\n", 
       r0, g0, b0, a0, r1, g1, b1, a1);

  if ((item = pfs_item_find (name)) == NULL)
    {
      fprintf (stderr, "Cannot find item '%s'\n", name);
      return 0;
    }

  my_fx = pf_fx_new (0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);

  pf_fx_set_color (my_fx, PF_START, r0, g0, b0, a0);
  pf_fx_set_color (my_fx, PF_END, r1, g1, b1, a1);

  pf_fx_configure (my_fx, frames, duration, 0, 0);



  pf_fx_mng_add_fx (my_fx);


  return 0;
}


int
pfs_cmd_fx_tcolor (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  r0, g0, b0, a0, r1, g1, b1, a1;
  float   frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %f %f %f %f %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &r0, &g0, &b0, &a0,
	  &r1, &g1, &b1, &a1
	  );
  LOG ("FX Color item '%s' (from %d duration: %d\n", name, frames, duration);
  LOG ("    (%f,%f,%f,%f) -> (%f,%f,%f,%f)\n", 
       r0, g0, b0, a0, r1, g1, b1, a1);

  if ((item = pfs_item_find (name)) == NULL)
    {
      fprintf (stderr, "Cannot find item '%s'\n", name);
      return 0;
    }

  my_fx = pf_fx_new (0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);

  pf_fx_set_color (my_fx, PF_START, r0, g0, b0, a0);
  pf_fx_set_color (my_fx, PF_END, r1, g1, b1, a1);

  pf_fx_tconfigure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}




int
pfs_cmd_fx_color_to (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  r0, g0, b0, a0, r1, g1, b1, a1;
  int    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %d %d %f %f %f %f", 
	  name, &frames, &duration,
	  &r1, &g1, &b1, &a1
	  );

  if ((item = pfs_item_find (name)) == NULL)
    {
      fprintf (stderr, "Cannot find item '%s'\n", name);
      return 0;
    }
  pf_ritem_get_color (item, &r0, &g0, &b0, &a0);

  LOG ("FX Color item '%s' (from %d duration: %d\n", name, frames, duration);
  LOG ("    (%f,%f,%f,%f) -> (%f,%f,%f,%f)\n", 
       r0, g0, b0, a0, r1, g1, b1, a1);


  my_fx = pf_fx_new (0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);

  pf_fx_set_color (my_fx, PF_START, r0, g0, b0, a0);
  pf_fx_set_color (my_fx, PF_END, r1, g1, b1, a1);

  pf_fx_configure (my_fx, frames, duration, 0, 0);



  pf_fx_mng_add_fx (my_fx);


  return 0;
}





int
pfs_cmd_fx_tcolor_to (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  r0, g0, b0, a0, r1, g1, b1, a1;
  float    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &r1, &g1, &b1, &a1
	  );

  if ((item = pfs_item_find (name)) == NULL)
    {
      fprintf (stderr, "Cannot find item '%s'\n", name);
      return 0;
    }
  pf_ritem_get_color (item, &r0, &g0, &b0, &a0);

  LOG ("FX Color item '%s' (from %d duration: %d\n", name, frames, duration);
  LOG ("    (%f,%f,%f,%f) -> (%f,%f,%f,%f)\n", 
       r0, g0, b0, a0, r1, g1, b1, a1);


  my_fx = pf_fx_new (0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);

  pf_fx_set_color (my_fx, PF_START, r0, g0, b0, a0);
  pf_fx_set_color (my_fx, PF_END, r1, g1, b1, a1);

  pf_fx_tconfigure (my_fx, frames, duration, 0, 0);



  pf_fx_mng_add_fx (my_fx);


  return 0;
}







int
pfs_cmd_fx_pos (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  int    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %d %d %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &x0, &y0, &z0,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_pos (my_fx, PF_END, x1, y1, z1);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);


  pf_fx_configure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}


int
pfs_cmd_fx_tpos (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  float    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %f %f %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &x0, &y0, &z0,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_pos (my_fx, PF_END, x1, y1, z1);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);


  pf_fx_tconfigure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}



int
pfs_cmd_fx_move_to (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  int    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %d %d %f %f %f", 
	  name, &frames, &duration,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  pf_ritem_get_abs_pos (item, &x0, &y0, &z0);

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_pos (my_fx, PF_END, x1, y1, z1);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);


  pf_fx_configure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}




int
pfs_cmd_fx_tmove_to (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  float   frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %f %f %f %f %f", 
	  name, &frames, &duration,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  pf_ritem_get_abs_pos (item, &x0, &y0, &z0);

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_pos (my_fx, PF_END, x1, y1, z1);

  pf_fx_set_rot (my_fx, PF_START, item->a, item->b, item->c);
  pf_fx_set_rot (my_fx, PF_END, item->a, item->b, item->c);


  pf_fx_tconfigure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}



int
pfs_cmd_fx_rot (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  int    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %d %d %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &x0, &y0, &z0,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_rot (my_fx, PF_END, x1, y1, z1);


  pf_fx_configure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}




int
pfs_cmd_fx_trot (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  float   frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %f %f %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &x0, &y0, &z0,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_rot (my_fx, PF_END, x1, y1, z1);


  pf_fx_tconfigure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}





int
pfs_cmd_fx_rot_to (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  int    frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %d %d %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &x0, &y0, &z0,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_rot (my_fx, PF_END, x1, y1, z1);


  pf_fx_configure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}




int
pfs_cmd_fx_trot_to (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  x0, y0, z0, x1, y1, z1;
  float   frames, duration;
  PF_FX    my_fx;

  sscanf (in, "%s %f %f %f %f %f %f %f %f", 
	  name, &frames, &duration,
	  &x0, &y0, &z0,
	  &x1, &y1, &z1
	  );
  LOG ("Spining item '%s'\n", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  my_fx = pf_fx_new (0);
  pf_fx_use_colors (my_fx, 0);
  pf_fx_set_item (my_fx, name);

  pf_fx_set_pos (my_fx, PF_START, item->x, item->y, item->z);
  pf_fx_set_pos (my_fx, PF_END, item->x, item->y, item->z);

  pf_fx_set_rot (my_fx, PF_START, x0, y0, z0);
  pf_fx_set_rot (my_fx, PF_END, x1, y1, z1);


  pf_fx_tconfigure (my_fx, frames, duration, 0, 0);

  pf_fx_mng_add_fx (my_fx);


  return 0;
}





int 
pfs_cmd_fx_swap_fade (char *in, char *out)
{
  return 0;
}

int 
pfs_cmd_fx_start (char *in, char *out)
{
  n_ri_list++;
  return 0;
}

int 
pfs_cmd_fx_end (char *in, char *out)
{
  n_ri_list--;
  return 0;
}


int 
pfs_cmd_list (char *in, char *out)
{
  int i;
  char  buffer[1024];

  for (i = 0; i < n_ri_list; i++)
    {
      net_printf ("LIST %s\n", pf_ritem_get_name (ri_list[i]));
      //snprintf (buffer, 1024, "LIST %s\n", pf_ritem_get_name (ri_list[i]));
      //broadcast_msg (buffer);
      fflush (0);
    }
  //printf ("LIST $EOF\n");
  //broadcast_msg ("LIST $EOF\n");
  net_printf ("LIST $EOF\n");
  fflush (0);
  return 0;
}


int 
pfs_cmd_save (char *in, char *out)
{
#if 0
  int i;
  FILE *f;

  /* Dump into file */
  for (i = 0; i < n_ri_list; i++)
    {
      if (ri_list[i]->save)
	ri_list[i]->save (ri_list[i], f);
    }
#endif
  return 0;
}

int
pfs_cmd_set_shader (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024], shader[1024];
  int    prg;

  sscanf (in, "%s %s", 
	  name, shader);

  printf ("Appying shader %s toitem '%s'\n", shader, name);
  if ((prg = get_shader (shader)) < 0)
    {
      fprintf (stderr, "Cannot find shader '%s'\n", shader);
      
      return -1;
    }
  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  pf_ritem_set_prg (item, prg);

  return 0;
}

int
pfs_cmd_shader (char *in, char *out)
{
  char   shader[1024];
  int    prg;

  sscanf (in, "%s", shader);

  LOG ("Appying shader %s to item '%s'\n", shader, ri_list[n_ri_list]->name);
  if ((prg = get_shader(shader)) < 0)
    {
      fprintf (stderr, "Cannot find shader '%s'\n", shader);
      
      return -1;
    }

  pf_ritem_set_prg (ri_list[n_ri_list], prg);

  return 0;
}

int
pfs_cmd_goto_slide (char *in, char *out)
{
  int  i;
  sscanf (in, "%d", &i);
  if (i == 0) current_slide = 1;
  else current_slide = i;
  fprintf (stderr, "Loading slide %d\n", current_slide);
  transition = 2;
  return 0;
}

int 
pfs_cmd_txt_set_para (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  i1, i2, p;

  sscanf (in, "%s %f %f %f", name, &i1, &i2, &p);

  LOG ("Adjusting paragraph item '%s' (%f,%f,%f)\n", 
       name, i1, i2, p);
  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  if (item->id == PFRI_TEXT)
    pf_text_set_para (item, i1, i2, p);
  else if (item->id == PFRI_STEXT)
    pf_stext_set_para (item, i1, i2, p);

  return 0;

}

int 
pfs_cmd_txt_set_hl (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  int    flag;

  sscanf (in, "%s %d", name, &flag);

  printf ("Setting Highlight  item '%s' (%d)\n", 
	  name, flag);
  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  pf_text_set_hl (item,flag);

  return 0;

}



int 
pfs_cmd_txt_set_hl_color (char *in, char *out)
{
  PF_RITEM  item;
  char   name[1024];
  float  r, g, b, alpha;

  sscanf (in, "%s %f %f %f %f", name, &r, &g, &b, &alpha);

  LOG ("Setting Text Highlight item '%s' color to (%f,%f.%f,%f)\n", 
       name, r, g, b, alpha);
  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  pf_stext_set_hl_color (item, r, g, b, alpha);

  return 0;

}


int
pfs_cmd_set_xform (char *in, char *out)
{
  PF_RITEM  item;
  float     a[16];
  char      name[1024];
  int       i;


  sscanf (in, "%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
	  name, 
	  &a[0], &a[1], &a[2], &a[3],
	  &a[4], &a[5], &a[6], &a[7],
	  &a[8], &a[9], &a[10], &a[11],
	  &a[12], &a[13], &a[14], &a[15]
	  );

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  //pf_ritem_set_abs_pos (item, x, y, z);
  for (i = 0; i < 16; i++)
    item->current_trans[i] = a[i];

  return 0;
}

int
pfs_cmd_get_xform (char *in, char *out)
{
  PF_RITEM  item;
  char      name[1024];
  int       i;

  sscanf (in, "%s", name);

  if ((item = pfs_item_find (name)) == NULL)
    return 0;
  LOG ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  
  //pf_ritem_set_abs_pos (item, x, y, z);
  for (i = 0; i < 16; i++)
    printf ("%f ",item->current_trans[i]);
  printf ("\n");

  return 0;
}

int 
pfs_cmd_add_to_group (char *in, char *out)
{
  PF_RITEM  item, group;
  char      name[1024], gname[1024];
  int       i;

  sscanf (in, "%s %s", gname, name);

  printf ("Adding item '%s' to group '%s'\n", name, gname); 

  if ((item = pfs_item_find (name)) == NULL)
    return 0;

  if ((group = pfs_item_find (gname)) == NULL)
    return 0;


  printf ("Found item '%s' (%d) at: %p \n", name, item->id, item);
  printf ("--> Adding to group '%s' (%d) at: %p\n", gname, group->id, group);

  pf_group_add_child (group, item);
  return 0;
}
