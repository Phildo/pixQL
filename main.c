#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define error(n,err, ...) ({ fprintf(stderr,err, ##__VA_ARGS__); printf("\n"); fflush(stderr); exit(n); })

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

typedef union DIBHeader
{
  BITMAPCOREHEADER bitmap_core_header;
  char OS22XBITMAPHEADER[64];
  BITMAPINFOHEADER bitmap_info_header;
  char BITMAPV2INFOHEADER[52];
  char BITMAPV3INFOHEADER[56];
  char BITMAPV4HEADER[108];
  char BITMAPV5HEADER[124];
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

const char *usage = "Usage: pixql -i input_file -o output_file \"query\"";
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

  FILE *in, *out;

  if(!(in  = fopen(infile,  "r"))) error(1,"Can't open input file- %s",infile);
  if(!(out = fopen(outfile, "w"))) error(1,"Can't open output file- %s",outfile);


/*
  char c;
  c = fgetc(in)
  {
    if(c == EOF) error(1,"Premature end of file");
  //b.bitmap_file_header
  }
*/


  Bitmap b;
  int n;
  //read header_field
  n = 2; if(n != fread(b.bitmap_file_header.header_field, sizeof(char), n, in)) error(1,"Invalid file");
  //read size
  n = 4; if(n != fread(b.bitmap_file_header.size, sizeof(char), n, in)) error(1,"Invalid file");
  //get through reserved
  n = 2; if(n != fread(b.bitmap_file_header.reserved_a, sizeof(char), n, in)) error(1,"Invalid file");
  n = 2; if(n != fread(b.bitmap_file_header.reserved_b, sizeof(char), n, in)) error(1,"Invalid file");
  //read offset
  n = 4; if(n != fread(b.bitmap_file_header.offset, sizeof(char), n, in)) error(1,"Invalid file");
  printf("read %d\n",*((int*)b.bitmap_file_header.offset));

  fclose(in);
  fclose(out);

  return 0;
}

