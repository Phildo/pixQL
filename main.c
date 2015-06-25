#include <stdio.h>  //printf, fread, fwrite
#include <stdlib.h>  //free (to please valgrind)

#include "dotypes.h"
#include "str.h"
#include "err.h"

#include "pix.h"
#include "query.h"
#include "io.h"
#include "meat.h"

const char *usage = "Usage: pixql -i input_file -o output_file \"query\"";
const char *invalid = "Invalid file- file does not conform to bitmap spec";

ERR_EXISTS parseArgs(int argc, char **argv, char **infile, char **outfile, char **queryfile, char **query, PixErr *err)
{
  for(int i = 0; i < argc; i++)
  {
         if(cmp(argv[i],"-i") == 0) *infile  = argv[++i];
    else if(cmp(argv[i],"-o") == 0) *outfile = argv[++i];
    else                            *query   = argv[i];
  }

  if(!*infile)  ERROR("%s\nNo input file specified.",  usage);
  if(!*outfile) ERROR("%s\nNo output file specified.", usage);
  if(!*queryfile && !*query)
                ERROR("%s\nNo query specified.",       usage);

  return NO_ERR;
}

int main(int argc, char **argv)
{
  PixErr err = {0};

  char *infile_str = 0;
  char *outfile_str = 0;
  char *queryfile_str = 0;
  char *query_str = 0;
  if(!parseArgs(argc, argv, &infile_str, &outfile_str, &queryfile_str, &query_str, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  Query query = {0};
  if(!parseQuery(query_str, &query, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  PixImg in_img = {0};
  if(!readFile(infile_str, &in_img, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  PixImg out_img = {0};
  if(!executeQuery(&query, &in_img, &out_img, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  if(!writeFile(outfile_str, &out_img, &err))
  { fprintf(stderr,"%s\n",err.info); return 1; }

  //to please valgrind
  freeQuery(&query);
  free(in_img.data);
  free(out_img.data);

  return 0;
}

