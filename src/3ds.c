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

/* Acknowledge:
 *
 * Initial versions of this code was based on 3DS Loader tutorial from:
 *   http://www.spacesimulator.net/tut4_3dsloader.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>

#include "pf_tex_mng.h"
#include "3ds.h"

#include "log.h"

long 
file_size (int f)
{
  struct stat buf;
  
  if ((fstat (f, &buf)) < 0)
    return -1;
  
  return buf.st_size;
}

void
_vector_sub (VECTOR3D a, VECTOR3D b, VECTOR3D *c)
{
  c->x = a.x - b.x;
  c->y = a.y - b.y;
  c->z = a.z - b.z;
}

void
_vector_product (VECTOR3D v1, VECTOR3D v2, VECTOR3D *r)
{
  r->x = ((v1.y * v2.z) - (v1.z * v2.y));
  r->y = ((v1.z * v2.x) - (v1.x * v2.z));
  r->z = ((v1.x * v2.y) - (v1.y * v2.x));
  
}

int
_calculate_current_mesh_normals (OBJ3D *obj)
{
  VECTOR3D    a, b;
  VECTOR3D    v1, v2, n;
  double      m;
  int         i, indx;
  
  indx = obj->indx;

  /* Allocate memory for normals */
  if ((obj->mesh[indx].normal = malloc (sizeof(VECTOR3D) * obj->mesh[indx].n_poly)) == NULL)
    {
      fprintf (stderr, "Cannot allocate memory for mesh %d normals\n", indx);
      return -1;
    }
  LOG ("Calculating %ld normals\n", obj->mesh[indx].n_poly);
  for (i = 0; i < obj->mesh[indx].n_poly; i++)
    {

      /* Calcula polygon vectors */
      a.x = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].a].x;
      a.y = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].a].y;
      a.z = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].a].z;

      b.x = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].c].x;
      b.y = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].c].y;
      b.z = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].c].z;

      _vector_sub (a, b, &v1);

      a.x = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].b].x;
      a.y = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].b].y;
      a.z = obj->mesh[indx].vertex[obj->mesh[indx].poly[i].b].z;

      _vector_sub (b, a, &v2);

      /* Calculate normal */
      _vector_product (v1, v2, &n);

      /* Normalize and store */
      m = sqrt ((n.x * n.x) + (n.y * n.y) + (n.z * n.z));
  

      obj->mesh[indx].normal[i].x = n.x / m;
      obj->mesh[indx].normal[i].y = n.y / m;
      obj->mesh[indx].normal[i].z = n.z / m;

    }

  return 0;
}

/* ****************************************************************** */
/* Helper function for file access  */
/* ****************************************************************** */
int
_get_chunck (FILE *f, unsigned short *id, unsigned int *len)
{
  if ((fread (id, 2, 1, f)) < 0)
    return -1;

  if ((fread (len, 4, 1, f)) < 0)
    return -1;

  return 0;
}

int
_get_strz (FILE *f, char *buf, int len)
{
  int     i = 0;

  do
    {
      fread (&buf[i], 1, 1, f);
      i++;
    } while (buf[i - 1] != 0 && i < len);

  LOG ("Read strz : '%s' (%d bytes)\n", buf, i);
  return 0;
}

unsigned short
_get_short (FILE *f)
{
  unsigned short temp;

  if ((fread (&temp, sizeof (unsigned short), 1, f)) < 0)
    return -1;
  else
    return temp;
}

int
_get_3Dvector (FILE *f, VECTOR3D *v)
{
  if ((fread (&v->x, sizeof(float), 1, f)) < 0) return -1;
  if ((fread (&v->y, sizeof(float), 1, f)) < 0) return -1;
  if ((fread (&v->z, sizeof(float), 1, f)) < 0) return -1;

  return 0;
}


int
_get_2Dvector (FILE *f, VECTOR2D *v)
{
  if ((fread (&v->u, sizeof(float), 1, f)) < 0) return -1;
  if ((fread (&v->v, sizeof(float), 1, f)) < 0) return -1;

  //LOG ("Mapping coordinates: %f, %f\n", v->u, v->v);

  return 0;
}


int
_get_4Dvector_int (FILE *f, VECTOR4D_INT *v)
{

  if ((fread (&v->a, sizeof(unsigned short), 1, f)) < 0) return -1;
  if ((fread (&v->b, sizeof(unsigned short), 1, f)) < 0) return -1;
  if ((fread (&v->c, sizeof(unsigned short), 1, f)) < 0) return -1;
  if ((fread (&v->flags, sizeof(unsigned short), 1, f)) < 0) return -1;

  return 0;
}

int
_get_color (FILE *f, MATERIAL *m)
{
  if ((fread (&m->color, sizeof(COLOR), 1, f)) < 0) return -1;
  LOG ("Read Color: (%d,%d,%d)\n", m->color.r, m->color.g, m->color.b);
  return 0;
}

int
_get_material (OBJ3D *obj, char *mat_name)
{
  int   i;

  for (i = 0; i < obj->n_mats; i++)
    {
      if (strcmp (obj->mat_list[i].name, mat_name) == 0)
	return i;
    }
  return -1;
}

int
_skip_current_chunk (FILE *f, unsigned int len)
{
  return fseek (f, len - 6, SEEK_CUR);
}

int
_dump_obj3d (OBJ3D *obj)
{
  int             i, j;
  float           bbx1, bbx2, bby1, bby2, bbz1, bbz2;
  float           x1, y1, z1, max;
  float           sx;
  //float           sx, sy, sz;

  /* Initialise bounding box*/
  bbx1 = bby1 = bbz1= 1000000;
  bbx2 = bby2 = bbz2= -1000000;

  LOG ("%s", "\n---------------------------------------------\n");
  LOG ("%s", "Material list:\n");
  for (i = 0; i < obj->n_mats; i++)
    {
      LOG ("Material %d: %s [%s]", i, obj->mat_list[i].name, 
	      obj->mat_list[i].id_tex == -1 ? "COLOR" : "TEXTURE");
      if (obj->mat_list[i].id_tex == -1)
	LOG (" (%3d,%3d,%3d", 
		obj->mat_list[i].color.r,
		obj->mat_list[i].color.g,
		obj->mat_list[i].color.b
		);
      LOG ("\n");
    }
  //obj->mat_list[i].id_tex == NULL ? "COLOR" : "TEXTURE");

   LOG ("Object has %d meshes:\n", obj->n_meshes);
   for (i = 0; i < obj->n_meshes; i++)
     LOG ("Mesh %d has %ld vertex and %ld polygons with material %s\n", i,
	     obj->mesh[i].n_vertex, obj->mesh[i].n_poly,
	     obj->mat_list[obj->mesh[i].mat].name);
   for (i = 0; i < obj->n_meshes; i++)
     {
       for (j = 0; j < obj->mesh[i].n_vertex; j++)
	 {
	   if (obj->mesh[i].vertex[j].x > bbx2) bbx2 = obj->mesh[i].vertex[j].x;
	   if (obj->mesh[i].vertex[j].x < bbx1) bbx1 = obj->mesh[i].vertex[j].x;

	   if (obj->mesh[i].vertex[j].y > bby2) bby2 = obj->mesh[i].vertex[j].y;
	   if (obj->mesh[i].vertex[j].y < bby1) bby1 = obj->mesh[i].vertex[j].y;

	   if (obj->mesh[i].vertex[j].z > bbz2) bbz2 = obj->mesh[i].vertex[j].z;
	   if (obj->mesh[i].vertex[j].z < bbz1) bbz1 = obj->mesh[i].vertex[j].z;
	 }
     }

   x1 = (bbx2 -bbx1) / 2.0f;
   y1 = (bby2 -bby1) / 2.0f;
   z1 = (bbz2 -bbz1) / 2.0f;

   max = y1;
   if (x1 > y1) max = x1;
   if (z1 > max) max = z1;

   sx = max * 2;

   LOG ("%s", "\n---------------------------------------------\n");
   LOG ("%s", "Bounding Box....\n");
   LOG ("(%f,%f,%f) - (%f, %f, %f)\n", bbx1, bby1, bbz1, bbx2, bby2, bbz2);
   LOG ("Correcting values: (%f,%f,%f)\n",x1, y1, z1);
   LOG ("Scaling %f: (%f,%f,%f)\n",sx , x1 / sx, y1 /sx, z1/sx);
   LOG ("%s", "\n---------------------------------------------\n");

   /* Center vertexs  and normalize*/

   for (i = 0; i < obj->n_meshes; i++)
     {
       for (j = 0; j < obj->mesh[i].n_vertex; j++)
	 {	  

	   obj->mesh[i].vertex[j].x /= sx;
	   obj->mesh[i].vertex[j].y /= sx;
	   obj->mesh[i].vertex[j].z /= sx;

	 }
     }

   return 0;
 }

int
loader_3ds_free (OBJ3D *obj)
{
  int   i;


  if (!obj) return -1;
#if 0
  /* Delete materials*/
  for (i = 0; obj->n_mats; i++)
    {
      if (obj->mat_list[i].name) free (obj->mat_list[i].name);
    }
#endif
  for (i = 0; i < obj->n_meshes; i++)
    {
      if (obj->mesh[i].vertex) free (obj->mesh[i].vertex);
      if (obj->mesh[i].normal) free (obj->mesh[i].normal);
      if (obj->mesh[i].poly) free (obj->mesh[i].poly);
      if (obj->mesh[i].map) free (obj->mesh[i].map);
    }
  return 0;
}

 int 
 loader_3ds (OBJ3D *obj, char *fname)
 {
   FILE            *f;
   unsigned short  id, id1;    /* File chunck id */
   unsigned int    len, len1;   /* File chunk lenghth*/
   int             i, flag;
   char            data[1024];

   if (!obj)   return -1;
   if (!fname) return -1;


   /* Basic object initialisation */
   obj->n_meshes = 0;
   obj->n_mats = 0;
   obj->mesh = NULL;
   obj->mat_list = NULL;
   flag = 0;

   if ((f = fopen (fname, "rb")) == NULL)
     {
       fprintf (stderr, "Cannot open file: '%s'\n", fname);
       return -1;
     }

   /* Create default material for object without material */
   if ((obj->mat_list = realloc (obj->mat_list, sizeof(MATERIAL) * 
				 (obj->n_mats + 1))) == NULL)
     {
       fprintf (stderr, "%s", "Cannot alloc memory for new material\n");
       /* XXX: Free memory */
       return -1;
     }
   obj->mat_indx = obj->n_mats;
   obj->n_mats++;

   obj->mat_list[obj->mat_indx].name = NULL;
   obj->mat_list[obj->mat_indx].texture = NULL;
   /*obj->mat_list[obj->mat_indx].id_tex = NULL;*/
   obj->mat_list[obj->mat_indx].id_tex = -1;

   if ((obj->mat_list[obj->mat_indx].name = malloc (20)) == NULL)
     {
       fprintf (stderr, "%s", "Cannot allocate memory for material name\n");
       /* XXX: Free memory */
       return -1;
     }
   strncpy (obj->mat_list[obj->mat_indx].name, "DEFAULT", 20);
   /*
   obj->mat_list[obj->mat_indx].color.r = 128;
   obj->mat_list[obj->mat_indx].color.g = 128;
   obj->mat_list[obj->mat_indx].color.b = 128;
   */
   //obj->mat_list[obj->mat_indx].id_tex = NULL;
  obj->mat_list[obj->mat_indx].color.r = 255;
  obj->mat_list[obj->mat_indx].color.g = 255;
  obj->mat_list[obj->mat_indx].color.b = 255;

  obj->mat_list[obj->mat_indx].id_tex = -1;
  

  /* Check file format */
  _get_chunck (f, &id, &len);
  if (id != 0x4d4d)
    {
      fprintf (stderr, "File '%s' is not a valid 3DS file\n", fname);
      return -1;
    }

  while (ftell (f) < file_size (fileno (f)))
    {
      /* Read chunck from file */
      _get_chunck (f, &id, &len);
      LOG ("Read chunk %x of len %d at %lx\n", id, len, ftell (f));
      /* Process chuck */
      switch (id)
	{

	  /* 0 lenght chunck to skip on */
	case 0x3d3d:
	case 0x4100:
	  LOG ("Found 0 len chunck... skiping\n");
	  break;
	  
	case 0x4000: /*  Object Block */
	  {
	    //if (flag) _skip_current_chunk (f, len);
	    /* 3DS object name is 20 characters long */
	    /* XXX: Check if len field contains string length */
	    if ((obj->name = malloc (20 + 1)) == NULL)
	      {
		fprintf (stderr, "%s", "Cannot allocate object name\n");
		return -1;
	      }
	    _get_strz (f, obj->name, 20);	    
	    flag = 1;  /* Only allow a single object for file */
	    break;
	  }


	case 0x4110: /* Vertex list */
	  {
	    LOG ("Found vertex list\n");
	    /* Vertex list found -> Adding new mesh to object */
	    if ((obj->mesh = realloc (obj->mesh, sizeof(MESH3D) * 
				      (obj->n_meshes + 1))) == NULL)
	      {
		fprintf (stderr, "%s", "Cannot allocate 3Dmesh for object");
		/* XXX: Free memory*/
		return -1;
	      }
	    obj->indx = obj->n_meshes;
	    obj->n_meshes++;
	    obj->mesh[obj->indx].vertex = NULL;
	    obj->mesh[obj->indx].normal = NULL;
	    obj->mesh[obj->indx].poly = NULL;
	    obj->mesh[obj->indx].map = NULL;
	    obj->mesh[obj->indx].n_vertex = _get_short (f);
	    /* Allocate vertex array */
	    if ((obj->mesh[obj->indx].vertex = 
		 malloc (sizeof(VECTOR3D) * 
			 (obj->mesh[obj->indx].n_vertex))) == NULL)
	      {
		fprintf (stderr, "Cannot allocate memory for %ld "
			 "points long mesh %d\n",
			 obj->mesh[obj->indx].n_vertex, obj->indx);
		/* XXX: Free memory */
		return -1;
	      }
	    LOG ("Reading %ld vertexs from file\n", 
		 obj->mesh[obj->indx].n_vertex);
	    for (i = 0; i < obj->mesh[obj->indx].n_vertex; i++)
	      _get_3Dvector (f, &obj->mesh[obj->indx].vertex[i]);

	    obj->mesh[obj->indx].mat = 0; 
	    /* Default material to be overwritten later if necessary*/

	    break;

	  }


	case 0x4120: /* Polygon list */
	  {
	    /* FIXME: Meshes are added when vertex list is found.
	       Should be some TAG for this*/
	    obj->mesh[obj->indx].n_poly = _get_short (f);
	    /* Allocate polygon array */
	    if ((obj->mesh[obj->indx].poly = 
		 malloc (sizeof(VECTOR4D_INT) * 
			 (obj->mesh[obj->indx].n_poly))) == NULL)
	      {
		fprintf (stderr, "Cannot allocate memory for %ld "
			 "polygons for mesh %d\n",
			 obj->mesh[obj->indx].n_poly, obj->indx);
		/* XXX: Free memory */
		return -1;
	      }
	    LOG ("Reading %ld polygons from file\n", 
		 obj->mesh[obj->indx].n_poly);
	    for (i = 0; i < obj->mesh[obj->indx].n_poly; i++)
	      _get_4Dvector_int (f, &obj->mesh[obj->indx].poly[i]);

	    /* XXX: Calculate Normals */
	    _calculate_current_mesh_normals (obj);

	    break;
	  }


	case 0x4140:   /* Texture u,v mapping coordinates */
	  {
	    /* FIXME: Meshes are added when vertex list is found. 
	       Should be some TAG for this*/
	    obj->mesh[obj->indx].n_map = _get_short (f);

	    /* Allocate Texture mapping coordinates */
	    if ((obj->mesh[obj->indx].map = 
		 malloc (sizeof(VECTOR2D) * 
			 (obj->mesh[obj->indx].n_map))) == NULL)
	      {
		fprintf (stderr, "Cannot allocate memory for %ld "
			 "mapping coordinates for mesh %d\n",
			 obj->mesh[obj->indx].n_poly, obj->indx);
		/* XXX: Free memory */
		return -1;
	      }
	    LOG ("Reading %ld mapping coordinates from file\n", 
		 obj->mesh[obj->indx].n_map);
	    for (i = 0; i < obj->mesh[obj->indx].n_map; i++)
	      _get_2Dvector (f, &obj->mesh[obj->indx].map[i]);
	    break;

	  }

	case 0x4130:   /* Object Material chunck */
	  {
	    memset (data, 0, 1024);
	    _get_strz (f, data, 30);
	    LOG ("-->Object Material %x: '%s'\n", id, data);
	    /* Locate material in material list and assign to current mesh */
	    obj->mesh[obj->indx].mat = _get_material (obj, data);

	    if (obj->mesh[obj->indx].mat == -1)
	      {
		fprintf (stderr, "material '%s' not found in current "
			 "material list\n", data);
		obj->mesh[obj->indx].mat = 0;
	      }
	    _skip_current_chunk (f, len - strlen (data) - 1);
	    break;
	  }
	  
	  /* Material realted chincks */

	case 0xa000:  /* Material Name */
	  {
	    /* Add new material to object */
	    if ((obj->mat_list = realloc (obj->mat_list, sizeof(MATERIAL) 
					  * (obj->n_mats + 1))) == NULL)
	      {
		fprintf (stderr, "%s", 
			 "Cannot alloc memory for new material\n");
		/* XXX: Free memory */
		return -1;
	      }
	    obj->mat_indx = obj->n_mats;
	    obj->n_mats++;
	    obj->mat_list[obj->mat_indx].name = NULL;
	    obj->mat_list[obj->mat_indx].texture = NULL;
	    //obj->mat_list[obj->mat_indx].id_tex = NULL;
	    obj->mat_list[obj->mat_indx].id_tex = -1;
	    if ((obj->mat_list[obj->mat_indx].name = malloc (20)) == NULL)
	      {
		fprintf (stderr, "%s", 
			 "Cannot allocate memory for material name\n");
		/* XXX: Free memory */
		return -1;
	      }
	    /* FIXME: is 20 max lenght??? */
	    _get_strz (f, obj->mat_list[obj->mat_indx].name, 20); 

	    LOG ("Found Material '%s'\n", obj->mat_list[obj->mat_indx].name);
	    //obj->mat_list[obj->mat_indx].id_tex = NULL;
	    obj->mat_list[obj->mat_indx].id_tex = -1;
	    break;
	  }

	case 0xa020: /* Material color */
	  {
	    _get_chunck (f, &id1, &len1);
	    _get_color (f, &obj->mat_list[obj->mat_indx]);
	    //obj->mat_list[obj->mat_indx].id_tex = NULL; /* If color no texture */
	    obj->mat_list[obj->mat_indx].id_tex = -1; /* If color no texture */
	    break;
	  }
	case 0xa300:
	  {
	    /* Store texture information */
	    if ((obj->mat_list[obj->mat_indx].texture = 
		 malloc (len - 6)) == NULL)
	      {
		fprintf (stderr, "Cannot allocate memory for texture "
			 "name on material %d\n", 
			 obj->mat_indx);
		/* XXX: free memory */
		return -1;
	      }
	    memset (data, 0, 1024);
	    _get_strz (f, obj->mat_list[obj->mat_indx].texture, len -6);
	    LOG ("Found texture '%s' for material %d\n", 
		    obj->mat_list[obj->mat_indx].texture, obj->mat_indx);

	    /* Generate texture */
	    /* XXX: Need to update pd_3ds to transform tex coordinates */
	    //obj->mat_list[obj->mat_indx].id_tex = pf_tmng_add_from_file (obj->mat_list[obj->mat_indx].texture);
#if 0	    
	    obj->mat_list[obj->mat_indx].id_tex = SDLGL_new_texture ();
	    res = SDLGL_read_from_file (obj->mat_list[obj->mat_indx].id_tex,
				  obj->mat_list[obj->mat_indx].texture);
	    if (res < 0) /* Cannot load texture */
#endif
	      {
		//SDLGL_free_texture (obj->mat_list[obj->mat_indx].id_tex);
		/* Create default material */
		//obj->mat_list[obj->mat_indx].id_tex = NULL;
		obj->mat_list[obj->mat_indx].id_tex = -1;
		obj->mat_list[obj->mat_indx].color.r = 0.5;
		obj->mat_list[obj->mat_indx].color.g = 0.5;
		obj->mat_list[obj->mat_indx].color.b = 0.5;
	      }
	    //obj->mat_list[obj->mat_indx].id_tex = LoadBitmap (obj->mat_list[obj->mat_indx].texture);
	    /* Skip rest of chunk */
	    //_skip_current_chunk (f, len - strlen (data) - 1);
	    break;
	  }

	case 0xa200:
 	case 0xafff:
	  //fread (data, sizeof(char) * l_chunk_lenght - 6, 1, l_file);
	  //LOG ("-->Chunk %x: '%s'\n", l_chunk_id, data);

	  break;

	  /* Otherwise skip chunk */
	default:  
	  _skip_current_chunk (f, len);
  
	}
    }
  _dump_obj3d (obj);
  LOG ("3DS model load DONE\n");
  fclose (f);


  return 0;
}

