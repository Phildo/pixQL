#include "io.h"

#include <stdlib.h>
#include <stdio.h>

static void dataToPix(byte *data, int bpp, int roww, PixImg *img, PixErr *err);
static void pixToData(PixImg *img, int bpp, int roww, byte *data, PixErr *err);

static void dataToPix(byte *data, int bpp, int roww, PixImg *img, PixErr *err)
{
  switch(bpp)
  {
    case 32:
      for(int i = 0; i < img->height; i++)
      {
        for(int j = 0; j < img->width; j++)
        {
          img->data[(i*img->width)+j].a = data[(i*roww)+(j*4)+0];
          img->data[(i*img->width)+j].b = data[(i*roww)+(j*4)+1];
          img->data[(i*img->width)+j].g = data[(i*roww)+(j*4)+2];
          img->data[(i*img->width)+j].r = data[(i*roww)+(j*4)+3];
        }
      }
    break;
    default:
      //ERROR(1,"Invaid BPP");
      break;
  }
}

static void pixToData(PixImg *img, int bpp, int roww, byte *data, PixErr *err)
{
  switch(bpp)
  {
    case 32:
      for(int i = 0; i < img->height; i++)
      {
        for(int j = 0; j < img->width; j++)
        {
          data[(i*roww)+(j*4)+0] = img->data[(i*img->width)+j].a;
          data[(i*roww)+(j*4)+1] = img->data[(i*img->width)+j].b;
          data[(i*roww)+(j*4)+2] = img->data[(i*img->width)+j].g;
          data[(i*roww)+(j*4)+3] = img->data[(i*img->width)+j].r;
        }
      }
    break;
    default:
      //ERROR(1,"Invaid BPP");
      break;
  }
}

ERR_EXISTS readFile(const char *infile, PixImg *img, PixErr *err)
{
  Bitmap b = {0};
  readBitmap(infile, &b, err);

  img->width  = b.simple.width;
  img->height = b.simple.height;

  img->data = calloc(img->width*img->height*sizeof(Pix),1);

  dataToPix(b.pixel_array, b.simple.bpp, b.simple.row_w, img, err);

  return NO_ERR;
}

ERR_EXISTS writeFile(const char *outfile, const char *bmptemplate, PixImg *img, PixErr *err)
{
  writeBitmap(outfile, bmptemplate, img, err);

  return NO_ERR;
}

