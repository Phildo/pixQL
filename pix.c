#include "pix.h"

Pix *pixAt(PixImg *img, uint32 col, uint32 row)
{
  return &img->data[(row*img->width)+col];
}

Pix *set(Pix *pix, byte r, byte g, byte b, byte a)
{
  pix->r = r;
  pix->g = g;
  pix->b = b;
  pix->a = a;
  return pix;
}

