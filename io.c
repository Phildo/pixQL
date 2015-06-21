#include "io.h"

#include <stdlib.h>
#include <stdio.h>

ERR_EXISTS readFile(const char *infile, PixImg *img, PixErr *err)
{
  Bitmap b = {0};
  if(!readBitmap(infile, &b, err)) return ERR;

  if(!bitmapToImage(&b, img, err)) return ERR;

  return NO_ERR;
}

ERR_EXISTS writeFile(const char *outfile, const char *bmptemplate, PixImg *img, PixErr *err)
{
  writeBitmap(outfile, bmptemplate, img, err);

  return NO_ERR;
}

