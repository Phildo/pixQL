#include <stdio.h>

#include "err.h"
#include "query.h"

#define ERR(s,...) { fprintf(stderr,s,##__VA_ARGS__); return 1; }

int main(int argc, char **argv)
{
  PixErr err;
  Query query = {0};
  QueryInit *init = 0;
  QueryProcedure *pro = 0;
  QuerySelection *sel = 0;
  QueryOperation *op = 0;
  QueryExpression *exp = 0;
  QueryExpression *expa = 0;
  QueryExpression *expb = 0;
  QueryValue *val = 0;
  QueryMember *mem = 0;
  QueryConstant *con = 0;

  char *q_str;
  int i = 0;

  //TEST 0
  q_str = "COPY; SELECT WHERE COL < 100; OPERATE SET A = 0;"; //simple
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  init = &query.init;
  if(init->type != QUERY_INIT_TYPE_COPY) ERR("Init type incorrect");
  if(query.n_procedures != 1) ERR("Wrong number procedures");
  pro = query.procedures;
  sel = &(pro->selection);
  if(sel->selecting != QUERY_TARGET_IN) ERR("Wrong selection target");
  exp = &(sel->exp);
  if(exp->type != QUERY_EXPRESSION_TYPE_LT) ERR("Wrong selection expression type");
  expa = exp->a;
  if(expa->type != QUERY_EXPRESSION_TYPE_VALUE) ERR("Wrong selection expression type for COL");
  val = &(expa->v);
  if(val->type != QUERY_VALUE_TYPE_CONSTANT) ERR("Wrong value type for COL");
  con = &(val->constant);
  if(con->type != QUERY_CONSTANT_TYPE_COL) ERR("Wrong constant type for COL");
  expb = exp->b;
  if(expb->type != QUERY_EXPRESSION_TYPE_VALUE) ERR("Wrong selection expression type for 100");
  val = &(expb->v);
  if(val->type != QUERY_VALUE_TYPE_CONSTANT) ERR("Wrong value type for 100");
  con = &(val->constant);
  if(con->type != QUERY_CONSTANT_TYPE_NUMBER) ERR("Wrong constant type for 100");
  if(con->value != 100) ERR("Wrong constant value for 100");
  if(pro->n_operations != 1) ERR("Wrong num operations");
  op = pro->operations;
  if(op->operating != QUERY_TARGET_IN) ERR("Wrong operating target");
  mem = &(op->lval);
  if(mem->type != QUERY_MEMBER_TYPE_A) ERR("Wrong operation member type");
  if(mem->target != QUERY_TARGET_FALLBACK) ERR("Wrong operation member target");
  if(mem->row != NULL) ERR("Wrong operation member row");
  if(mem->col != NULL) ERR("Wrong operation member col");
  exp = &(op->rval);
  if(exp->type != QUERY_EXPRESSION_TYPE_VALUE) ERR("Wrong operation expression type for 0");
  val = &(exp->v);
  if(val->type != QUERY_VALUE_TYPE_CONSTANT) ERR("Wrong value type for 0");
  con = &(val->constant);
  if(con->type != QUERY_CONSTANT_TYPE_NUMBER) ERR("Wrong constant type for 0");
  if(con->value != 0) ERR("Wrong constant value for 0");

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 1
  q_str = "OPERATE SET R=0;"; //minimal
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 2
  q_str = "OPERATE SET R=R-100;"; //spacing
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 3
  q_str = "OPERATE SET R = R+10/2;"; //compound
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 4
  q_str = "OPERATE SET R = (R+10)/2;"; //parens + order of operations
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 5
  q_str = "OPERATE SET R = ((R))/2;"; //redundant/odd parens
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 6
  q_str = "SELECT WHERE COL%100 != 0; OPERATE SET R = 0;"; //compound delimeter token (!=,<=,etc...)
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 7
  q_str = "SELECT WHERE NOT COL %20 > 1; OPERATE SET R = 0;"; //not
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 8
  q_str = "SELECT WHERE SIN(COL) < 0; OPERATE SET R = 0;"; //sin
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 9
  q_str = "SELECT WHERE COL%100 = 0 OR ROW % 100 = 0; OPERATE SET R = 255-R;"; //OR
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 10
  q_str = "OPERATE SET R = B; OPERATE SET A = 255-((R+G+B)/3);"; //compound operate with complex operations
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 11
  q_str = "SELECT WHERE R < B; OPERATE SET R = B; SELECT WHERE G < B; OPERATE SET G = B;"; //multiple procedures
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 12
  q_str = "SELECT WHERE IN.R < IN.B; OPERATE SET R = OUT.B;"; //access targets
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;


  //TEST 13
  q_str = "WHITE; SELECT WHERE COL%100 < 50; OPERATE SET R = R; OPERATE SET G = G; OPERATE SET B = B; SELECT WHERE OUT.R = 255 AND OUT.G = 255 AND OUT.B = 255; OPERATE SET G = 255-G;"; //weird long compound
  printf("Test %d: %s\n",i,q_str);
  if(!parseQuery(q_str, &query, &err))
  ERR("%s",err.info);

  freeQuery(&query);
  printf("Passed\n");
  i++;






  /*
  q_str = "SELECT IN FROM IN WHERE COL < 100 AND ROW < 100; OPERATE R = 0; OPERATE G = 0;";
  q_str = "SELECT OUT FROM IN WHERE A < 128; OPERATE R = A;
  q_str = "SELECT IN FROM OUT WHERE A < 128; OPERATE R = R/2;
  q_str = "BLANK; SELECT WHERE A < (COL/WIDTH)*256; OPERATE R = 255;";
  q_str = "BLANK; SELECT WHERE COL < ROW; OPERATE R = 255; SELECT WHERE COL < (HEIGHT-ROW); OPERATE B = 255;";
  */

}

