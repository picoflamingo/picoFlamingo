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

#ifndef PFS_CMD_H
#define PFS_CMD_H

#include "pf_render_item.h"

#ifdef __cplusplus
extern "C" {
#endif

  PF_RITEM pfs_item_find (char *name);
  int pfs_item_find_indx (char *name);

  int pfs_cmd_find (char *name);
  int pfs_cmd_run (int index, char *in, char *out);

  int pfs_cmd_default (char *in, char *out);
  // Commands
  int pfs_cmd_include (char *in, char *out);
  int pfs_cmd_background (char *in, char *out);
  int pfs_cmd_add_cube (char *in, char *out);
  int pfs_cmd_add_sphere (char *in, char *out);
  int pfs_cmd_add_image (char *in, char *out);
  int pfs_cmd_add_centered_image (char *in, char *out);
  int pfs_cmd_add_model (char *in, char *out);
  int pfs_cmd_add_quad (char *in, char *out);
  int pfs_cmd_add_text (char *in, char *out);
  int pfs_cmd_add_data (char *in, char *out);
  int pfs_cmd_set_text_interline (char *in, char *out);
  int pfs_cmd_update_text_interline (char *in, char *out);
  int pfs_cmd_update_text_scale (char *in, char *out);
  int pfs_cmd_update_font (char *in, char *out);
  int pfs_cmd_update_text_width (char *in, char *out);
  int pfs_cmd_update_quad (char *in, char *out);
  int pfs_cmd_set_quad (char *in, char *out);
  
  int pfs_cmd_position (char *in, char *out);
  int pfs_cmd_rotation (char *in, char *out);
  int pfs_cmd_scale (char *in, char *out);
  int pfs_cmd_color (char *in, char *out);

  int pfs_cmd_set_focus (char *in, char *out);
  int pfs_cmd_name (char *in, char *out);

  int pfs_cmd_next ();
  int pfs_cmd_prev ();
  int pfs_cmd_slide_show ();
  int pfs_cmd_update_text (char *in, char *out);
  int pfs_cmd_rot (char *in, char *out);
  int pfs_cmd_show (char *in, char *out);
  int pfs_cmd_connect (char *in, char *out);
  int pfs_cmd_disconnect (char *in, char *out);
  int pfs_cmd_shot (char *in, char *out);
  int pfs_cmd_shot_with_name (char *in, char *out);

  int pfs_cmd_set_color (char *in, char *out);
  int pfs_cmd_set_pos (char *in, char *out);
  int pfs_cmd_set_rot (char *in, char *out);
  int pfs_cmd_set_image (char *in, char *out);
  int pfs_cmd_set_auto (char *in, char *out);

  int pfs_cmd_spin (char *in, char *out);
  int pfs_cmd_flash_item (char *in, char *out);

  int pfs_cmd_fx_color (char *in, char *out);
  int pfs_cmd_fx_tcolor (char *in, char *out);
  int pfs_cmd_fx_pos (char *in, char *out);
  int pfs_cmd_fx_tpos (char *in, char *out);
  int pfs_cmd_fx_rot (char *in, char *out); 
  int pfs_cmd_fx_trot (char *in, char *out); 
  int pfs_cmd_fx_move_to (char *in, char *out);
  int pfs_cmd_fx_tmove_to (char *in, char *out);
  int pfs_cmd_fx_rot_to (char *in, char *out);
  int pfs_cmd_fx_trot_to (char *in, char *out);
  int pfs_cmd_fx_color_to (char *in, char *out);
  int pfs_cmd_fx_tcolor_to (char *in, char *out);


  int pfs_cmd_fx_swap_fade (char *in, char *out);
  int pfs_cmd_fx_start (char *in, char *out);
  int pfs_cmd_fx_end (char *in, char *out);


  int  pfs_cmd_txt_set_line (char *in, char *out);
  int  pfs_cmd_txt_set_lines_up (char *in, char *out);
  int  pfs_cmd_txt_set_lines_down (char *in, char *out);

  int pfs_cmd_generic_events (char *in, char *out);
  int pfs_cmd_list (char *in, char *out);
  int pfs_cmd_save (char *in, char *out);
  int pfs_cmd_get_pos (char *in, char *out);
  int pfs_cmd_get_color (char *in, char *out);
  int pfs_cmd_get_text_params (char *in, char *out);
  int pfs_cmd_get_quad_params (char *in, char *out);

  int pfs_cmd_set_shader (char *in, char *out);
  int pfs_cmd_shader (char *in, char *out);

  int pfs_cmd_depth_test (char *in, char *out);
  int pfs_cmd_clickable (char *in, char *out);
  int pfs_cmd_set_depth_test (char *in, char *out);
  int pfs_cmd_goto_slide (char *in, char *out);

  int pfs_cmd_txt_clear (char *in, char *out);
  int pfs_cmd_txt_add_line (char *in, char *out);
  int pfs_cmd_txt_set_para (char *in, char *out);

  int pfs_cmd_txt_set_hl (char *in, char *out);
  int pfs_cmd_txt_set_hl_color (char *in, char *out);

  int pfs_cmd_set_xform (char *in, char *out);
  int pfs_cmd_get_xform (char *in, char *out);


  int pfs_cmd_add_stext (char *in, char *out);
  int pfs_cmd_add_group (char *in, char *out);
  int pfs_cmd_add_to_group (char *in, char *out);

#ifdef __cplusplus
}
#endif

#endif
