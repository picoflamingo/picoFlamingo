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

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "log.h"
#include "const.h"

#include "pf_font.h"

typedef struct font_item_t
{
  char     *name;
  PF_FONT  *font;
} FONT_ITEM;

#define MAX_FONTS 255

static FONT_ITEM _font_list[MAX_FONTS];
static int       _n_fonts = 0;

int build_font(char* filename, PF_FONT* font);
int build_character (FT_Face face, int c, PF_FONT *font);

static int
_pf_add_font (char *name, char *file, int size, int style)
{
  PF_FONT *afont;

  // Create font object
  afont = malloc (sizeof(PF_FONT));
  afont->style = style;
  afont->pointsize = size;
  pf_font_new (file, afont);

  // Register font
  _font_list[_n_fonts].font = afont;
  _font_list[_n_fonts].name = strdup (name);
  _n_fonts++;
  LOG ("Font '%s' registered. %d available fonts\n", name, _n_fonts);

  return 0;  
}

PF_FONT*
pf_get_font (char *name)
{
  int  i;
  
  for (i = 0; i < _n_fonts; i++)
    {
      if (!strcmp (name, _font_list[i].name))
	return _font_list[i].font;
    }

  // Return default font  
  return _font_list[0].font;
}


int
pf_font_mng_init ()
{
  FILE *f;
  int  size, l, line;
  char buffer[1024];
  char *delim = "\t ";
  char *token, *name, *fname;


  _pf_add_font ("default", "FreeSans.ttf", 32, PF_FONT_STYLE_NORMAL);
  if ((f = fopen ("font_list.txt", "rt")) == NULL)
    {
      fprintf (stderr, "Cannot open local font_list.txt\n");
      if ((f = fopen (DATA_DIR "/font_list.txt", "rt")) == NULL)
	{
	  fprintf (stderr, "Cannot open system font_list.txt\n");
	  return -1;
	}
    }
  line = 0;
  while  (!feof (f))
    {
      line++;
      fgets (buffer, 1024, f);

      //LOG ("Processing line '%s'\n", buffer);
      l = strlen (buffer);
      if (buffer[0] == ';') continue;
      buffer[l - 1] = 0; // CHOMP
      if (l == 0) break;
      token = strtok (buffer, delim);
      if (token == NULL)
	{
	  fprintf (stderr, "Parse error in line %d...\n", line);
	  continue;
	}
      size = atoi (token);
      name = strtok (NULL, delim);
      fname = strtok (NULL, delim);
      if (!name || !fname) continue;
      LOG ("Registering '%s' with size %d from file '%s'\n", 
	      name, size, fname);


      _pf_add_font (name, fname, size, PF_FONT_STYLE_NORMAL);
    }

  fclose (f);

  LOG ("Done!\n");

  return 0;
}

int
pf_font_new (char *name, PF_FONT *font)
{

  if ( build_font(name, font) < 0)
    {
      LOG ("ERROR building font: %s\n", name);
      return -1;		
    }
  return 0;
}


/* Helper functions */
int 
build_font (char* filename1, PF_FONT* font)
{
  FT_Library  library;
  FT_Face     face;
  int     i;
  char        filename[1024];


  snprintf (filename, 1024, "fonts/%s", filename1);
  if (FT_Init_FreeType (&library))
    {
      fprintf (stderr, "%s", "Cannot init FT_Library\n");
      return -1;
    }

  if (FT_New_Face (library, filename, 0, &face))
    {
      snprintf (filename, 1024, "%s/fonts/%s", DATA_DIR, filename1);
      if (FT_New_Face (library, filename, 0, &face))
	{
	  fprintf (stderr, "%s", "Cannot init FT_Face\n");
	  return -1;
	}
    }

  FT_Set_Char_Size (face, font->pointsize << 6, font->pointsize << 6, 96, 96);

  /* Create Character Textures */
  for (i = 0; i < 128; i++)
    build_character (face, (unsigned char)i, font);

  FT_Done_Face (face);
  FT_Done_FreeType (library);

  return 0;

}



/* Quick utility function for texture creation */
static int 
next_p2(int input)
{
  int value = 1;
  
  while ( value < input ) {
    value <<= 1;
  }
  return value;
}



/* Helper functions */
int
build_character (FT_Face face, int c, PF_FONT *font)
{
  FT_Glyph         glyph;
  FT_BitmapGlyph   bitmap_glyph;
  FT_Bitmap        bitmap;
  int              width, height, i, j;
  float            x, y;
  GLubyte          *expanded_data;
  unsigned char    ch = (unsigned char)c;

  /* Load the Glyph for our character. */
  if (FT_Load_Glyph (face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT ))
    {
      fprintf (stderr, "ERROR: %s", "Load_GLyph\n");
      return -1;
    }

  /* Move the face's glyph into a Glyph object. */
  if (FT_Get_Glyph( face->glyph, &glyph ))
    {
      fprintf (stderr, "ERROR: %s", "FT_Get_Glyph\n");
      return -1;
    }
   
  /* Convert the glyph to a bitmap. */
  FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
  bitmap_glyph = (FT_BitmapGlyph)glyph;

  bitmap = bitmap_glyph->bitmap;

  /* Use our helper function to get the widths of
   * the bitmap data that we will need in order to create
   * our texture.
   */
  width = next_p2( bitmap.width );
  height = next_p2( bitmap.rows );


  /* Allocate memory for the texture data.*/
  if ((expanded_data = (GLubyte *) malloc ( 4 * width * (height + 1))) == NULL)
    {
      fprintf (stderr, "ERROR: %s", "Cannot allocalte memory for "
	       "character texture\n");
      /* FIXME: Cleanup memory */
      return -1;
    }

  /* Generate texture from bitmap info */
  for(j = 0; j < height; j++) 
    {
      for(i = 0; i < width; i++)
	{

	  expanded_data[4 * (i + j * width) + 0]= 
	    (i >= bitmap.width || j >= bitmap.rows) ? //0 : 255;
	     0 :  bitmap.buffer[i + bitmap.width * j];

	    expanded_data[4 * (i + j * width) + 1] = 
	      (i >= bitmap.width || j >= bitmap.rows) ? //0: 255;
	      0 : bitmap.buffer[i + bitmap.width * j];

	    expanded_data[4 * (i + j * width) + 2] = 
	      (i >= bitmap.width || j >= bitmap.rows) ? //0: 255;
	      0 : bitmap.buffer[i + bitmap.width * j];

	    expanded_data[4 * (i + j * width) + 3] = 
	      (i >= bitmap.width || j >= bitmap.rows) ? //0: 1;
	      0 : bitmap.buffer[i + bitmap.width * j];
	}
    }

  // Generate texture
  glGenTextures (1, (GLuint *)&font->glyphs[ch].texid);

  glBindTexture   (GL_TEXTURE_2D, font->glyphs[ch].texid);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, expanded_data );
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


  /* XXX: Expanded data is kept for STATIC_TEXT
   *     STATIC_TEXT produces a texture for the whole text so it needs
   *     the expanded bitmap data.
   */
  font->glyphs[ch].data = (char *)expanded_data;

  /* Generate display list */
  x = (float) bitmap.width / (float) width;
  y = (float) bitmap.rows / (float) height;

  font->glyphs[ch].texcoord[0] = 0;
  font->glyphs[ch].texcoord[1] = 0;

  font->glyphs[ch].texcoord[2] = x;
  font->glyphs[ch].texcoord[3] = y;

  font->glyphs[ch].minx = 0;
  font->glyphs[ch].miny = 0;
  font->glyphs[ch].maxx = bitmap.width;
  font->glyphs[ch].maxy = bitmap.rows;

  font->glyphs[ch].tw = width;
  font->glyphs[ch].th = height;

  font->glyphs[ch].advance = face->glyph->advance.x  >> 6;
  font->glyphs[ch].base = bitmap_glyph->top - bitmap.rows;

#if 0
  LOG ("%d (%d, %d, %d, %d) -> %d \n",
	  font->glyphs[ch].minx,
	  font->glyphs[ch].miny,
	  font->glyphs[ch].maxx,
	  font->glyphs[ch].maxy,
	  font->glyphs[ch].advance
	  );
#endif


  return 0;
}
