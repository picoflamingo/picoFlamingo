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

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "shader.h"
#include "matrix.h"

#include "pf_mesh.h"
#include "render_engine.h"

#define VERTEX_ARRAY    0
#define COLOR_ARRAY     1
#define NORMAL_ARRAY    2
#define TEXCOORD_ARRAY  3

int egl_error (const char *str);

PF_MESH 
pf_mesh_new (int n_vertex, int n_index)
{
  PF_MESH   m;

  if ((m = malloc (sizeof(struct pf_mesh_t))) == NULL)
    {
      fprintf (stderr, "Cannot allocate memory for 3D object\n");
      exit (1);
    }

  m->n_vertex = n_vertex;
  m->n_index = n_index;

  // Initialise data structure
  m->vertex = NULL;
  m->index = NULL;
  m->normal = NULL;
  m->color = NULL;
  m->texcoord = NULL;
  m->vert_shader = m->frag_shader = m->program = 0;
  
  return m;
}

int     
pf_mesh_free (PF_MESH m)
{
  if (!m) return -1;
  if (m->vertex) free (m->vertex);
  if (m->index) free (m->index);
  if (m->color) free (m->color);
  if (m->normal) free (m->normal);

  // Reset data structure
  m->vertex = NULL;
  m->index = NULL;
  m->normal = NULL;
  m->color = NULL;
  m->texcoord = NULL;
  m->n_vertex = m->n_index = 0;


  free (m);
  return 0;
}

int     
pf_mesh_set_zc_vertex (PF_MESH m, float *vertex)
{
  int  size;

  if (!m) return -1;
  if (!vertex) return -1;

  size = sizeof(GLfloat) * 3 * m->n_vertex;
  if (m->vertex == NULL) 
    {
      m->vertex = malloc (size);
    }
  memcpy (m->vertex, vertex, size);

  return 0;
}

int     
pf_mesh_set_vertex (PF_MESH m, float *vertex)
{
  int  size;

  if (!m) return -1;
  if (!vertex) return -1;

  size = sizeof(GLfloat) * 3 * m->n_vertex;
  if (m->vertex) free (m->vertex);

  m->vertex = malloc (size);
  memcpy (m->vertex, vertex, size);

  return 0;
}

float *
pf_mesh_get_vertex (PF_MESH m)
{
  if (!m) return NULL;
  return m->vertex;
}

int
pf_mesh_set_texcoord (PF_MESH m, float *texcoord)
{
  int size;
  
  if (!m) return -1;
  if (!texcoord) return -1;


  size = sizeof(GLfloat) * 2 * m->n_vertex;
  if (m->texcoord) free (m->texcoord);

  m->texcoord = malloc (size);
  memcpy (m->texcoord, texcoord, size);

  return 0;
}


int
pf_mesh_set_zc_texcoord (PF_MESH m, float *texcoord)
{
  int size;
  
  if (!m) return -1;
  if (!texcoord) return -1;


  size = sizeof(GLfloat) * 2 * m->n_vertex;
  if (m->texcoord == NULL)
    m->texcoord = malloc (size);

  memcpy (m->texcoord, texcoord, size);

  return 0;
}

float*
pf_mesh_get_texcoord (PF_MESH m)
{
  if (!m) return NULL;
  return m->texcoord;
}

int     
pf_mesh_set_index  (PF_MESH m, int   *index)
{
  int  size;

  if (!m) return -1;
  if (!index) return -1;

  size = sizeof(int) * m->n_index;
  if (m->index) free (m->index);

  m->index = malloc (size);
  memcpy (m->index, index, size);

  return 0;
}

int     
pf_mesh_set_normal (PF_MESH m, float *normal)
{
  int  size;

  if (!m) return -1;
  if (!normal) return -1;

  size = sizeof(GLfloat) * 3 * m->n_vertex;
  if (!m->normal) free (m->normal);

  m->normal = malloc (size);
  memcpy (m->normal, normal, size);

  return 0;
}

int     
pf_mesh_set_color  (PF_MESH m, float *color)
{
  int  size;

  if (!m) return -1;
  if (!color) return -1;

  size = sizeof(GLfloat) * 4 * m->n_vertex;
  if (m->color) free (m->color);

  m->color = malloc (size);
  memcpy (m->color, color, size);

  return 0;
}

int     
pf_mesh_set_rel_alpha  (PF_MESH m, float alpha)
{
  int  i;

  if (!m) return -1;
  if (!m->color) return -1;

  for (i = 0; i < m->n_vertex; i++)
    {
      m->color[i * 4 + 3] += alpha;
      if (m->color[i * 4 + 3] < 0.0) m->color[i * 4 + 3] = 0.0;
    }

  return 0;
}

int     
pf_mesh_set_prg (PF_MESH m, int prg)
{
  if (!m) return -1;

  m->program = prg;
  return 0;
}

int
pf_mesh_get_program (PF_MESH m)
{
  if (!m) return -1;

  return m->program;
}

int     
pf_mesh_set_program (PF_MESH m, char* vertex, char* fragment)
{
  m->vert_shader = load_shader (GL_VERTEX_SHADER, vertex);
  m->frag_shader = load_shader (GL_FRAGMENT_SHADER, fragment);
  
  // Create the shader program
  m->program = glCreateProgram();

  // Attach the fragment and vertex shaders to it
  glAttachShader(m->program, m->frag_shader);
  glAttachShader(m->program, m->vert_shader);

  // Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
  if (m->vertex)
    glBindAttribLocation(m->program, VERTEX_ARRAY, "myVertex");
  if (m->color)
    glBindAttribLocation(m->program, COLOR_ARRAY, "myColor");
  if (m->normal)
    glBindAttribLocation(m->program, NORMAL_ARRAY, "myNormal");
  if (m->texcoord)
    glBindAttribLocation(m->program, TEXCOORD_ARRAY, "myUV");
  
  // Link the program
  glLinkProgram(m->program);

  // Check if linking succeeded in the same way we checked 
  // for compilation success
  GLint bLinked;
  glGetProgramiv(m->program, GL_LINK_STATUS, &bLinked);
  
  if (!bLinked)
    {
      int log_len, n;
      glGetProgramiv(m->program, GL_INFO_LOG_LENGTH, &log_len);
      char* log_str = malloc (log_len);
      glGetProgramInfoLog(m->program, log_len, &n, 
			  log_str);
      fprintf (stderr, "Failed to link program: %s\n", log_str);
      free (log_str);
      return -1;
    }

  return 0;
}

int
pf_mesh_render (PF_MESH m, PF_MATRIX44 modelView)
{
  int i1, i2, i3, i;

  i1 = glGetUniformLocation(m->program, "myPMVMatrix"); 
  glUniformMatrix4fv( i1, 1, GL_FALSE, modelView);

  i2 = glGetUniformLocation(m->program, "lightPosition"); 
  glUniform3fv (i2, 1, pf_re_get_light_position());


  i3 = glGetUniformLocation(m->program, "myModelViewIT"); 
  glUniformMatrix4fv (i3, 1, GL_FALSE, modelView);

  if (m->texcoord)
    {
      glUniform1i(glGetUniformLocation(m->program, "sampler2d"), 0);
    }

  
  glEnableVertexAttribArray(VERTEX_ARRAY);
  if (m->vertex)
    glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, m->vertex);
  else
    fprintf (stderr,"OOps no vertex array on mesh\n");

  if (m->color)
    {
      glEnableVertexAttribArray(COLOR_ARRAY);
      glVertexAttribPointer(COLOR_ARRAY, 4, GL_FLOAT, GL_FALSE, 0, m->color);
    }
  if (m->normal)
    {
      glEnableVertexAttribArray(NORMAL_ARRAY);
      glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, m->normal);
    }

  if (m->texcoord)
    {
      glEnableVertexAttribArray(TEXCOORD_ARRAY);
      glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, 
			    m->texcoord);
    }
  

  if (m->index)
    glDrawElements ( GL_TRIANGLES, m->n_index, GL_UNSIGNED_INT, m->index);
  else
    glDrawArrays (GL_TRIANGLE_STRIP, 0, m->n_vertex);

  if (!egl_error("glDrawArrays"))
    {
      return -1;
    }

  glDisableVertexAttribArray(VERTEX_ARRAY);  

  if (m->color)
    glDisableVertexAttribArray(COLOR_ARRAY);  

  if (m->normal)
    glDisableVertexAttribArray(NORMAL_ARRAY);  

  if (m->texcoord)
    glDisableVertexAttribArray(TEXCOORD_ARRAY);  

  return 0;
  
}


int
pf_mesh_render2 (PF_MESH m, PF_MATRIX44 modelView, PF_MATRIX44 normal)
{
  PF_MATRIX44 it;
  int i1, i2, i3;
 
  i1 = glGetUniformLocation(m->program, "myPMVMatrix"); 
  glUniformMatrix4fv( i1, 1, GL_FALSE, modelView);

  i2 = glGetUniformLocation(m->program, "lightPosition"); 
  glUniform3fv (i2, 1, pf_re_get_light_position());

  // XXX: Calculate MV inverse (transpose)
  it[0] = normal [0];
  it[1] = normal [1];
  it[2] = normal [2];
  it[3] = 0.0;

  it[4] = normal [4];
  it[5] = normal [5];
  it[6] = normal [6];
  it[7] = 0.0;

  it[8] = normal [8];
  it[9] = normal [9];
  it[10] = normal [10];
  it[11] = 0.0;

  it[12] = 0.0;
  it[13] = 0.0;
  it[14] = 0.0;
  it[15] = 0.0;


  i3 = glGetUniformLocation(m->program, "myModelViewIT"); 

  glUniformMatrix4fv (i3, 1, GL_FALSE, it);

  if (m->texcoord)
    {
      glUniform1i(glGetUniformLocation(m->program, "sampler2d"), 0);
    }
 
  glEnableVertexAttribArray(VERTEX_ARRAY);
  glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, m->vertex);

  if (m->color)
    {
      glEnableVertexAttribArray(COLOR_ARRAY);
      glVertexAttribPointer(COLOR_ARRAY, 4, GL_FLOAT, GL_FALSE, 0, m->color);
    }
  if (m->normal)
    {
      glEnableVertexAttribArray(NORMAL_ARRAY);
      glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, m->normal);
    }

  if (m->texcoord)
    {
      glEnableVertexAttribArray(TEXCOORD_ARRAY);
      glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, 
			    m->texcoord);
    }
  

  if (m->index)
    glDrawElements ( GL_TRIANGLES, m->n_index, GL_UNSIGNED_INT, m->index);
  else
    glDrawArrays (GL_TRIANGLE_STRIP, 0, m->n_vertex);

  if (!egl_error("glDrawArrays"))
    {
      return -1;
    }

  glDisableVertexAttribArray(VERTEX_ARRAY);  

  if (m->color)
    glDisableVertexAttribArray(COLOR_ARRAY);  

  if (m->normal)
    glDisableVertexAttribArray(NORMAL_ARRAY);  

  if (m->texcoord)
    glDisableVertexAttribArray(TEXCOORD_ARRAY);  

  
  return 0;
  
}
