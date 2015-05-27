#include "query.h"

int main(int argc, char **argv)
{
  initTokens();

  Query query;
  QueryError err;

  char *q_str;
  q_str = "COPY; SELECT WHERE COL < 100; OPERATE A = 0;"; //simple
  /*
  q_str = "SELECT IN FROM IN WHERE COL < 100 AND ROW < 100; OPERATE R = 0; OPERATE G = 0;";
  q_str = "SELECT OUT FROM IN WHERE A < 128; OPERATE R = A;
  q_str = "SELECT IN FROM OUT WHERE A < 128; OPERATE R = R/2;
  q_str = "BLANK; SELECT WHERE A < (COL/WIDTH)*256; OPERATE R = 255;";
  q_str = "BLANK; SELECT WHERE COL < ROW; OPERATE R = 255; SELECT WHERE COL < (HEIGHT-ROW); OPERATE B = 255;";
  */

  parseQuery(q_str, &query, &err);
}

