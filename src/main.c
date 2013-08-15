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

#include <time.h>
#include <sys/time.h>

#include <unistd.h>


/* Network Specific */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <fcntl.h>

#include <math.h>

#include "matrix.h"
#include "render_engine.h"
#include "pf_tex_mng.h"
#include "pf_render_item.h"

#include "pf_font.h"
#include "pf_text.h"
#include "pf_image.h"

#include "pf_fx.h"

#include "input.h"
#include "pfs_files.h"

#include "pfs_cmd.h"
#include "video_net.h"

#include "const.h"

#include "log.h"


//#define SW_VERSION "0.4.0"
//#define SW_TYPE    "ALPHA"
#define VERSION_SLOGAN "Something is coming out!\n"


// FIXME: Global vars accessed from other modules
//        Define a proper interface
#define DEFAULT_FPS  50.0

int    generic_events = 1;
int    current_slide = 1;
int    transition = 1;
int    slide_show = 0;
int    slide_show_delay = 20;

int    tcp_port = 5000;
int    bt_channel = 1;

//int    transition_duration = 2;
int    transition_duration = 0;
float  rotx, roty, rotz;
int    current_item = -1;
float  fps = 0.0;       /* Frames per second */
float  sfps= 0.0;       /* Maximum Frames per second*/
float  ufps = DEFAULT_FPS;     /* User Frames per second */
float  fperiod = 1000000.0 / DEFAULT_FPS; /* frame period in usecs*/

float        w0, h0;
extern int (*cmd) ();

int ticks = 0;

int    reload = 0;
int    splash = 1;

int    screen_w = 640;
int    screen_h = 480;
int    screen_bpp = 24;
char   *pres_dir = NULL;
char   *pf_identity = NULL;

PF_RITEM ri_list[MAX_RITEMS];
int      n_ri_list = 0;


#define MAX_EFFECTS 6

typedef int (*BUILD_SLIDE)();

int build_splash ();

static   float lp[3] = {0.0, 0.0, 10.0};

int
init_system ()
{ 
  pf_fx_mng_init ();
  pf_tmng_init ();
  video_net_init ();

  return 0;
}

int
init_libraries ()
{
  return 0;
}

int
init_render_engine ()
{
  PF_MATRIX44  p;
  //float        v, size, w0, h0;
  float        v, size;
  float        near, far, fov;

  /* TODO: Move to a different file */
  /* Generate basic render engine data */
  pfmat_identity (p);
  v = (float)screen_h / (float)screen_w;
  fov = 60.0;
  near = 0.05;
  far = 100.0;

  size = tan ((fov * 3.14159f/ 180.0f) / 2.0);

  h0 = near * size;
  w0 = h0 / v;
  pfmat_frustum (p, -w0, w0, -h0, h0, near, far);


  pf_re_projection (p);
  pf_re_reset_modelview ();
  pf_re_light_position (lp);

  // Produce text style graphical objects
  // Init render list
  LOG("Starting Render Engine...\n");
  pf_re_init (screen_w, screen_h, screen_bpp);

  pf_font_mng_init ();
  init_shaders ();

  return 0;
}



int
render_scene ()
{
  int i;

  for (i = 0; i < n_ri_list; i++)
    {
      LOG ("Rendering Scene: item %d\n", i);
      /* Render items in the root group
       * Group nodes will take care of rendering anything under them
       */
      if (ri_list[i] && ri_list[i]->group < 0) ri_list[i]->render (ri_list[i]);
    }

  return 0;
}


int effect = 6;

int
scene_out (int ticks)
{ 
  int i, j;
  float  inc;
  if (ticks < 7) return 0;
  
  for (i = 1; i < n_ri_list; i++)
    {
      inc = (-0.05 - i / 50.0);
      switch (effect)
	{
	case 0:
	  {
	    pf_ritem_set_pos (ri_list[i], inc, 0.0, 0.0);
	    break;
	  }
	case 1:
	  {
	    pf_ritem_set_pos (ri_list[i], -inc, 0.0, 0.0);
	    break;
	  }
	case 2:
	  {
	    pf_ritem_set_pos (ri_list[i], 0.0, inc, 0.0);
	    break;
	  }
	case 3:
	  {
	    pf_ritem_set_pos (ri_list[i], 0.0, -inc, 0.0);
	    break;
	  }
	case 4:
	  {
	    pf_ritem_set_pos (ri_list[i], 0.0, 0.0, -inc / 6.0);
	    break;
	  }
	case 5:
	  {
	    pf_ritem_set_pos (ri_list[i], 0.0, 0.0, +inc / 4.0);
	    break;
	  }
	case 6:
	  {
	    // XXX: Not working with 3D models. 
	    for (j = 0; j < ri_list[i]->n_meshes; j++)
	      pf_mesh_set_rel_alpha (ri_list[i]->mesh[j], -0.015);
	    break;
	  }

	}

    }
  return 0;
}

int
process_args (int argc, char *argv[])
{
  int   i;

  pres_dir = strdup ("./");
  for (i = 0; i < argc; i++)
    {
      LOG ("Processing arg: %s\n", argv[i]);
      if (strcmp (argv[i], "--goto") == 0)
	{
	  current_slide = atoi (argv[i + 1]);
	  i++;
	  continue;
	}
      if (strcmp (argv[i], "--slideshow") == 0)
	{
	  slide_show = 1;
	  continue;
	}
      if (strcmp (argv[i], "--reload") == 0)
	{
	  reload = 1;
	  current_slide = atoi (argv[i + 1]);
	  i++;
	  continue;
	}

      if (strcmp (argv[i], "--fps") == 0)
	{
	  i++;
	  ufps = atof (argv[i]);
	  if (ufps == 0.0)
	    fperiod = 0.0;
	  else
	    fperiod = 1000000.0 / ufps;
	  LOG ("Setting fps = %f\n", ufps);
	  continue;
	}


      if (strcmp (argv[i], "--w") == 0)
	{
	  i++;
	  screen_w = atoi (argv[i]);
	  if (screen_w == 0) screen_w = 640;
	  LOG ("Setting width = %d\n", screen_w);
	  continue;
	}
      if (strcmp (argv[i], "--h") == 0)
	{
	  i++;
	  screen_h = atoi (argv[i]);
	  if (screen_h == 0) screen_h = 480;
	  LOG ("Setting height = %d\n", screen_h);
	  continue;
	}
      if (strcmp (argv[i], "--bpp") == 0)
	{
	  i++;
	  screen_bpp = atoi (argv[i]);
	  if (screen_bpp != 16) screen_bpp = 24;
	  LOG ("Setting bpp = %d\n", screen_bpp);
	  continue;
	}
      if (strcmp (argv[i], "--transition_duration") == 0)
	{
	  i++;
	  transition_duration = atoi (argv[i]);
	  LOG ("Setting Transition duration = %d secs\n", transition_duration);
	  continue;
	}

      if (strcmp (argv[i], "--dir") == 0)
	{
	  i++;
	  free (pres_dir);
	  pres_dir = strdup (argv[i]);
	  continue;
	}

      if (strcmp (argv[i], "--port") == 0)
	{
	  i++;
	  tcp_port = atoi (argv[i]);
	  continue;
	}

      if (strcmp (argv[i], "--bt-channel") == 0)
	{
	  i++;
	  bt_channel = atoi (argv[i]);
	  continue;
	}


      if (strcmp (argv[i], "--identity") == 0)
	{
	  i++;
	  free (pf_identity);
	  pf_identity = strdup (argv[i]);
	  continue;
	}
      if (strcmp (argv[i], "--postproc") == 0)
	{
	  i++;
	  pf_re_set_post_proc_shader (argv[i]);
	  continue;
	}
    }

  return 0;
}

int
build_splash ()
{
  n_ri_list = 0;
  pf_re_set_clear_color (0.0, 0.0, 0.0, 1.0);

  ri_list[n_ri_list] = pf_image_new ("splash.png", 20.0, 1);
  pf_ritem_set_pos (ri_list[n_ri_list], 0.0, 0.0, -10.0);
  n_ri_list++;

  return 0;
}



int
do_update ()
{
  int   i;

  /* Run autoupdate functions if any */
  for (i = 0; i < n_ri_list; i++)
    if (ri_list[i]->auto_update) ri_list[i]->auto_update (ri_list[i]);

  /* Run fx effects if any */
  pf_fx_mng_run ();

  return 0;
}

/* Transitions ****/
static time_t    t0, t1;
static int       (*anim) (int);

int
do_transition ()
{

  if (anim)
    {
      if (slide_show) anim (ticks);
      else anim (ticks + 6);
    }


  if (splash)
    {
      //if (ticks < 1) return 0;
      if (ticks < 3) return 0;
      load_slide ("slide", current_slide);
      splash = 0;
      time (&t0);
      ticks = 0;
    }
  

  if (slide_show) transition = 1;
  
  if ((reload) && (ticks > 4) && anim == NULL) 
    {
      load_slide ("slide", current_slide);
      time (&t0);
      ticks = 0;
      transition = 0;
      anim = NULL;
      LOG ("Automatic Reload!!!\n");
    }
  
  if ((slide_show) && (ticks > slide_show_delay - transition_duration)) 
    {
      anim = scene_out;
      transition = 1;	  
    }
  
  if (!slide_show && transition && anim == NULL)
    {
      LOG ("TRANSITION COMMAND!\n");
      anim = scene_out;
      time (&t0);
      ticks = 0;
    }
  
  if (((slide_show) && (ticks > slide_show_delay)) || 
      ((!slide_show) && (transition) && (ticks > transition_duration)))
    {
      LOG ("TRANSITION ENDS!!!\n");
      if (transition != 2)
	current_slide += transition;

      if (load_slide ("slide", current_slide) < 0) 
	{
	  current_slide = 1;

	  load_slide ("slide", current_slide);
	}
      
      time (&t0);
      ticks = 0;
      anim = NULL;
      transition = 0;
    }
  return 0;
}



/* process events */
int
do_events ()
{
  int            i, type, x, y;
  float          sx, sy;
  int            item_clicked = 0;
  static float   x1, y1, z1;
  static int     button, key;
  static int     last_focus = -1;

  button = 0;
  pf_re_events (&type, &x, &y, &key, &button);

  sx = 2.0 * ((float)x - (float) screen_w / 2.0) / (float) screen_w;
  sy = 2.0 * (((float) screen_h - (float) y) - (float) screen_h / 2.0) / (float) screen_h;


  if (type != PF_EV_NONE)
    {
      LOG ("(CURRENT: %d)Type: %d (but: %d) x:%d y:%d key:%d\n", 
	   current_item,
	   type, button, x, y, key);
      if (type == PF_EV_KEY)
	{
	  //printf ("KEY %d\n", key);
	  bc_printf ("KEY %d\n", key);
	  fflush (0);
	}
      if (type == PF_EV_MOUSE_CLICK)
	{
	  if (!generic_events)
	    {
	      float ix, iy, iz;
#if 0
	      if (button)
		{
		  for (i = 0; i < n_ri_list; i++)
		    if (ri_list[i]) 
		      {
			if (pf_ritem_test_bb (ri_list[i], sx, sy))
			  {
			    current_item = i;
			    break;
			  }
		      }
		}
#endif


	      if (current_item == n_ri_list) return 0;
	      pf_ritem_get_abs_pos (ri_list[current_item], &ix, &iy, &iz);
	      x1 = (float) x / (float) screen_w;
	      y1 = (float) y / (float) screen_h;
	      x1 -= 0.5;
	      y1 -= 0.5;
	      
	      if (button == 2)
		{
		  x1 *= 7.0;
		  y1 *= -7.0;
		  
		  pf_ritem_set_abs_pos (ri_list[current_item], 
					x1, y1, iz);
					//x1, y1, -2.0);
		  /*
		  printf ("SET_POS %s %f %f %f\n", 
			  ri_list[current_item]->name,
			  x1, y1, iz);
		  */
		  bc_printf ("SET_POS %s %f %f %f\n", 
			  ri_list[current_item]->name,
			  x1, y1, iz);

		}
	      else if (button == 3 || button == 4)
		{
		  x1 *= 5.0;
		  z1 = x1;
		  pf_ritem_set_abs_pos (ri_list[current_item], 
					ix, iy, z1);
					//0.0, 0.0, z1);
		  LOG ("z: %f\n", z1);
		  
		}
	  
	      else if (button == 1)
		{
		  x1 *= -7.0;
		  y1 *= 7.0;
		  pf_ritem_set_abs_rot (ri_list[current_item], 
					x1, y1, 0.0); //-3.4
		  /*
		  printf ("SET_ROT %s %f %f %f\n", 
			  ri_list[current_item]->name,
			  x1, y1, 0.0);
		  */
		  bc_printf ("SET_ROT %s %f %f %f\n", 
			  ri_list[current_item]->name,
			  x1, y1, 0.0);

		}
		  

	    }
	  else
	    {
	      /*
	      printf ("CLICK %d %f %f\n", button, 
		      (float)x / (float) screen_w, 
		      (float)y / (float) screen_h);
	      */
	      /* Check if we clicked on something */
	      item_clicked = 0;
	      for (i = 0; i < n_ri_list; i++)
		if (ri_list[i]) 
		  {
		    if (pf_ritem_test_bb (ri_list[i], sx, sy))
		      {
			if (button)
			  //printf ("CLICKED %s %d %f %f\n", ri_list[i]->name, button, sx, sy);
			  bc_printf ("CLICKED %s %d %f %f\n", ri_list[i]->name, button, sx, sy);
			else
			  {
			    if (i == last_focus)
			      //printf ("HOVER %s %d %f %f\n", ri_list[i]->name, button, sx, sy);
			      bc_printf ("HOVER %s %d %f %f\n", ri_list[i]->name, button, sx, sy);
			    else
			      {
				if (last_focus = -1)
				  {
				    //printf ("ENTER %s %d %f %f\n", ri_list[i]->name, button, sx, sy);
				    bc_printf ("ENTER %s %d %f %f\n", ri_list[i]->name, button, sx, sy);
				    last_focus = i;
				  }
			      }
			  }
			item_clicked = 1;
		      }
		  }
	      LOG ("%d found: %d last_focus:%d button:%d\n", 
		      item_clicked, i, last_focus, button);
	      if (!item_clicked && last_focus > 0)
		{
		  //printf ("EXIT %s %d %f %f\n", ri_list[last_focus]->name, button, sx, sy);
		  bc_printf ("EXIT %s %d %f %f\n", ri_list[last_focus]->name, button, sx, sy);
		  last_focus = -1;
		}
	      if (!item_clicked && button)
		//printf ("CLICK %d %f %f\n", button, sx, sy);
		bc_printf ("CLICK %d %f %f\n", button, sx, sy);

	      fflush (0);
	    }
	}
    }
  
  return 0;
}

/* process events ends*/

int
main (int argc, char *argv[])
{

  int            frames;
  struct timeval tf0, tf1, tf;
  double         tf_us;
 
  reload = 0;
  anim = NULL;

  pf_identity = strdup ("PF");

  /* Process arguments */
  process_args (argc, argv);


  //MSG ("picoFlamingo " SW_TYPE " Version " SW_VERSION "\n");
  MSG ("picoFlamingo " SW_TYPE " Version " PF_VERSION 
       "("__DATE__" "__TIME__")\n");
  MSG (VERSION_SLOGAN);
  MSG ("-----------------------------------------\n");
  MSG ("Starting up system...\n");	

  init_system ();
  init_libraries ();
  init_render_engine ();

  build_splash ();

  init_user_input_thread ();
  
  time (&t0);
  ticks = 0;
  anim = NULL;

  transition = slide_show;
  frames = 0;
  fps = 0.0;
  while (1)
    {
      gettimeofday (&tf0, NULL);
      do_update ();

      pf_re_render (render_scene);

      time (&t1);
      ticks = difftime (t1, t0);


      do_transition ();

      // TODO: Make this a queue??
      if (cmd) 
	{
	  cmd();
	  cmd = NULL;
	}
      do_events ();
      fflush (NULL);
      // FIXME: Effectivelly calculate frame rate


      // Calculate time diff
      // Uncomment for fixed frame rate
      // TODO: Make this configurable

      gettimeofday (&tf1, NULL);
      timersub (&tf1, &tf0, &tf);

      tf_us = (float)tf.tv_sec * 1000000.0 + (float)tf.tv_usec;
      sfps = 1000000.0 / tf_us;
      frames++;


#if 1
#if 0
      //printf ("Frame time: (%f, %f) %f\n", t1_us, t0_us, tf_us);
      if (tf_us < 20000.0)
	usleep (20000 - (int)tf_us);
#endif
      if (fperiod < 10.0)
	usleep (0);  // Free Run
      else
	if (tf_us < fperiod)
	  usleep ((int)(fperiod - tf_us));
#else
      // XXX: 2 ms is an arbitrary value... 
      //      Use 0 should provide maximum poerformance
      usleep (20000);
#endif

      gettimeofday (&tf1, NULL);
      timersub (&tf1, &tf0, &tf);
      tf_us = (float)tf.tv_sec + (float)tf.tv_usec / 1000000.0;
      fps = 1.0 / tf_us;
      frames++;
#if 0
      if ((frames % 500) == 0)
	printf ("Frame rate is %f (single frame: %f\n", fps, sfps);
#endif

    }
  
  return 0;
}
