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

#ifndef THREE_DS_H
#define THREE_DS_H

typedef struct color_t
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
} COLOR;

typedef struct vector3D_t
{
  float   x;
  float   y;
  float   z;
} VECTOR3D;

typedef struct Vector3D_int_t
{
  unsigned short    a;
  unsigned short    b;
  unsigned short    c;
  unsigned short    flags;
} VECTOR4D_INT;

typedef struct vector2D_t
{
  float   u;
  float   v;
} VECTOR2D;

typedef struct material_t
{
  char      *name;
  char      *texture;
  int 	    id_tex;
  //int       id_tex;
  COLOR     color;
  /* Other material parameters follow */
} MATERIAL;

typedef struct mesh3D_t
{
  long         n_vertex;
  long         n_poly;
  long         n_map;
  VECTOR3D     *vertex;
  VECTOR3D     *normal;
  VECTOR4D_INT *poly;
  VECTOR2D     *map;
  int          mat;
} MESH3D;

typedef struct object3D_t
{
  char      *name;
  int       indx;       /* Current mesh being processed */
  int       mat_indx;   /* Current material being processed */
  int       n_meshes;
  int       n_mats;
  MESH3D    *mesh;
  MATERIAL  *mat_list;
} OBJ3D;

/* Prototypes */
int loader_3ds (OBJ3D *obj, char *fname);
int loader_3ds_free (OBJ3D *obj);
#endif
