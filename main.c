#include <stdlib.h> //malloc
#include <stdio.h>  //printf, fread, fwrite
#include <errno.h>  //errno

#include "dotypes.h"
#include "str.h"

#include "bitmap.h"
#include "token.h"
#include "query.h"

#define ERROR(n,err, ...) ({ fprintf(stderr,err, ##__VA_ARGS__); printf("\n"); exit(n); })

const char *usage = "Usage: pixql -i input_file -o output_file \"query\"";
const char *invalid = "Invalid file- file does not conform to bitmap spec";

typedef struct Pix
{
  byte r;
  byte g;
  byte b;
  byte a;
} Pix;


void dataToPix(byte *data, int bpp, int roww, int w, int h, Pix *pix)
{
  switch(bpp)
  {
    case 32:
      for(int i = 0; i < h; i++)
      {
        for(int j = 0; j < w; j++)
        {
          pix[(i*w)+j].r = data[(i*roww)+j+0];
          pix[(i*w)+j].g = data[(i*roww)+j+1];
          pix[(i*w)+j].b = data[(i*roww)+j+2];
          pix[(i*w)+j].a = data[(i*roww)+j+3];
        }
      }
    break;
    default:
      ERROR(1,"Invaid BPP");
  }
}

void pixToData(Pix *pix, int bpp, int roww, int w, int h, byte *data)
{
  switch(bpp)
  {
    case 32:
      for(int i = 0; i < h; i++)
      {
        for(int j = 0; j < w; j++)
        {
          data[(i*roww)+j+0] = pix[(i*w)+j].r;
          data[(i*roww)+j+1] = pix[(i*w)+j].g;
          data[(i*roww)+j+2] = pix[(i*w)+j].b;
          data[(i*roww)+j+3] = pix[(i*w)+j].a;
        }
      }
    break;
    default:
      ERROR(1,"Invaid BPP");
  }
}

int main(int argc, char **argv)
{
  initTokens();

  //Read args
  char *infile_str = 0;
  char *outfile_str = 0;
  char *query_str = 0;
  for(int i = 0; i < argc; i++)
  {
         if(cmp(argv[i],"-i") == 0) infile_str  = argv[++i];
    else if(cmp(argv[i],"-o") == 0) outfile_str = argv[++i];
    else                            query_str   = argv[i];
  }

  if(!infile_str)  ERROR(1,"%s\nNo input file specified.", usage);
  if(!outfile_str) ERROR(1,"%s\nNo output file specified.",usage);
  if(!query_str)   ERROR(1,"%s\nNo query specified.",      usage);

  Query query;
  QueryError err;
  Bitmap b;
  BitmapFileHeader *bh = &b.bitmap_file_header;
  DIBHeader *dh = &b.dib_header;
  BITMAPINFOHEADER *ih;

  Pix *IN_DATA;
  Pix *OUT_DATA;

  int l = parseQuery(query_str, &query, &err);
  if(l == -1) ERROR(1,"There was an error");

  //INPUT
  FILE *in;
  if(!(in = fopen(infile_str,  "r"))) ERROR(1,"Can't open input file- %s",infile_str);

  //bitmap_header
  #define READFIELD(field,fp) if(sizeof(field) != fread(&field, sizeof(byte), sizeof(field), fp)) ERROR(1,"%s",invalid);

  //read header_field
  READFIELD(bh->header_field,in);
  READFIELD(bh->size,in);
  READFIELD(bh->reserved_a,in);
  READFIELD(bh->reserved_b,in);
  READFIELD(bh->offset,in);

  //read dib_header
  READFIELD(dh->header_size,in);
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
      ERROR(1,"Unsupported DIB header\n");
  }
  READFIELD(ih->width,in);
  READFIELD(ih->height,in);
  READFIELD(ih->nplanes,in);
  READFIELD(ih->bpp,in);

  fseek(in, bh->offset, SEEK_SET);
  int roww = ((ih->bpp*ih->width+31)/32)*4;
  int pixel_bytes = roww*ih->height;
  b.pixel_array = malloc(pixel_bytes);
  if(pixel_bytes != fread(b.pixel_array, sizeof(byte), pixel_bytes, in)) ERROR(1,"%s",invalid);

  IN_DATA  = malloc(ih->width*ih->height*sizeof(Pix));
  OUT_DATA = malloc(ih->width*ih->height*sizeof(Pix));
  dataToPix(b.pixel_array, ih->bpp, roww, ih->width, ih->height, IN_DATA);

  //COPY
  for(int i = 0; i < ih->height; i++)
    for(int j = 0; j < ih->width; j++)
      OUT_DATA[(i*ih->width)+j] = IN_DATA[(i*ih->width)+j];

  //MODIFY
  for(int i = 0; i < ih->height; i++)
    OUT_DATA[(i*ih->width)+(int)(((float)i/(float)ih->height)*ih->width)].r = 0xFF;

  //OUTPUT
  FILE *out;
  if(!(out = fopen(outfile_str, "w"))) ERROR(1,"Can't open output file- %s",outfile_str);

  byte *indata = malloc(bh->size);
  fseek(in, 0, SEEK_SET);
  fread( indata, sizeof(byte), bh->size, in);
  fwrite(indata, sizeof(byte), bh->size, out);
  fseek(out, bh->offset, SEEK_SET);

  pixToData(OUT_DATA, ih->bpp, roww, ih->width, ih->height, b.pixel_array);

  fwrite(b.pixel_array, sizeof(byte), pixel_bytes, out);

  fclose(in);
  fclose(out);

  return 0;
}

