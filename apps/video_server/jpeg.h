#ifndef JPEG_H
#define JPEG_H

int write_jpeg_mem( char **dst, char *buffer, int w, int h, int bpp );
int write_jpeg_file( char *filename, char *buffer, int w, int h, int bpp );


#endif
