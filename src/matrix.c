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

#include "log.h"
#include "matrix.h"


const PF_MATRIX44 pf_id = {1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0};

void
pfmat_identity (PF_MATRIX m)
{
  memcpy (m, pf_id, sizeof (PF_MATRIX44));

}


void 
pfmat_mul (PF_MATRIX r, PF_MATRIX a, PF_MATRIX b)
{
  PF_MATRIX44 t;

  t[0] = a[0] *b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
  t[1] = a[0] *b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
  t[2] = a[0] *b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
  t[3] = a[0] *b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

  t[4] = a[4] *b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
  t[5] = a[4] *b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
  t[6] = a[4] *b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
  t[7] = a[4] *b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

  t[8]  = a[8] *b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
  t[9]  = a[8] *b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
  t[10] = a[8] *b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
  t[11] = a[8] *b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

  t[12] = a[12] *b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
  t[13] = a[12] *b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
  t[14] = a[12] *b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
  t[15] = a[12] *b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];

  memcpy (r, t, sizeof(PF_MATRIX44));

}
void      
pfmat_vmul (float *r, PF_MATRIX a, float *v )
{
  float w;

#if 0
  w = a[12] * v[0] + a[13] * v[1] + a[14] * v[2] + a[15];

  r[0] = (a[0] * v[0] + a[1] * v[1] + a[2] * v[2] + a[3]) / w;
  r[1] = (a[4] * v[0] + a[5] * v[1] + a[6] * v[2] + a[7]) / w;
  r[2] = (a[8] * v[0] + a[9] * v[1] + a[10] * v[2] + a[11]) / w;
#endif 

  w = a[3] * v[0] + a[7] * v[1] + a[11] * v[2] + a[15];
  r[0] = (a[0] * v[0] + a[4] * v[1] + a[8] * v[2] + a[12]) / w;
  r[1] = (a[1] * v[0] + a[5] * v[1] + a[9] * v[2] + a[13]) / w;
  r[2] = (a[2] * v[0] + a[6] * v[1] + a[10] * v[2] + a[14]) / w;


}  


PF_MATRIX
pfmat_frustum (PF_MATRIX m,
	       float       left, float       right,
	       float       bottom, float       top,
	       float       z_near, float       z_far)
{
  float x, y, a, b, c, d;
  PF_MATRIX frustum;

  frustum = malloc (sizeof(PF_MATRIX44));
  x = (2.0f * z_near) / (right - left);
  y = (2.0f * z_near) / (top - bottom);
  a = (right + left) / (right - left);
  b = (top + bottom) / (top - bottom);
  c = -(z_far + z_near) / ( z_far - z_near);
  d = -(2.0f * z_far* z_near) / (z_far - z_near);

  frustum[0] = x;
  frustum[1] = 0.0f;
  frustum[2] = 0.0f;
  frustum[3] = 0.0f;

  frustum[4] = 0.0f;
  frustum[5] = y;
  frustum[6] = 0.0f;
  frustum[7] = 0.0f;

  frustum[8] = a;
  frustum[9] = b;
  frustum[10] = c;
  frustum[11] = -1.0f;

  frustum[12] = 0.0f;
  frustum[13] = 0.0f;
  frustum[14] = d;
  frustum[15] = 0.0f;

  pfmat_mul (m, frustum, m);
  free (frustum);
  return m;
}


PF_MATRIX
pfmat_frustum2 (PF_MATRIX m,
	       float       left, float       right,
	       float       bottom, float       top,
	       float       z_near, float       z_far)
{
  float x, y, a, b, c, d;
  PF_MATRIX frustum;

  frustum = malloc (sizeof(PF_MATRIX44));
  x = (2.0f * z_near) / (right - left);
  y = (2.0f * z_near) / (top - bottom);
  a = (right + left) / (right - left);
  b = (top + bottom) / (top - bottom);
  c = -(z_far + z_near) / ( z_far - z_near);
  d = -(2.0f * z_far* z_near) / (z_far - z_near);

  frustum[0] = x;
  frustum[4] = 0.0f;
  frustum[8] = 0.0f;
  frustum[12] = 0.0f;

  frustum[1] = 0.0f;
  frustum[5] = y;
  frustum[9] = 0.0f;
  frustum[13] = 0.0f;

  frustum[2] = a;
  frustum[6] = b;
  frustum[10] = c;
  frustum[14] = -1.0f;

  frustum[3] = 0.0f;
  frustum[7] = 0.0f;
  frustum[11] = d;
  frustum[15] = 0.0f;

  pfmat_mul (m, m, frustum);
  free (frustum);
  return m;
}


void 
pfmat_rotation (PF_MATRIX m, float a, float b, float c)
{
  PF_MATRIX44 y;

  memset (m, 0, PF_MATRIX44_SIZE);
  memset (y, 0, PF_MATRIX44_SIZE);
  m[0] = cos (a);
  m[2] = sin (a);
  m[5] = 1.0;
  m[8] = -sin (a);
  m[10] = cos (a);
  m[15] = 1.0;
  

  y[0] = 1.0;
  y[5] = cos (b);
  y[6] = sin (b);
  y[9] = -sin (b);
  y[10] = cos (b);
  y[15] = 1.0;
  pfmat_mul (m, m, y);


  memset (y, 0, PF_MATRIX44_SIZE);
  
  y[0] = cos (c);
  y[1] = sin (c);
  y[4] = -sin (c);
  y[5] = cos (c);
  y[10] = 1;
  y[15] = 1.0;
  pfmat_mul (m, m, y);
  
  

}


void 
pfmat_translate (PF_MATRIX m, float a, float b, float c)
{
  memcpy (m, pf_id, PF_MATRIX44_SIZE);

  m[0] = m[5] = m[10] = m[15] = 1.0;
  m[12] = a;
  m[13] = b;
  m[14] = c;
}


void
pfmul_translate (PF_MATRIX m, float tx, float ty, float tz)
{
  m[3]  += (m[0] * tx + m[1] * ty + m[2] * tz);
  m[7]  += (m[4] * tx + m[5] * ty + m[6] * tz);
  m[11] += (m[8] * tx + m[9] * ty + m[10] * tz);
  m[15] += (m[12] * tx + m[13] * ty + m[14] * tz);
}

void
pfmat_imodelview (PF_MATRIX m, PF_MATRIX i)
{
  memset (i, 0, PF_MATRIX44_SIZE);

  /* Transpose rotation */
  i[0]  = m[0];
  i[1]  = m[4];
  i[2]  = m[8];

  i[4]  = m[1];
  i[5]  = m[5];
  i[6]  = m[9];

  i[8]  = m[2];
  i[9]  = m[6];
  i[10] = m[7];

  pfmul_translate (i, -m[12], -m[13], -m[14]);
}


void
pfmat_print (PF_MATRIX m, char *title)
{
  LOG ("-[%s]------------------------\n", title);
  LOG ("%6.2f %6.2f %6.2f %6.2f\n", m[0], m[4], m[8], m[12]);
  LOG ("%6.2f %6.2f %6.2f %6.2f\n", m[1], m[5], m[9], m[13]);
  LOG ("%6.2f %6.2f %6.2f %6.2f\n", m[2], m[6], m[10], m[14]);
  LOG ("%6.2f %6.2f %6.2f %6.2f\n", m[3], m[7], m[11], m[15]);
  LOG ("-------------------------\n");
}
