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

ERR_EXISTS writeFile(const char *outfile, PixImg *img, PixErr *err)
{
  Bitmap b = {0};
  if(!imageToBitmap(img, &b, err)) return ERR;
  if(!writeBitmap(outfile, &b, err)) return ERR;

  return NO_ERR;
}

