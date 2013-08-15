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

#ifndef MATRIX_H
#define MATRIX_H

typedef float PF_MATRIX44[16];
typedef float PF_MATRIX33[9];
typedef float *PF_MATRIX;


#define PF_MATRIX44_SIZE sizeof(float) * 16
#define PF_MATRIX33_SIZE sizeof(float) * 9

#ifdef __cplusplus
extern "C" {
#endif

  void      pfmat_print (PF_MATRIX m, char *title);
  void      pfmat_identity (PF_MATRIX m);
  void      pfmat_mul (PF_MATRIX r, PF_MATRIX a, PF_MATRIX b);
  void      pfmat_vmul (float *r, PF_MATRIX a, float *v);
  void      pfmat_rotation (PF_MATRIX m, float a, float b, float c);
  void      pfmat_translate (PF_MATRIX m, float a, float b, float c);
  PF_MATRIX pfmat_frustum (PF_MATRIX m, 
		      float       left, float       right,
		      float       bottom, float       top,
		      float       z_near, float       z_far);
  PF_MATRIX pfmat_frustum2 (PF_MATRIX m, 
		      float       left, float       right,
		      float       bottom, float       top,
		      float       z_near, float       z_far);

  void pfmul_translate (PF_MATRIX m, float a, float b, float c);
  void pfmat_imodelview (PF_MATRIX m, PF_MATRIX i);
#ifdef __cplusplus
}
#endif


#endif
