#ifndef PIX_H
#define PIX_H

#include "dotypes.h"
#include "err.h"

typedef struct
{
  byte r;
  byte g;
  byte b;
  byte a;
} Pix;

typedef struct
{
  uint32 width;
  uint32 height;
  Pix *data;
} PixImg;

Pix *pixAt(PixImg *img, uint32 col, uint32 row);
Pix *set(Pix *pix, byte r, byte g, byte b, byte a);
ERR_EXISTS initImg(PixImg *img, uint32 width, uint32 height, uint32 color, PixErr *err);

#endif

