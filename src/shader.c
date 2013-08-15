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

#include "log.h"

#include "const.h"

#include "shader.h"

#define MAX_SHADERS 128
#define BUF_SIZE 10240

typedef struct shader_item_t
{
  char *name;
  int   prog;
} SHADER_ITEM;

static SHADER_ITEM shader_list[MAX_SHADERS];
static int         n_shaders = 0;

int _compile_shader (int type, char *buffer);

int
get_shader (char *name)
{
  int i;

  for (i = 0; i < n_shaders; i++)
    if (strcmp (name, shader_list[i].name) == 0)
      return shader_list[i].prog;

  return -1;
}

int
_add_shader (char *name, char *attrib[])
{
  shader_list[n_shaders].name = strdup (name);
  shader_list[n_shaders].prog = shader_program (name, attrib);
  n_shaders++;

  return 0;
}

/* XXX: Get shader list from a file */
int
init_shaders ()
{
  FILE *f;
  int  l, line;
  char buffer[1024];



  char*    shader_attribs[] = {"myVertex", "myColor", "myNormal", "myUV", NULL};


  /* Try to open local shader_list */
  if ((f = fopen ("shader_list.txt", "rt")) == NULL)
    {
      fprintf (stderr, "Cannot open local shader_list.txt\n");
      if ((f = fopen (DATA_DIR "/shader_list.txt", "rt")) == NULL)
	{
	  fprintf (stderr, "Cannot open system shader_list.txt\n");
	  return -1;
	}
    }
  line = 0;
  while  (!feof (f))
    {
      line++;
      if (!fgets (buffer, 1024, f)) break;

      //LOG ("Processing line '%s'\n", buffer);
      l = strlen (buffer);
      if (buffer[0] == ';') continue;
      buffer[l - 1] = 0; // CHOMP

      printf ("Registering '%s' shader\n", buffer);

      _add_shader (buffer, shader_attribs);
    }

  fclose (f);

  LOG ("Done!\n");


  return 0;
}

int
load_shader (int type, char *fname1)
{
  GLuint shader;
  FILE   *f;
  char   *buffer;
  int    len;
  char   fname[2048];

  buffer = malloc (BUF_SIZE);
  memset (buffer, 0, BUF_SIZE);
  /* Open File and read it*/
  snprintf (fname, 2048, "./shaders/%s", fname1);
  if ((f = fopen (fname, "rb")) == NULL)
    {
      snprintf (fname, 2048, DATA_DIR "/shaders/%s", fname1);
      if ((f = fopen (fname, "rb")) == NULL)
	{

	  fprintf (stderr, "Cannot open file '%s'\n", fname);
	  free (buffer);
	  return -1;
	}

    }
  len = 0;
  while (!feof (f))
    {
      len += fread (buffer + len, 1, 1024, f);
    }
  fclose (f);

  /* Create Shader */
  shader = _compile_shader (type, buffer);
  if (shader < 0)
    fprintf (stderr, "*** Cannot compile shader '%s'\n", fname);

  return shader;
}

int
_compile_shader (int type, char *buffer)
{
  GLuint shader;
  int    compiled;

  shader = glCreateShader (type);

  glShaderSource(shader, 1, (const char**)&buffer, NULL);

  glCompileShader (shader);

  /* Check for compilation errors */
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled)
    {
      /* Get and print error messages */
      int       len, cw;
      char     *log;

      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
      
      log = malloc (len);
      glGetShaderInfoLog(shader, len, &cw, log);
      
      fprintf (stderr, "Failed to compile fragment shader: %s\n", log);
      free (log);

      return -1;
    }

  free (buffer);

  return shader;
}


/* FIXME: This should store programs in a "cache" including shader to
 *        reuse them
 */
int 
shader_program (char *name, char *sh_attrib[])
{
  char fname[1024];
  int  vs, fs, prg, i = 0;

  /* Load shaders */
  snprintf (fname, 1024, "%s.vert", name);
  vs = load_shader (GL_VERTEX_SHADER, fname);

  snprintf (fname, 1024, "%s.frag", name);
  fs = load_shader (GL_FRAGMENT_SHADER, fname);

  prg = glCreateProgram();

  /* Attach shaders */
  glAttachShader(prg, vs);
  glAttachShader(prg, fs); 

  while (sh_attrib[i] != NULL)
    {
      glBindAttribLocation (prg, i, sh_attrib[i]);
      i++;
    }

  /* Linking */
  glLinkProgram(prg);

  /* Check for linking error */
  GLint bLinked;
  glGetProgramiv(prg, GL_LINK_STATUS, &bLinked);
  
  if (!bLinked)
    {
      int len, cw;
      char *log;

      glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &len);
      log = malloc (len);
      glGetProgramInfoLog(prg, len, &cw, log);

      fprintf (stderr, "Failed to link program: %s\n", log);
      free (log);

      return -1;
    }

  return prg;
}
