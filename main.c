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

int evaluateValue(QueryValue *v, int row, int col, int width, int height, Pix *target, Pix *in, Pix *out)
{
  Pix *t;
  switch(v->target)
  {
    case QUERY_TARGET_IN:  t = in; break;
    case QUERY_TARGET_OUT: t = in; break;
    case QUERY_TARGET_INVALID:
    default:
      t = target;
      break;
  }
  switch(v->type)
  {
    case QUERY_VALUE_TYPE_INVALID:
      return 0;
      break;
    case QUERY_VALUE_TYPE_ROW:
      return row;
      break;
    case QUERY_VALUE_TYPE_COL:
      return col;
      break;
    case QUERY_VALUE_TYPE_R:
      return t[(col*width)+row].r;
      break;
    case QUERY_VALUE_TYPE_G:
      return t[(col*width)+row].g;
      break;
    case QUERY_VALUE_TYPE_B:
      return t[(col*width)+row].b;
      break;
    case QUERY_VALUE_TYPE_A:
      return t[(col*width)+row].a;
      break;
    case QUERY_VALUE_TYPE_WIDTH:
      return width;
      break;
    case QUERY_VALUE_TYPE_HEIGHT:
      return height;
      break;
    case QUERY_VALUE_TYPE_CONSTANT:
      return v->value;
      break;
  }
}

int evaluateExpression(QueryExpression *qexp, int row, int col, int width, int height, Pix *target, Pix *in, Pix *out)
{
  switch(qexp->type)
  {
    case QUERY_EXPRESSION_TYPE_INVALID:
      return 0;
      break;
    case QUERY_EXPRESSION_TYPE_OR:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) || evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_AND:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) && evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_NOT:
      return !evaluateExpression(qexp->a, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_EQ:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) == evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_NE:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) != evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_LT:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) < evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_LTE:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) <= evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_GTE:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) >= evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_GT:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) > evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_SUB:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) - evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_ADD:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) + evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_DIV:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) / evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_MUL:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) * evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_MOD:
      return evaluateExpression(qexp->a, row, col, width, height, target, in, out) % evaluateExpression(qexp->b, row, col, width, height, target, in, out);
      break;
    case QUERY_EXPRESSION_TYPE_VALUE:
      return evaluateValue(&qexp->v, row, col, width, height, target, in, out);
      break;
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
  byte *SELECTION_MASK;

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
  SELECTION_MASK = malloc(ih->width*ih->height*sizeof(byte));

  dataToPix(b.pixel_array, ih->bpp, roww, ih->width, ih->height, IN_DATA);

  //MODIFY
  switch(query.mode)
  {
    case QUERY_INIT_MODE_COPY:
    default:
    {
      for(int i = 0; i < ih->height; i++)
        for(int j = 0; j < ih->width; j++)
          OUT_DATA[(i*ih->width)+j] = IN_DATA[(i*ih->width)+j];
    }
      break;
  }

  QueryProcedure *p;
  QuerySelection *s;
  QueryOperation *o;
  Pix *op_selection;
  Pix *sel_reference;
  for(int i = 0; i < query.nprocedures; i++)
  {
    p = &query.procedures[i];


    for(int j = 0; j < ih->width*ih->height; j++)
      SELECTION_MASK[j] = 0;
    for(int j = 0; j < p->nselects; j++)
    {
      s = &p->selects[j];
      switch(s->selecting)
      {
        case QUERY_TARGET_IN:  op_selection = IN_DATA;  break;
        case QUERY_TARGET_OUT: op_selection = OUT_DATA; break;
        case QUERY_TARGET_INVALID:
        default:
          op_selection = IN_DATA;
          break;
      }
      switch(s->reference)
      {
        case QUERY_TARGET_IN:  sel_reference = IN_DATA;  break;
        case QUERY_TARGET_OUT: sel_reference = OUT_DATA; break;
        case QUERY_TARGET_INVALID:
        default:
          sel_reference = IN_DATA;
          break;
      }

      for(int k = 0; k < ih->height; k++)
      {
        for(int l = 0; l < ih->width; l++)
        {
          if(evaluateExpression(&s->exp, l, k, ih->width, ih->height, sel_reference, IN_DATA, OUT_DATA))
          {
            SELECTION_MASK[j] = 1;
          }
        }
      }
    }
    for(int j = 0; j < p->noperations; j++)
    {
      o = &p->operations[j];
    }
  }


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

