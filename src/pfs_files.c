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
#include "pf_image.h"
#include "pf_fx.h"

#include "const.h"

#include "pfs_cmd.h"

extern PF_RITEM ri_list[MAX_RITEMS];
extern int      n_ri_list;
extern char     *pres_dir;
extern int      current_item;

int
clean_scene ()
{
  int  i;
  LOG ("Cleaning Effects\n");

  pf_fx_mng_clear ();
  // Force run to delete
  LOG ("Force Run delete\n");
  pf_fx_mng_run ();

  for (i = 0; i <n_ri_list; i++)
    {
      LOG ("Deleting item %d\n", i);
      pf_ritem_free (ri_list[i]);
      ri_list[i] = NULL;
    }

  n_ri_list = 0;

  return 0;
}


int
load_slide (char *fname, int indx)
{
  FILE *f;
  char name[1024];
  char buffer[1024];
  char *aux, id[1024];
  int  l, i, line;

  snprintf (name, 1024, "%s/%s%d.pfs", pres_dir, fname, indx);
  current_item = -1;
  if ((f = fopen (name, "rt")) == NULL)
    {
      fprintf (stderr, "Cannot open slide '%s'\n", name);
      return -1;
    }
  clean_scene ();
  n_ri_list = -1;
  line = 0;
  while (!feof (f))
    {
      line++;
      fgets (buffer, 1024, f);

      l = strlen (buffer);
      if (l == 0) continue;
      buffer[l - 1] = 0;
      LOG ("Processing Line: '%s'\n", buffer);
      if (buffer[0] == ';') continue;
      memset(id, 0, 1024);
      if ((aux = strchr (buffer, ' ')))
	{
	  *aux = 0;
	  strcpy (id, buffer);
	  *aux = 32;
	}
      else
	strcpy (id, buffer);
      
      i = pfs_cmd_find (id);
      LOG ("Looking for command '%s' index is %d\n", id, i);
      if (i < 0)
	{
	  fprintf (stderr, "Unknown command in line %d\n", line);
	  continue;
	}
      pfs_cmd_run (i, buffer, NULL);
    
    }

  n_ri_list++;
  fclose (f);

  return 0;
}


int
include_slide (char *fname)
{
  FILE *f;
  char name[1024];
  char buffer[1024];
  char *aux, id[1024];
  int  l, i, line;

  snprintf (name, 1024, "%s/%s", pres_dir, fname);

  if ((f = fopen (name, "rt")) == NULL)
    {
      fprintf (stderr, "Cannot open slide '%s'\n", name);
      return -1;
    }

  line = 0;
  while (!feof (f))
    {
      line++;
      fgets (buffer, 1024, f);

      l = strlen (buffer);
      if (l == 0) continue;
      buffer[l - 1] = 0;
      LOG ("Processing Line: '%s'\n", buffer);
      if (buffer[0] == ';') continue;
      memset(id, 0, 1024);
      if ((aux = strchr (buffer, ' ')))
	{
	  *aux = 0;
	  strcpy (id, buffer);
	  *aux = 32;
	}
      else
	strcpy (id, buffer);
      
      i = pfs_cmd_find (id);
      LOG ("Looking for command '%s' index is %d\n", id, i);
      if (i < 0)
	{
	  fprintf (stderr, "Unknown command in line %d\n", line);
	  continue;
	}
      pfs_cmd_run (i, buffer, NULL);
    }

  fclose (f);

  return 0;
}
