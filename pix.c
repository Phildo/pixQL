#include "pix.h"

Pix *pixAt(PixImg *img, uint32 col, uint32 row)
{
  return &img->data[(row*img->width)+col];
}

