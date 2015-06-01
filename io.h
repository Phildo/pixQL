#ifndef _IO_H_
#define _IO_H_

#include "dotypes.h"
#include "pix.h"
#include "bitmap.h"

void dataToPix(byte *data, int bpp, int roww, PixImg *img, PixErr *err);
void pixToData(PixImg *img, int bpp, int roww, byte *data, PixErr *err);
void readBitmap(const char *infile, Bitmap *b, PixErr *err);
void readFile(const char *infile, PixImg *img, PixErr *err);
void writeBitmap(const char *outfile, PixImg *img, PixErr *err);;
void writeFile(const char *outfile, PixImg *img, PixErr *err);;

#endif

