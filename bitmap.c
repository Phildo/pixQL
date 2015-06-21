#include "bitmap.h"

#include <stdlib.h>
#include <stdio.h>

#include "pix.h"
#include "str.h"

#define READFIELD(field) ({ if(sizeof(field) != fread(&field, sizeof(byte), sizeof(field), fp)) { sprintf(err->info, "Error reading field"); return ERR; } })

static ERR_EXISTS readFileHeader(FILE *fp, BitmapFileHeader *bfh, PixErr *err)
{
  READFIELD(bfh->header_field);
  if(cmp((char *)bfh->header_field,"BM")) ERROR("File not valid Bitmap"); //can cmp because bfh ought be 0'd before use
  READFIELD(bfh->size);
  if(bfh->size <= 0) ERROR("Filesize invalid");
  READFIELD(bfh->reserved_a);
  READFIELD(bfh->reserved_b);
  READFIELD(bfh->offset);
  if(bfh->size <= 0) ERROR("Data offset invalid");

  return NO_ERR;
}

static ERR_EXISTS readDIBHeader(FILE *fp, DIBHeader *dh, PixErr *err)
{
  READFIELD(dh->header_size);
  BITMAPINFOHEADER *ih = &dh->bitmap_info_header;
  BITMAPV5HEADER *v5h = &dh->bitmap_v5_header;
  switch(dh->header_size)
  {
    case BITMAPINFOHEADER_SIZE:
    case BITMAPV5HEADER_SIZE:
      break;
    case BITMAPCOREHEADER_SIZE:
    case OS22XBITMAPHEADER_SIZE:
    case BITMAPV2INFOHEADER_SIZE:
    case BITMAPV3INFOHEADER_SIZE:
    case BITMAPV4HEADER_SIZE:
    default:
      ERROR("Unsupported DIB header (header size %d)\n",dh->header_size);
      break;
  }
  READFIELD(ih->width);
  READFIELD(ih->height);
  READFIELD(ih->nplanes);
  READFIELD(ih->bpp);
  READFIELD(ih->compression);
  if(!(ih->compression == 0 || ih->compression == 3)) ERROR("Unable to process compressed bitmaps");
  READFIELD(ih->image_size);
  if(!(ih->image_size == 0 || ih->image_size >= ih->width*ih->height*(ih->bpp/8))) ERROR("Unable to process compressed bitmaps");
  READFIELD(ih->horiz_resolution);
  READFIELD(ih->vert_resolution);
  READFIELD(ih->ncolors);
  if(ih->ncolors != 0) ERROR("Unable to process indexed bitmaps");
  READFIELD(ih->nimportantcolors);

  READFIELD(v5h->bV5RedMask);
  READFIELD(v5h->bV5GreenMask);
  READFIELD(v5h->bV5BlueMask);
  READFIELD(v5h->bV5AlphaMask);

  return NO_ERR;
}

static ERR_EXISTS readColorTable(FILE *fp, byte *b, int size, PixErr *err)
{
  return NO_ERR;
}

static ERR_EXISTS readGap(FILE *fp, byte *b, int size, PixErr *err)
{
  return NO_ERR;
}

static ERR_EXISTS readPixelArray(FILE *fp, byte *b, int size, PixErr *err)
{
  if(size != fread(b, sizeof(byte), size, fp)) ERROR("Can't read specified length at offset in bitmap");
  return NO_ERR;
}

static ERR_EXISTS readColorProfile(FILE *fp, byte *b, int size, PixErr *err)
{
  return NO_ERR;
}

ERR_EXISTS readBitmap(const char *infile, Bitmap *b, PixErr *err)
{
  //INPUT
  FILE *in;
  if(!(in = fopen(infile, "r"))) ERROR("Can't open input file- %s",infile);

  InternalBitmap *simple = &b->simple;

  BitmapFileHeader *bh = &b->bitmap_file_header;
  if(!readFileHeader(in, bh, err)) { fclose(in); return ERR; }

  DIBHeader *dh = &b->dib_header;
  if(!readDIBHeader(in, dh, err)) { fclose(in); return ERR; }

  //put together useful info in reading the rest
  simple->width = dh->bitmap_info_header.width;
  simple->height = dh->bitmap_info_header.height;
  simple->bpp = dh->bitmap_info_header.bpp;
  simple->row_w = ((simple->bpp*simple->width+31)/32)*4;
  simple->pixel_n_bytes = simple->row_w*simple->height;
  simple->offset_to_data = bh->offset;
  simple->r_mask = dh->bitmap_v5_header.bV5RedMask;
  simple->g_mask = dh->bitmap_v5_header.bV5GreenMask;
  simple->b_mask = dh->bitmap_v5_header.bV5BlueMask;
  simple->a_mask = dh->bitmap_v5_header.bV5AlphaMask;

  if(dh->bitmap_info_header.ncolors > 0)
  {
    fclose(in); ERROR("Unable to process indexed bitmaps- shouldn't have gotten here");
    byte *ct = b->color_table;
    if(!readColorTable(in, ct, 0, err)) { fclose(in); return ERR; }
  }

  //does nothing...
  byte *g = b->gap1;
  if(!readGap(in, g, 0, err)) { fclose(in); return ERR; }

  fseek(in, simple->offset_to_data, SEEK_SET);
  b->pixel_array = calloc(simple->pixel_n_bytes,1);
  byte *pa = b->pixel_array;
  if(!readPixelArray(in, pa, simple->pixel_n_bytes, err)) { fclose(in); return ERR; }

  //does nothing...
  g = b->gap2;
  if(!readGap(in, g, 0, err)) { fclose(in); return ERR; }

  //does nothing...
  byte *cp = b->icc_color_profile;
  if(!readColorProfile(in, cp, 0, err)) { fclose(in); return ERR; }

  fclose(in);

  return NO_ERR;
}

ERR_EXISTS writeBitmap(const char *outfile, const char *bmptemplate, PixImg *img, PixErr *err)
{
/*
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
  b.pixel_array = calloc(b.extra_info.pixel_n_bytes,1);
  if(b.extra_info.pixel_n_bytes != fread(b.pixel_array, sizeof(byte), b.extra_info.pixel_n_bytes, in)) return;//ERROR(1,"%s",invalid);






  byte *indata = calloc(b.bitmap_file_header.size,1);
  fseek(in, 0, SEEK_SET);
  fread( indata, sizeof(byte), b.bitmap_file_header.size, in);
  fwrite(indata, sizeof(byte), b.bitmap_file_header.size, out);
  fseek(out, b.bitmap_file_header.offset, SEEK_SET);

  //pixToData(img, b.dib_header.bitmap_info_header.bpp, b.extra_info.row_w, b.pixel_array, err);

  fwrite(b.pixel_array, sizeof(byte), b.extra_info.pixel_n_bytes, out);

  fclose(in);
  fclose(out);
  return NO_ERR;
*/
return NO_ERR;
}

static ERR_EXISTS dataToPix(Bitmap *b, PixImg *img, PixErr *err)
{
  byte *data = b->pixel_array;
  int roww = b->simple.row_w;

  switch(b->simple.bpp)
  {
    case 32:
      if(!(
        b->simple.r_mask == 0xff000000 &&
        b->simple.g_mask == 0x00ff0000 &&
        b->simple.b_mask == 0x0000ff00 &&
        b->simple.a_mask == 0x00000000
        ))
        ERROR("Error parsing weird bit masks");

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
    case 24:
      if(!(
        b->simple.r_mask == 0xff000000 &&
        b->simple.g_mask == 0x00ff0000 &&
        b->simple.b_mask == 0x0000ff00 &&
        b->simple.a_mask == 0x00000000
        ))
        ERROR("Error parsing weird bit masks");

      for(int i = 0; i < img->height; i++)
      {
        for(int j = 0; j < img->width; j++)
        {
          img->data[(i*img->width)+j].a = 0;
          img->data[(i*img->width)+j].b = data[(i*roww)+(j*4)+1];
          img->data[(i*img->width)+j].g = data[(i*roww)+(j*4)+2];
          img->data[(i*img->width)+j].r = data[(i*roww)+(j*4)+3];
        }
      }
    break;
    default:
      ERROR("Error parsing unsupported bpp");
      break;
  }
  return NO_ERR;
}

static ERR_EXISTS pixToData(PixImg *img, int bpp, int roww, byte *data, PixErr *err)
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
  return NO_ERR;
}

ERR_EXISTS bitmapToImage(Bitmap *b, PixImg *img, PixErr *err)
{
  img->width  = b->simple.width;
  img->height = b->simple.height;
  img->data = calloc(img->width*img->height*sizeof(Pix),1);
  if(!dataToPix(b, img, err)) return ERR;
  return NO_ERR;
}

ERR_EXISTS imageToBitmap(PixImg *img, Bitmap *b, PixErr *err)
{
  return NO_ERR;
}

