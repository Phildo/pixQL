#include "io.h"

#include <stdlib.h>
#include <stdio.h>

void dataToPix(byte *data, int bpp, int roww, PixImg *img, PixErr *err)
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

void pixToData(PixImg *img, int bpp, int roww, byte *data, PixErr *err)
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

void readBitmap(const char *infile, Bitmap *b, PixErr *err)
{
  //INPUT
  FILE *in;
  if(!(in = fopen(infile, "r"))) return;//ERROR(1,"Can't open input file- %s",infile);

  BitmapFileHeader *bh = &b->bitmap_file_header;
  DIBHeader *dh = &b->dib_header;
  BITMAPINFOHEADER *ih;

  //bitmap_header
  #define READFIELD(field,fp) if(sizeof(field) != fread(&field, sizeof(byte), sizeof(field), fp)) return;//ERROR(1,"%s",invalid);

  //read header_field
  READFIELD(bh->header_field,in);
  READFIELD(bh->size,in);
  READFIELD(bh->reserved_a,in);
  READFIELD(bh->reserved_b,in);
  READFIELD(bh->offset,in);

  //read dib_header
  READFIELD(dh->header_size,in);
  ih = 0;
  switch(dh->header_size)
  {
    case BITMAPINFOHEADER_SIZE:
      ih = &dh->bitmap_info_header;
    case BITMAPV5HEADER_SIZE:
      ih = (BITMAPINFOHEADER *)&dh->bitmap_v5_header;
      break;
    case BITMAPCOREHEADER_SIZE:
    case OS22XBITMAPHEADER_SIZE:
    case BITMAPV2INFOHEADER_SIZE:
    case BITMAPV3INFOHEADER_SIZE:
    case BITMAPV4HEADER_SIZE:
    default:
      //ERROR(1,"Unsupported DIB header\n");
      break;
  }
  READFIELD(ih->width,in);
  READFIELD(ih->height,in);
  READFIELD(ih->nplanes,in);
  READFIELD(ih->bpp,in);

  fseek(in, bh->offset, SEEK_SET);
  b->extra_info.row_w = ((ih->bpp*ih->width+31)/32)*4;
  b->extra_info.pixel_n_bytes = b->extra_info.row_w*ih->height;
  b->pixel_array = malloc(b->extra_info.pixel_n_bytes);
  if(b->extra_info.pixel_n_bytes != fread(b->pixel_array, sizeof(byte), b->extra_info.pixel_n_bytes, in)) return;//ERROR(1,"%s",invalid);

  fclose(in);
}

ERR_EXISTS readFile(const char *infile, PixImg *img, PixErr *err)
{
  Bitmap b = {0};
  readBitmap(infile, &b, err);

  img->width  = b.dib_header.bitmap_info_header.width;
  img->height = b.dib_header.bitmap_info_header.height;

  img->data = malloc(img->width*img->height*sizeof(Pix));

  dataToPix(b.pixel_array, b.dib_header.bitmap_info_header.bpp, b.extra_info.row_w, img, err);

  return NO_ERR;
}

void writeBitmap(const char *outfile, const char *bmptemplate, PixImg *img, PixErr *err)
{
  FILE *out;
  if(!(out = fopen(outfile, "w"))) return;//ERROR(1,"Can't open output file- %s",outfile);
  FILE *in; //THIS IS DUMB
  if(!(in = fopen(bmptemplate, "r"))) return;//ERROR(1,"Can't open input file- %s",bmptemplate);







  Bitmap b = {0};

  BitmapFileHeader *bh = &b.bitmap_file_header;
  DIBHeader *dh = &b.dib_header;
  BITMAPINFOHEADER *ih;

  //bitmap_header
  #define READFIELD(field,fp) if(sizeof(field) != fread(&field, sizeof(byte), sizeof(field), fp)) return;//ERROR(1,"%s",invalid);

  //read header_field
  READFIELD(bh->header_field,in);
  READFIELD(bh->size,in);
  READFIELD(bh->reserved_a,in);
  READFIELD(bh->reserved_b,in);
  READFIELD(bh->offset,in);

  //read dib_header
  READFIELD(dh->header_size,in);
  ih = 0;
  switch(dh->header_size)
  {
    case BITMAPINFOHEADER_SIZE:
      ih = &dh->bitmap_info_header;
    case BITMAPV5HEADER_SIZE:
      ih = (BITMAPINFOHEADER *)&dh->bitmap_v5_header;
      break;
    case BITMAPCOREHEADER_SIZE:
    case OS22XBITMAPHEADER_SIZE:
    case BITMAPV2INFOHEADER_SIZE:
    case BITMAPV3INFOHEADER_SIZE:
    case BITMAPV4HEADER_SIZE:
    default:
      //ERROR(1,"Unsupported DIB header\n");
      break;
  }
  READFIELD(ih->width,in);
  READFIELD(ih->height,in);
  READFIELD(ih->nplanes,in);
  READFIELD(ih->bpp,in);

  fseek(in, bh->offset, SEEK_SET);
  b.extra_info.row_w = ((ih->bpp*ih->width+31)/32)*4;
  b.extra_info.pixel_n_bytes = b.extra_info.row_w*ih->height;
  b.pixel_array = malloc(b.extra_info.pixel_n_bytes);
  if(b.extra_info.pixel_n_bytes != fread(b.pixel_array, sizeof(byte), b.extra_info.pixel_n_bytes, in)) return;//ERROR(1,"%s",invalid);






  byte *indata = malloc(b.bitmap_file_header.size);
  fseek(in, 0, SEEK_SET);
  fread( indata, sizeof(byte), b.bitmap_file_header.size, in);
  fwrite(indata, sizeof(byte), b.bitmap_file_header.size, out);
  fseek(out, b.bitmap_file_header.offset, SEEK_SET);

  pixToData(img, b.dib_header.bitmap_info_header.bpp, b.extra_info.row_w, b.pixel_array, err);

  fwrite(b.pixel_array, sizeof(byte), b.extra_info.pixel_n_bytes, out);

  fclose(in);
  fclose(out);
}

ERR_EXISTS writeFile(const char *outfile, const char *bmptemplate, PixImg *img, PixErr *err)
{
  writeBitmap(outfile, bmptemplate, img, err);

  return NO_ERR;
}

