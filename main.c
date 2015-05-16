#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define error(n,err, ...) ({ fprintf(stderr,err, ##__VA_ARGS__); printf("\n"); exit(n); })

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

/* spec from http://en.wikipedia.org/wiki/BMP_file_format */
typedef struct BitmapFileHeader
{
  char header_field[2];
  char size[4];
  char reserved_a[2];
  char reserved_b[2];
  char offset[4];
} BitmapFileHeader;

const int BITMAPCOREHEADER_SIZE   = 12;
const int OS22XBITMAPHEADER_SIZE  = 64;
const int BITMAPINFOHEADER_SIZE   = 40;
const int BITMAPV2INFOHEADER_SIZE = 52;
const int BITMAPV3INFOHEADER_SIZE = 56;
const int BITMAPV4HEADER_SIZE     = 108;
const int BITMAPV5HEADER_SIZE     = 124;
typedef struct BITMAPCOREHEADER
{
  char header_size[4];
  char width[2];
  char height[2];
  char nplanes[2];//must be 1
  char bpp[2];
} BITMAPCOREHEADER;

typedef struct BITMAPINFOHEADER
{
  char header_size[4];
  char width[4];
  char height[4];
  char nplanes[2];//must be 1
  char bpp[2];//1,4,8,16,24,32
  char compression[4];
  char image_size[4];
  char horiz_resolution[4];//pixel per meter
  char vert_resolution[4];//pixel per meter
  char ncolors[4];//0 = default 2^n
  char nimportantcolors[4];//0 if every color is important
} BITMAPINFOHEADER;

typedef struct BITMAPV5HEADER
{
  char bV5Size[4];
  char bV5Width[4];
  char bV5Height[4];
  char bV5Planes[2];
  char bV5BitCount[2];
  char bV5Compression[4];
  char bV5SizeImage[4];
  char bV5XPelsPerMeter[4];
  char bV5YPelsPerMeter[4];
  char bV5ClrUsed[4];
  char bV5ClrImportant[4];
  char bV5RedMask[4];
  char bV5GreenMask[4];
  char bV5BlueMask[4];
  char bV5AlphaMask[4];
  char bV5CSType[4];
  //CIEXYZTRIPLE bV5Endpoints;
    //CIEXYZ ciexyzRed;
      char RciexyzX[2];
      char RciexyzY[2];
      char RciexyzZ[2];
    //CIEXYZ ciexyzGreen;
      char GciexyzX[2];
      char GciexyzY[2];
      char GciexyzZ[2];
    //CIEXYZ ciexyzBlue;
      char BciexyzX[2];
      char BciexyzY[2];
      char BciexyzZ[2];
  char bV5GammaRed[4];
  char bV5GammaGreen[4];
  char bV5GammaBlue[4];
  char bV5Intent[4];
  char bV5ProfileData[4];
  char bV5ProfileSize[4];
  char bV5Reserved[4];
} BITMAPV5HEADER;

typedef union DIBHeader
{
  char header_size[4]; //common to all
  BITMAPCOREHEADER bitmap_core_header;
  char OS22XBITMAPHEADER[64];
  BITMAPINFOHEADER bitmap_info_header;
  char BITMAPV2INFOHEADER[52];
  char BITMAPV3INFOHEADER[56];
  char BITMAPV4HEADER[108];
  BITMAPV5HEADER bitmap_v5_header;
} DIBHeader;

const int EXTRA_BIT_MASKS_SIZE = 14;
typedef struct Bitmap
{
  BitmapFileHeader bitmap_file_header;
  DIBHeader dib_header;
  char extra_bit_masks[EXTRA_BIT_MASKS_SIZE];
  char *color_table;
  char *gap1;
  char *pixel_array;
  char *gap2;
  char *icc_color_profile;
} Bitmap;

typedef struct Pix
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} Pix;

const char *usage = "Usage: pixql -i input_file -o output_file \"query\"";
const char *invalid = "Invalid file";
int main(int argc, char **argv)
{
  char *infile = 0;
  char *outfile = 0;
  char *query = 0;
  for(int i = 0; i < argc; i++)
  {
         if(cmp(argv[i],"-i") == 0) infile  = argv[++i];
    else if(cmp(argv[i],"-o") == 0) outfile = argv[++i];
    else                            query   = argv[i];
  }

  if(!infile)  error(1,"%s\nNo input file specified.", usage);
  if(!outfile) error(1,"%s\nNo output file specified.",usage);
  if(!query)   error(1,"%s\nNo query specified.",      usage);

/*
  printf("Infile: %s\n", infile);
  printf("Outfile: %s\n",outfile);
  printf("Query: %s\n", query);
*/

  FILE *in;
  if(!(in  = fopen(infile,  "r"))) error(1,"Can't open input file- %s",infile);

  Bitmap b;
  int n;

  //bitmap_header
  //read header_field
  n = 2; if(n != fread(b.bitmap_file_header.header_field, sizeof(char), n, in)) error(1,"%s",invalid);
  //read size
  n = 4; if(n != fread(b.bitmap_file_header.size, sizeof(char), n, in)) error(1,"%s",invalid);
  //get through reserved
  n = 2; if(n != fread(b.bitmap_file_header.reserved_a, sizeof(char), n, in)) error(1,"%s",invalid);
  n = 2; if(n != fread(b.bitmap_file_header.reserved_b, sizeof(char), n, in)) error(1,"%s",invalid);
  //read offset
  n = 4; if(n != fread(b.bitmap_file_header.offset, sizeof(char), n, in)) error(1,"%s",invalid);

  //dib_header
  //read dib_header_size
  n = 4; if(n != fread(b.dib_header.header_size, sizeof(char), n, in)) error(1,"%s",invalid);
  int header_size = (*(int*)b.dib_header.header_size);
  BITMAPINFOHEADER *bih;
  switch(header_size)
  {
    case BITMAPINFOHEADER_SIZE:
      bih = &b.dib_header.bitmap_info_header;
    case BITMAPV5HEADER_SIZE:
      bih = (BITMAPINFOHEADER *)&b.dib_header.bitmap_v5_header;
      break;
    case BITMAPCOREHEADER_SIZE:
    case OS22XBITMAPHEADER_SIZE:
    case BITMAPV2INFOHEADER_SIZE:
    case BITMAPV3INFOHEADER_SIZE:
    case BITMAPV4HEADER_SIZE:
    default:
      error(1,"Unsupported DIB header\n");
  }
  //read bitmap size
  n = 4; if(n != fread(bih->width,  sizeof(char), n, in)) error(1,"%s",invalid);
  n = 4; if(n != fread(bih->height, sizeof(char), n, in)) error(1,"%s",invalid);
  int width  = *(int *)bih->width;
  int height = *(int *)bih->height;
  //read n planes (must be 1)
  n = 2; if(n != fread(bih->nplanes,  sizeof(char), n, in)) error(1,"%s",invalid);
  n = 2; if(n != fread(bih->bpp,  sizeof(char), n, in)) error(1,"%s",invalid);
  int bpp = *(int *)bih->bpp;
  /*
  printf("width %d\n",width);
  printf("height %d\n",height);
  printf("bpp %d\n",bpp);
  */

  fseek(in, *((int*)b.bitmap_file_header.offset), SEEK_SET);
  int roww = ((bpp*width+31)/32)*4;
  int pixel_bytes = roww*height;
  b.pixel_array = malloc(pixel_bytes);
  n = pixel_bytes; if(n != fread(b.pixel_array, sizeof(char), n, in)) error(1,"%s",invalid);

  Pix *pixels = malloc(width*height*sizeof(Pix));
  switch(bpp)
  {
    case 32:
      for(int i = 0; i < height; i++)
      {
        for(int j = 0; j < width; j++)
        {
          pixels[(i*width)+j].r = b.pixel_array[(i*roww)+j+0];
          pixels[(i*width)+j].g = b.pixel_array[(i*roww)+j+1];
          pixels[(i*width)+j].b = b.pixel_array[(i*roww)+j+2];
          pixels[(i*width)+j].a = b.pixel_array[(i*roww)+j+3];
        }
      }
    break;
    default:
      error(1,"Invaid BPP");
  }

  //MODIFY
  for(int i = 0; i < height; i++)
    pixels[(i*width)+(int)(((float)i/(float)height)*width)].r = 0xFF;

  FILE *out;
  if(!(out = fopen(outfile, "w"))) error(1,"Can't open output file- %s",outfile);


  int filesize = *(unsigned int *)&b.bitmap_file_header.size;
  char *indata = malloc(filesize);

  fseek(in, 0, SEEK_SET);
  fread( indata, sizeof(char), filesize, in);
  fwrite(indata, sizeof(char), filesize, out);
  fseek(out, *((int*)b.bitmap_file_header.offset), SEEK_SET);

  switch(bpp)
  {
    case 32:
      for(int i = 0; i < height; i++)
      {
        for(int j = 0; j < width; j++)
        {
          b.pixel_array[(i*roww)+j+0] = pixels[(i*width)+j].r;
          b.pixel_array[(i*roww)+j+1] = pixels[(i*width)+j].g;
          b.pixel_array[(i*roww)+j+2] = pixels[(i*width)+j].b;
          b.pixel_array[(i*roww)+j+3] = pixels[(i*width)+j].a;
        }
      }
    break;
    default:
      error(1,"Invaid BPP");
  }

  fwrite(b.pixel_array, sizeof(char), pixel_bytes, out);

  fclose(in);
  fclose(out);

  return 0;
}

