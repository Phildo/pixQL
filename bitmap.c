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

ERR_EXISTS writeBitmap(const char *outfile, Bitmap *b, PixErr *err)
{
  FILE *out;
  FILE *fp;
  if(!(out = fopen(outfile, "w"))) ERROR("Can't open output file- %s",outfile);
  fp = out;

  #define WRITEFIELD(field) ({ if(sizeof(field) != fwrite(&field, sizeof(byte), sizeof(field), fp)) { fclose(out); return ERR; }  })

  BitmapFileHeader *h = &b->bitmap_file_header;
  WRITEFIELD(h->header_field);
  WRITEFIELD(h->size);
  WRITEFIELD(h->reserved_a);
  WRITEFIELD(h->reserved_b);
  WRITEFIELD(h->offset);

  DIBHeader *dh = &b->dib_header;
  BITMAPV5HEADER *v5h = &dh->bitmap_v5_header;
  WRITEFIELD(v5h->bV5Size);
  WRITEFIELD(v5h->bV5Width);
  WRITEFIELD(v5h->bV5Height);
  WRITEFIELD(v5h->bV5Planes);
  WRITEFIELD(v5h->bV5BitCount);
  WRITEFIELD(v5h->bV5Compression);
  WRITEFIELD(v5h->bV5SizeImage);
  WRITEFIELD(v5h->bV5XPelsPerMeter);
  WRITEFIELD(v5h->bV5YPelsPerMeter);
  WRITEFIELD(v5h->bV5ClrUsed);
  WRITEFIELD(v5h->bV5ClrImportant);
  WRITEFIELD(v5h->bV5RedMask);
  WRITEFIELD(v5h->bV5GreenMask);
  WRITEFIELD(v5h->bV5BlueMask);
  WRITEFIELD(v5h->bV5AlphaMask);
  WRITEFIELD(v5h->bV5CSType);
  WRITEFIELD(v5h->RciexyzX);
  WRITEFIELD(v5h->RciexyzY);
  WRITEFIELD(v5h->RciexyzZ);
  WRITEFIELD(v5h->GciexyzX);
  WRITEFIELD(v5h->GciexyzY);
  WRITEFIELD(v5h->GciexyzZ);
  WRITEFIELD(v5h->BciexyzX);
  WRITEFIELD(v5h->BciexyzY);
  WRITEFIELD(v5h->BciexyzZ);
  WRITEFIELD(v5h->bV5GammaRed);
  WRITEFIELD(v5h->bV5GammaGreen);
  WRITEFIELD(v5h->bV5GammaBlue);
  WRITEFIELD(v5h->bV5Intent);
  WRITEFIELD(v5h->bV5ProfileData);
  WRITEFIELD(v5h->bV5ProfileSize);
  WRITEFIELD(v5h->bV5Reserved);

  fwrite(b->pixel_array, sizeof(byte), b->simple.width*b->simple.height*4, out);

  fclose(out);
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

static ERR_EXISTS pixToData(PixImg *img, Bitmap *b, PixErr *err)
{
  byte *data = b->pixel_array;
  int roww = b->simple.row_w;

  switch(b->simple.bpp)
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
      ERROR("Malformed internal bitmap");
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
  int headersize = 2+4+2+2+4;
  int dibsize = BITMAPV5HEADER_SIZE;
  int datasize = img->width*img->height*4;;

  BitmapFileHeader *h = &b->bitmap_file_header;
  h->header_field[0] = 'B';
  h->header_field[1] = 'M';
  h->size = headersize+dibsize+datasize;
  h->offset = headersize+dibsize;

  DIBHeader *dh = &b->dib_header;
  BITMAPV5HEADER *v5h = &dh->bitmap_v5_header;
  v5h->bV5Size = BITMAPV5HEADER_SIZE;
  v5h->bV5Width = img->width;
  v5h->bV5Height = img->height;
  v5h->bV5Planes = 0;
  v5h->bV5BitCount = 32;
  v5h->bV5Compression = 0;
  v5h->bV5SizeImage = 0;
  v5h->bV5XPelsPerMeter = 360;
  v5h->bV5YPelsPerMeter = 360;
  v5h->bV5ClrUsed = 0;
  v5h->bV5ClrImportant = 0;
  v5h->bV5RedMask   = 0xff000000;
  v5h->bV5GreenMask = 0x00ff0000;
  v5h->bV5BlueMask  = 0x0000ff00;
  v5h->bV5AlphaMask = 0x00000000;

  b->pixel_array = calloc(datasize,1);
  if(!pixToData(img, b, err)) return ERR;

  return NO_ERR;
}

