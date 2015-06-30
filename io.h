#ifndef IO_H
#define IO_H

#include "dotypes.h"
#include "err.h"
#include "pix.h"
#include "bitmap.h"

ERR_EXISTS readFile(const char *infile, PixImg *img, PixErr *err);
ERR_EXISTS writeFile(const char *outfile, PixImg *img, PixErr *err);;

#endif

