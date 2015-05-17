#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "dotypes.h"

#include "bitmap.h"

#define ERROR(n,err, ...) ({ fprintf(stderr,err, ##__VA_ARGS__); printf("\n"); exit(n); })

int cmp(char *a, char *b)
{
  while(!(*a == '\0' && *b == '\0'))
  {
         if(*a > *b) return -1;
    else if(*a < *b) return 1;
    a++; b++;
  }
  return 0;
}

typedef struct Pix
{
  byte r;
  byte g;
  byte b;
  byte a;
} Pix;

const char *usage = "Usage: pixql -i input_file -o output_file \"query\"";
const char *invalid = "Invalid file";
int main(int argc, char **argv)
{
  //Read args
  char *infile = 0;
  char *outfile = 0;
  char *query = 0;
  for(int i = 0; i < argc; i++)
  {
         if(cmp(argv[i],"-i") == 0) infile  = argv[++i];
    else if(cmp(argv[i],"-o") == 0) outfile = argv[++i];
    else                            query   = argv[i];
  }

  if(!infile)  ERROR(1,"%s\nNo input file specified.", usage);
  if(!outfile) ERROR(1,"%s\nNo output file specified.",usage);
  if(!query)   ERROR(1,"%s\nNo query specified.",      usage);

  Bitmap b;
  BitmapFileHeader *bh = &b.bitmap_file_header;
  DIBHeader *dh = &b.dib_header;
  BITMAPINFOHEADER *ih;

  FILE *in;
  if(!(in  = fopen(infile,  "r"))) ERROR(1,"Can't open input file- %s",infile);

  //bitmap_header
  #define READFIELD(field,fp) if(sizeof(field) != fread(&field, sizeof(byte), sizeof(field), fp)) ERROR(1,"%s",invalid);

  //read header_field
  READFIELD(bh->header_field,in);
  READFIELD(bh->size,in);
  READFIELD(bh->reserved_a,in);
  READFIELD(bh->reserved_b,in);
  READFIELD(bh->offset,in);

  //dib_header
  //read dib_header_size
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
  /*
  printf("width %d\n",width);
  printf("height %d\n",height);
  printf("bpp %d\n",bpp);
  */

  fseek(in, bh->offset, SEEK_SET);
  int roww = ((ih->bpp*ih->width+31)/32)*4;
  int pixel_bytes = roww*ih->height;
  b.pixel_array = malloc(pixel_bytes);
  if(pixel_bytes != fread(b.pixel_array, sizeof(byte), pixel_bytes, in)) ERROR(1,"%s",invalid);

  Pix *pixels = malloc(ih->width*ih->height*sizeof(Pix));
  switch(ih->bpp)
  {
    case 32:
      for(int i = 0; i < ih->height; i++)
      {
        for(int j = 0; j < ih->width; j++)
        {
          pixels[(i*ih->width)+j].r = b.pixel_array[(i*roww)+j+0];
          pixels[(i*ih->width)+j].g = b.pixel_array[(i*roww)+j+1];
          pixels[(i*ih->width)+j].b = b.pixel_array[(i*roww)+j+2];
          pixels[(i*ih->width)+j].a = b.pixel_array[(i*roww)+j+3];
        }
      }
    break;
    default:
      ERROR(1,"Invaid BPP");
  }

  //MODIFY
  for(int i = 0; i < ih->height; i++)
    pixels[(i*ih->width)+(int)(((float)i/(float)ih->height)*ih->width)].r = 0xFF;

  FILE *out;
  if(!(out = fopen(outfile, "w"))) ERROR(1,"Can't open output file- %s",outfile);


  int filesize = *(unsigned int *)&bh->size;
  byte *indata = malloc(filesize);

  fseek(in, 0, SEEK_SET);
  fread( indata, sizeof(byte), filesize, in);
  fwrite(indata, sizeof(byte), filesize, out);
  fseek(out, bh->offset, SEEK_SET);

  switch(ih->bpp)
  {
    case 32:
      for(int i = 0; i < ih->height; i++)
      {
        for(int j = 0; j < ih->width; j++)
        {
          b.pixel_array[(i*roww)+j+0] = pixels[(i*ih->width)+j].r;
          b.pixel_array[(i*roww)+j+1] = pixels[(i*ih->width)+j].g;
          b.pixel_array[(i*roww)+j+2] = pixels[(i*ih->width)+j].b;
          b.pixel_array[(i*roww)+j+3] = pixels[(i*ih->width)+j].a;
        }
      }
    break;
    default:
      ERROR(1,"Invaid BPP");
  }

  fwrite(b.pixel_array, sizeof(byte), pixel_bytes, out);

  fclose(in);
  fclose(out);

  return 0;
}

