#ifndef _PIX_H_
#define _PIX_H_

#include "dotypes.h"

typedef struct
{
  int e;
  char *s;
} PixErr;

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

#endif

