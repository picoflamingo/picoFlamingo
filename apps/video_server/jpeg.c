#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jpeglib.h>

int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

/******************************************************************************/


typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */
  int size;
  JOCTET * buffer;		/* start of buffer */
} buf_destination_mgr;

typedef buf_destination_mgr * buf_dest_ptr;


METHODDEF(void)
init_destination (j_compress_ptr cinfo)
{
  buf_dest_ptr dest = (buf_dest_ptr) cinfo->dest;

  dest->size = 65536*256;
  /* Allocate the output buffer --- it will be released when done with image */
  dest->buffer = (JOCTET *) malloc(dest->size * sizeof(JOCTET));
  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = dest->size;
}

METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
  buf_dest_ptr dest = (buf_dest_ptr) cinfo->dest;
  printf("TOO BIG!\n");
	exit (0);
  return TRUE;
}

METHODDEF(void)
term_destination (j_compress_ptr cinfo)
{
  buf_dest_ptr dest = (buf_dest_ptr) cinfo->dest;
  size_t datacount = dest->size - dest->pub.free_in_buffer;
  dest->size=datacount;
}


/***********************************************************************************/

int write_jpeg_mem( char **dst, char *buffer, int w, int h, int bpp )
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  buf_dest_ptr dest;
  /* this is a pointer to one row of image data */
  JSAMPROW row_pointer[1];

  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_compress(&cinfo);

  if (cinfo.dest == NULL) {	// first time for this JPEG object? 
    cinfo.dest = (struct jpeg_destination_mgr *)
      (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
				 sizeof(buf_destination_mgr));
  }

  dest = (buf_dest_ptr) cinfo.dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;

  
  /* Setting the parameters of the output file here */
#if 0
  cinfo.image_width = cam_width;
  cinfo.image_height = cam_height;
  cinfo.input_components = bytes_per_pixel;
#endif
  cinfo.image_width = w;
  cinfo.image_height = h;
  cinfo.input_components = bpp;
  cinfo.in_color_space = color_space;

  /* default compression parameters, we shouldn't be worried about these */
  jpeg_set_defaults( &cinfo );
  /* Now do the compression .. */
  jpeg_start_compress( &cinfo, TRUE );
  /* like reading a file, this time write one row at a time */
  while( cinfo.next_scanline < cinfo.image_height )
    {
      row_pointer[0] = &buffer[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
      jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
  /* similar to read file, clean up after we're done compressing */
  jpeg_finish_compress( &cinfo );

  *dst = malloc (dest->size);
  memcpy (*dst, dest->buffer, dest->size);


  jpeg_destroy_compress( &cinfo );

  free (dest->buffer);

  /* success code is 1! */
  return dest->size;
}



int write_jpeg_file( char *filename, char *buffer, int w, int h, int bpp )
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  
  /* this is a pointer to one row of image data */
  JSAMPROW row_pointer[1];
  FILE *outfile = fopen( filename, "wb" );
  
  if ( !outfile )
    {
      printf("Error opening output jpeg file %s\n!", filename );
      return -1;
    }
  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, outfile);
  
  /* Setting the parameters of the output file here */
#if 0
  cinfo.image_width = cam_width;
  cinfo.image_height = cam_height;
  cinfo.input_components = bytes_per_pixel;
#endif
  cinfo.image_width = w;
  cinfo.image_height = h;
  cinfo.input_components = bpp;
  cinfo.in_color_space = color_space;
  /* default compression parameters, we shouldn't be worried about these */
  jpeg_set_defaults( &cinfo );
  /* Now do the compression .. */
  jpeg_start_compress( &cinfo, TRUE );
  /* like reading a file, this time write one row at a time */
  while( cinfo.next_scanline < cinfo.image_height )
    {
      row_pointer[0] = &buffer[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
      jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
  /* similar to read file, clean up after we're done compressing */
  jpeg_finish_compress( &cinfo );
  jpeg_destroy_compress( &cinfo );
  fclose( outfile );
  /* success code is 1! */
  return 1;
}




