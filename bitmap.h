#ifndef BITMAP_H
#define BITMAP_H

#include "dotypes.h"
#include "err.h"

#include "pix.h"

/* spec from http://en.wikipedia.org/wiki/BMP_file_format */
typedef struct
{
  byte header_field[2];
  uint32 size;
  uint16 reserved_a;
  uint16 reserved_b;
  uint32 offset;
} BitmapFileHeader;

//this is bonkers
enum { BITMAPCOREHEADER_SIZE   = 12 };
enum { OS22XBITMAPHEADER_SIZE  = 64 };
enum { BITMAPINFOHEADER_SIZE   = 40 };
enum { BITMAPV2INFOHEADER_SIZE = 52 };
enum { BITMAPV3INFOHEADER_SIZE = 56 };
enum { BITMAPV4HEADER_SIZE     = 108 };
enum { BITMAPV5HEADER_SIZE     = 124 };

typedef struct
{
  uint32 header_size;
  uint16 width;
  uint16 height;
  uint16 nplanes;//must be 1
  uint16 bpp;
} BITMAPCOREHEADER;

typedef struct
{
  uint32 header_size;
  int32 width;
  int32 height;
  uint16 nplanes;//must be 1
  uint16 bpp;//1,4,8,16,24,32
  uint32 compression;//0 for BI_RGB (uncompressed)
  uint32 image_size;//0 for BI_RGB (uncompressed)
  int32 horiz_resolution;//pixel per meter
  int32 vert_resolution;//pixel per meter
  uint32 ncolors;//0 = default 2^n
  uint32 nimportantcolors;//0 if every color is important
} BITMAPINFOHEADER;

typedef struct
{
  uint32 bV5Size;
  int32 bV5Width;
  int32 bV5Height;
  uint16 bV5Planes;
  uint16 bV5BitCount;
  uint32 bV5Compression;
  uint32 bV5SizeImage;
  int32 bV5XPelsPerMeter;
  int32 bV5YPelsPerMeter;
  uint32 bV5ClrUsed;
  uint32 bV5ClrImportant;
  uint32 bV5RedMask;
  uint32 bV5GreenMask;
  uint32 bV5BlueMask;
  uint32 bV5AlphaMask;
  uint32 bV5CSType;
  //CIEXYZTRIPLE bV5Endpoints;
    //CIEXYZ ciexyzRed;
      int32 RciexyzX;
      int32 RciexyzY;
      int32 RciexyzZ;
    //CIEXYZ ciexyzGreen;
      int32 GciexyzX;
      int32 GciexyzY;
      int32 GciexyzZ;
    //CIEXYZ ciexyzBlue;
      int32 BciexyzX;
      int32 BciexyzY;
      int32 BciexyzZ;
  uint32 bV5GammaRed;
  uint32 bV5GammaGreen;
  uint32 bV5GammaBlue;
  uint32 bV5Intent;
  uint32 bV5ProfileData;
  uint32 bV5ProfileSize;
  uint32 bV5Reserved;
} BITMAPV5HEADER;

typedef union DIBHeader
{
  uint32 header_size; //common to all
  BITMAPCOREHEADER bitmap_core_header;
  byte OS22XBITMAPHEADER[OS22XBITMAPHEADER_SIZE];
  BITMAPINFOHEADER bitmap_info_header;
  byte BITMAPV2INFOHEADER[BITMAPV2INFOHEADER_SIZE];
  byte BITMAPV3INFOHEADER[BITMAPV3INFOHEADER_SIZE];
  byte BITMAPV4HEADER[BITMAPV4HEADER_SIZE];
  BITMAPV5HEADER bitmap_v5_header;
} DIBHeader;

//internal rep of bitmap
//non-standardized struct
//don't have to wade through versions to access data
typedef struct
{
  uint32 width;
  uint32 height;
  uint32 bpp;
  uint32 row_w;
  uint32 pixel_n_bytes;
  uint32 offset_to_data;
  uint8  compression;
  uint32 r_mask;
  uint32 g_mask;
  uint32 b_mask;
  uint32 a_mask;
} InternalBitmap;

typedef struct
{
  long zero_pad;
  BitmapFileHeader bitmap_file_header;
  DIBHeader dib_header;
  byte *color_table;
  byte *gap1;
  byte *pixel_array;
  byte *gap2;
  byte *icc_color_profile;
  InternalBitmap simple; //<- not part of spec
} Bitmap;

ERR_EXISTS readBitmap(const char *infile, Bitmap *b, PixErr *err);
ERR_EXISTS writeBitmap(const char *outfile, Bitmap *b, PixErr *err);

ERR_EXISTS bitmapToImage(Bitmap *b, PixImg *img, PixErr *err);
ERR_EXISTS imageToBitmap(PixImg *img, Bitmap *b, PixErr *err);

#endif

