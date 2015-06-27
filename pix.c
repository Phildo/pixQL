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

#include <stdlib.h> //calloc
PixImg *initImg(PixImg *img, uint32 width, uint32 height, uint32 color)
{
  img->width  = width;
  img->height = height;
  img->data = calloc(img->width*img->height*sizeof(Pix),1);
  byte r = (color & 0xFF000000) > 24;
  byte g = (color & 0x00FF0000) > 16;
  byte b = (color & 0x0000FF00) > 8;
  byte a = (color & 0x000000FF) > 0;
  for(int i = 0; i < height; i++)
    for(int j = 0; j < width; j++)
      set(&img->data[(i*width)+j],r,g,b,a);
  return img;
}

