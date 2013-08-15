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

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <render_engine.h>

#include "pf_mesh.h"
#include "shader.h"

#include "render_engine_sys.h"

#include "matrix.h"
#include "render_engine.h"

// Constants
#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480


// EGL variables
static EGLDisplay  egl_disp      = 0;
static EGLConfig   egl_cfg       = 0;
static EGLSurface  egl_surf      = 0;
static EGLContext  egl_cntx      = 0;

// EGL native variables
static EGLNativeWindowType  native_window;
static EGLNativeDisplayType native_display;

static EGLint cntx_attrib[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

// Post processing related data

#define MAX_PS_TEX 2
#define MAX_PS_FBO 2

static GLuint   pf_ps_tex[MAX_PS_TEX];
static GLuint   pf_ps_db[MAX_PS_TEX];
static GLuint   pf_ps_fbo[MAX_PS_FBO];
static GLuint   pf_ps_fbo0 = 0;
static int      pf_ps_tex_size = 1024;
static PF_MESH  pf_ps_mesh = NULL;

static char *postproc_prg = NULL;
static int  _fbo_w, _fbo_h;


static GLfloat pf_ps_vert[] = {-1.0, -1.0, 0.0f, 
			       1.0, -1.0, 0.0f, 
			       -1.0, 1.0, 0.0f,
			       1.0, 1.0, 0.0f};

static float pf_ps_tc[] ={0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };

  
static GLfloat pf_ps_colors[] = {1.0f, 1.0f, 1.0f, 1.0,  
				 1.0f, 1.0f, 1.0f, 1.0,  
				 1.0f, 1.0f, 1.0f, 1.0,
				 1.0f, 1.0f, 1.0f, 1.0
};

static PF_MATRIX44  id_matrix;


int pf_re_postprocess_init ();


// Projection
static PF_MATRIX44 projection;
static PF_MATRIX44 modelview;
static float       light_pos[4];


int
pf_re_projection (PF_MATRIX proj)
{
  memcpy (projection, proj, sizeof (PF_MATRIX44));
  return 0;
}

PF_MATRIX
pf_re_get_projection ()
{
  return projection;
}

PF_MATRIX
pf_re_reset_modelview ()
{
  pfmat_identity (modelview);
  return modelview;
}


int
pf_re_set_modelview (PF_MATRIX proj)
{
  memcpy (modelview, proj, sizeof (PF_MATRIX44));
  return 0;
}

PF_MATRIX
pf_re_get_modelview ()
{
  return modelview;
}


int
pf_re_light_position (float *p)
{
  memcpy (light_pos, p, sizeof(float) * 3);

  return 0;
}

float*
pf_re_get_light_position ()
{
  return light_pos;
}

int 
egl_error(const char* str)
{
  /*
    eglGetError returns the last error that has happened using egl,
    not the status of the last called function. The user has to
    check after every single egl call or at least once every frame.
  */
  EGLint err = eglGetError();
  if (err != EGL_SUCCESS)
    {
      fprintf(stderr, "%s failed (%d).\n", str, err);
      return 0;
    }
  
  return 1;
}


int 
pf_re_init (int w, int h, int bpp)
{
  EGLint vmajor, vminor;
  EGLint cfg_attribs[7];
  int    n_cfg;

  pf_re_native_init (&native_display, &native_window, w, h);
  _fbo_w = w;
  _fbo_h = h;

  /* Initialising EGL */
  egl_disp = eglGetDisplay(native_display);

  /* Initialise EGL*/
  if (!eglInitialize (egl_disp, &vmajor, &vminor))
    {
      fprintf (stderr, "Error: eglInitialize() failed.\n");
      goto cleanup;
    }
  /* Make it current API */
  eglBindAPI(EGL_OPENGL_ES_API);
  
  if (!egl_error("eglBindAPI"))
    {
      goto cleanup;
    }


  /* Configuration Attributes */
  cfg_attribs[0] = EGL_SURFACE_TYPE;
  cfg_attribs[1] = EGL_WINDOW_BIT;
  cfg_attribs[2] = EGL_RENDERABLE_TYPE;
  cfg_attribs[3] = EGL_OPENGL_ES2_BIT;
  cfg_attribs[4] = EGL_DEPTH_SIZE;
  cfg_attribs[5] = 16;
  
  cfg_attribs[6] = EGL_NONE;


  /* Find Config */
  if (!eglChooseConfig (egl_disp, cfg_attribs, 
		       &egl_cfg, 1, &n_cfg) || 
      (n_cfg != 1))
    {
      fprintf (stderr, "Error: eglChooseConfig() failed.\n");
      goto cleanup;
    }



  /* Create surface to draw */
  egl_surf = eglCreateWindowSurface (egl_disp, egl_cfg, 
				     native_window, NULL);
  if (!egl_error("eglCreateWindowSurface"))
    {
      goto cleanup;
    }
  /* Create Context */
  egl_cntx = eglCreateContext (egl_disp, egl_cfg, NULL, 
				cntx_attrib);
  if (!egl_error("eglCreateContext"))
    {
      goto cleanup;
    }
  /* Bind Context */
  eglMakeCurrent (egl_disp, egl_surf, egl_surf, egl_cntx);
  if (!egl_error("eglMakeCurrent"))
    {
      goto cleanup;
    }
  
  glViewport(0, 0, w, h);

  // If Post Process???
  if (postproc_prg != NULL)
    pf_re_postprocess_init ();

  return 0;

cleanup:
  eglMakeCurrent (egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ;
  eglTerminate (egl_disp);
  return -1;
}


int
pf_re_end ()
{
  eglMakeCurrent (egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ;
  eglTerminate (egl_disp);

  pf_re_native_end ();

  return 0;
}


int
pf_re_set_clear_color (float r, float g, float b, float a)
{
  glClearColor (r, g, b, a);
  return 0;
}


int
pf_re_render (RENDER_SCENE_FUNC func)
{
  int err;
  // Process Render mode
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Enable backface culling and depth test

  glEnable(GL_DEPTH_TEST);

  if (postproc_prg)
    {
      // First render to FBO
      glBindFramebuffer(GL_FRAMEBUFFER, pf_ps_fbo[0]);
      
      if((err = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == 
	 GL_FRAMEBUFFER_COMPLETE)
	{
	  glViewport(0, 0, pf_ps_tex_size, pf_ps_tex_size);
	  
	  // Clear the screen by this colour
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	  
	  func ();
	}
      
      // Now select framebuffer
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
	  glBindFramebuffer(GL_FRAMEBUFFER, pf_ps_fbo0);

	  glBindTexture(GL_TEXTURE_2D, pf_ps_tex[0]);

	  // Render quad with texture and post-proc shader
	  glViewport(0, 0, _fbo_w, _fbo_h);
	  glUseProgram (get_shader (postproc_prg)); 
	  pf_mesh_render (pf_ps_mesh, id_matrix);
	  
	}
    }
else
  func ();


  
  eglSwapBuffers (egl_disp, egl_surf);

  return 0;

}

int
pf_re_set_post_proc_shader (char *name)
{
  if (!name) return -1;
  if (postproc_prg) free (postproc_prg);
  postproc_prg = strdup (name);
  fprintf (stderr, "Loading shader...'%s' for post processing...\n", 
	   postproc_prg);

  return 0;
}

/* Post processing */
/* FIXME: For this quick test we will only support a single Texture + FBO*/

int
pf_re_postprocess_init ()
{

  if (postproc_prg == NULL)
    pf_re_set_post_proc_shader ("post1");

  fprintf (stderr, "Initialising Post Processing system...\n");
  // Create texture... make them 1024 x 1024 to cover 800x600 resolution
  // XXX: This should be calculated according to screen resolution
  glGenTextures (1, pf_ps_tex);

  pf_ps_tex_size = 1024;
  // Binds this texture handle so we can load the data into it
  glBindTexture (GL_TEXTURE_2D, pf_ps_tex[0]);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 
		pf_ps_tex_size, pf_ps_tex_size, 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  printf ("Texture id: %d\n", pf_ps_tex[0]);

  // Create FBO
  
  glGenFramebuffers(1, pf_ps_fbo);
  egl_error ("Generate FBO..");
  /*
    Get the currently bound frame buffer object. 
    On most platforms this just gives 0.
  */
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&pf_ps_fbo0);
  printf ("Current FBO %d\n", pf_ps_fbo0);
  printf ("Generated FBO %d\n", pf_ps_fbo[0]);
  /*
    Attach the renderable objects (e.g. textures) to the frame buffer 
    object now as they will stay attached to the frame buffer object 
    even when it is not bound.
  */
  
  /*
    Firstly, to do anything with a frame buffer object we need to bind it. 
    In the case below we are binding our frame buffer object to the 
    frame buffer.
  */
  glBindFramebuffer(GL_FRAMEBUFFER, pf_ps_fbo[0]);
  egl_error ("Binding FB0");
  /*
    To render to a texture we need to attach it texture to the 
    frame buffer object. GL_COLOR_ATTACHMENT0 tells it to attach 
    the texture to the colour buffer, the 0 on the end refers to 
    the colour buffer we want to attach it to as a frame buffer object can
    have more than one colour buffer.
  */
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			  GL_TEXTURE_2D, pf_ps_tex[0], 0);
  
  egl_error ("Attache color texture..");
  // Clear the color buffer for this FBO
  glClear(GL_COLOR_BUFFER_BIT);

#if 1 
  /*
    Create and bind a depth buffer to the frame buffer object.
    
    A depth buffer isn't needed for this training course but will likely be 
    required for most uses of frame buffer objects so its attachment is being 
    demonstrated here.
  */

  // Generate and bind the handle for the render buffer 
  // (which will become our depth buffer)

  glGenRenderbuffers(1, &pf_ps_db[0]);
  egl_error ("Generate RenderBuffer..");
  glBindRenderbuffer(GL_RENDERBUFFER, pf_ps_db[0]);
  egl_error ("Attach render buffer..");
  /* 
     Currently it is unknown to GL that we want our new render buffer 
     to be a depth buffer. glRenderbufferStorage will fix this 
     and in this case will allocate a depth buffer of
    pf_ps_tex_size by pf_ps_tex_size.
  */

  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 
			 pf_ps_tex_size, pf_ps_tex_size);

  egl_error ("Define Render buffer storage as depth buffer..");

  // Now we have our depth buffer attach it to our frame buffer object.
  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
			     GL_RENDERBUFFER, pf_ps_db[0]);

  printf ("Render buffer %d\n", pf_ps_db[0]);
  egl_error ("Attache renderbuffer as depth buffer..");
#endif
  /*
    Unbind the frame buffer object so rendering returns back to the backbuffer.
  */

  glBindFramebuffer(GL_FRAMEBUFFER, pf_ps_fbo0);

  // Create mesh to render post process effects
  pf_ps_mesh = pf_mesh_new (4, 4);

  pf_mesh_set_vertex (pf_ps_mesh, pf_ps_vert);
  pf_mesh_set_color (pf_ps_mesh, pf_ps_colors);
  pf_mesh_set_texcoord (pf_ps_mesh, pf_ps_tc);


  pfmat_identity (id_matrix);

  return 0;
}
