#include <stdlib.h> //malloc
#include <stdio.h>  //printf, fread, fwrite
#include <errno.h>  //errno

#include "dotypes.h"
#include "str.h"

#include "pix.h"
#include "query.h"
#include "io.h"
#include "meat.h"

#define ERROR(n,err, ...) ({ fprintf(stderr,err, ##__VA_ARGS__); printf("\n"); exit(n); })

const char *usage = "Usage: pixql -i input_file -o output_file \"query\"";
const char *invalid = "Invalid file- file does not conform to bitmap spec";

void parseArgs(int argc, char **argv, char **infile, char **outfile, char **query, PixErr *err)
{
  for(int i = 0; i < argc; i++)
  {
         if(cmp(argv[i],"-i") == 0) *infile  = argv[++i];
    else if(cmp(argv[i],"-o") == 0) *outfile = argv[++i];
    else                            *query   = argv[i];
  }

  if(!*infile)  ERROR(1,"%s\nNo input file specified.",  usage);
  if(!*outfile) ERROR(1,"%s\nNo output file specified.", usage);
  if(!*query)   ERROR(1,"%s\nNo query specified.",       usage);
}

int main(int argc, char **argv)
{
  PixErr err;

  char *infile_str = 0;
  char *outfile_str = 0;
  char *query_str = 0;
  parseArgs(argc, argv, &infile_str, &outfile_str, &query_str, &err);

  Query query;
  int l = parseQuery(query_str, &query, &err);
  if(l == -1) ERROR(1,"There was an error");

  PixImg in_img;
  readFile(infile_str, &in_img, &err);

  PixImg out_img;
  executeQuery(&query, &in_img, &out_img, &err);

  writeFile(outfile_str, &out_img, &err);

  return 0;
}

