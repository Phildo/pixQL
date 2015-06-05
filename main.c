#include <stdio.h>  //printf, fread, fwrite

#include "dotypes.h"
#include "str.h"
#include "err.h"

#include "pix.h"
#include "query.h"
#include "io.h"
#include "meat.h"

const char *usage = "Usage: pixql -i input_file -o output_file \"query\"";
const char *invalid = "Invalid file- file does not conform to bitmap spec";

ERR_EXISTS parseArgs(int argc, char **argv, char **infile, char **outfile, char **query, PixErr *err)
{
  for(int i = 0; i < argc; i++)
  {
         if(cmp(argv[i],"-i") == 0) *infile  = argv[++i];
    else if(cmp(argv[i],"-o") == 0) *outfile = argv[++i];
    else                            *query   = argv[i];
  }

  if(!*infile)  ERROR(err,"%s\nNo input file specified.",  usage);
  if(!*outfile) ERROR(err,"%s\nNo output file specified.", usage);
  if(!*query)   ERROR(err,"%s\nNo query specified.",       usage);

  return NO_ERR;
}

int main(int argc, char **argv)
{
  PixErr err;

  char *infile_str = 0;
  char *outfile_str = 0;
  char *query_str = 0;
  if(!parseArgs(argc, argv, &infile_str, &outfile_str, &query_str, &err))
  { fprintf(stderr,"%s",err.info); return 1; }

  Query query;
  if(!parseQuery(query_str, &query, &err))
  { fprintf(stderr,"%s",err.info); return 1; }

  PixImg in_img;
  if(!readFile(infile_str, &in_img, &err))
  { fprintf(stderr,"%s",err.info); return 1; }

  PixImg out_img;
  if(!executeQuery(&query, &in_img, &out_img, &err))
  { fprintf(stderr,"%s",err.info); return 1; }

  if(!writeFile(outfile_str, infile_str, &out_img, &err))
  { fprintf(stderr,"%s",err.info); return 1; }

  return 0;
}

