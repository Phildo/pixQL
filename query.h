#ifndef _QUERY_H_
#define _QUERY_H_

#include <string.h>
#include "str.h"
#include "token.h"

typedef enum
{
  QUERY_INIT_MODE_INVALID,
  QUERY_INIT_MODE_NEW,
  QUERY_INIT_MODE_COPY,
  QUERY_INIT_MODE_BLANK,
  QUERY_INIT_MODE_COUNT
} QUERY_INIT_MODE;
typedef enum
{
  QUERY_SRC_INVALID,
  QUERY_SRC_IN,
  QUERY_SRC_OUT,
  QUERY_SRC_COUNT
} QUERY_SRC;
typedef enum
{
  QUERY_BOOL_OPERATION_TYPE_INVALID,
  QUERY_BOOL_OPERATION_TYPE_AND,
  QUERY_BOOL_OPERATION_TYPE_OR,
  QUERY_BOOL_OPERATION_TYPE_NOT,
  QUERY_BOOL_OPERATION_TYPE_NONE,
  QUERY_BOOL_OPERATION_TYPE_COUNT
} QUERY_BOOL_OPERATION_TYPE;
typedef enum
{
  QUERY_BOOL_EXPRESSION_TYPE_INVALID,
  QUERY_BOOL_EXPRESSION_TYPE_BOOL_OPERATION,
  QUERY_BOOL_EXPRESSION_TYPE_LT,
  QUERY_BOOL_EXPRESSION_TYPE_LTE,
  QUERY_BOOL_EXPRESSION_TYPE_EQ,
  QUERY_BOOL_EXPRESSION_TYPE_NE,
  QUERY_BOOL_EXPRESSION_TYPE_GTE,
  QUERY_BOOL_EXPRESSION_TYPE_GT,
  QUERY_BOOL_EXPRESSION_TYPE_COUNT
} QUERY_BOOL_EXPRESSION_TYPE;
typedef enum
{
  QUERY_VALUE_EXPRESSION_TYPE_INVALID,
  QUERY_VALUE_EXPRESSION_TYPE_VALUE,
  QUERY_VALUE_EXPRESSION_TYPE_ADD,
  QUERY_VALUE_EXPRESSION_TYPE_SUB,
  QUERY_VALUE_EXPRESSION_TYPE_MUL,
  QUERY_VALUE_EXPRESSION_TYPE_DIV,
  QUERY_VALUE_EXPRESSION_TYPE_MOD,
  QUERY_VALUE_EXPRESSION_TYPE_COUNT
} QUERY_VALUE_EXPRESSION_TYPE;
typedef enum
{
  QUERY_VALUE_TYPE_INVALID,
  QUERY_VALUE_TYPE_VALUE_EXPRESSION,
  QUERY_VALUE_TYPE_CONSTANT,
  QUERY_VALUE_TYPE_ROW,
  QUERY_VALUE_TYPE_COL,
  QUERY_VALUE_TYPE_R,
  QUERY_VALUE_TYPE_G,
  QUERY_VALUE_TYPE_B,
  QUERY_VALUE_TYPE_A,
  QUERY_VALUE_TYPE_COUNT
} QUERY_VALUE_TYPE;

//listed in reverse-depth order to minimize forward decl hell
typedef struct QueryOperation
{
  QUERY_VALUE_TYPE type; //cannot be .._TYPE_CONSTANT or .._TYPE_EXPRESSION
} QueryOperation;
struct QueryValExpression;
typedef struct QueryValue
{
  QUERY_VALUE_TYPE type;
  int constant;
  QUERY_SRC src;
  struct QueryValExpression *expression;
} QueryValue;
typedef struct QueryValExpression
{
  QUERY_VALUE_EXPRESSION_TYPE type;
  QueryValue a;
  QueryValue b;
} QueryValExpression;
struct QueryBoolOperation;
typedef struct QueryBoolExpression
{
  QUERY_BOOL_EXPRESSION_TYPE type;
  QueryValExpression a;
  QueryValExpression b;
  struct QueryBoolOperation *boolOp;
} QueryBoolExpression;
typedef struct QueryBoolOperation
{
  QUERY_BOOL_OPERATION_TYPE type;
  QueryBoolExpression a;
  QueryBoolExpression b;
} QueryBoolOperation;
typedef struct QuerySelection
{
  QUERY_SRC selecting;
  QUERY_SRC reference;
  QueryBoolOperation boolOp;
} QuerySelection;
typedef struct QueryProcedure
{
  QuerySelection *selects;
  int nselects;
  QueryOperation *operations;
  int noperations;
} QueryProcedure;
typedef struct Query
{
  QUERY_INIT_MODE mode;
  int new_w;
  int new_h;
  QueryProcedure *procedures;
  int nprocedures;
} Query;

const char *query_usage = "WHAT";

#define err(s) { printf("%s",s); exit(1); }
void *expand(void *src, int cur_n, int size)
{
  void *tmp = malloc(cur_n*(size+1));
  if(size > 0)
  {
    memcpy(tmp, src, cur_n*size);
    free(src);
  }
  return tmp;
}

void tokenAfterParenExpress(char *q, int o, char *token)
{
  int l = 0;

  tok;
  if(!(teq("("))) err("Expected '('");
  commit;

  int parens_deep = 1;
  while(parens_deep)
  {
    tok;
    if(teq("(")) parens_deep++;
    if(teq(")")) parens_deep--;
    commit;
  }
  tok;
}

void tokenBetweenParenExpress(char *q, int o, char *token)
{
  int l = 0;

  tok;
  if(!(teq("("))) err("Expected '('");
  commit;

  tok;
  if(teq("("))
  {
    tokenAfterParenExpress(q,o,token);
    return;
  }
  else
  {
    while(!isMiddleBit(token))
    {
      commit;
      tok;
    }
  }
}


// WHERE ( X < 5 )
// [[((x + 5) != 4) AND (2 < 5)] OR ( 2 == y )]
//                               OR
//                  AND
//            !=
//       +
//                         <
//                                      ==
// [[((x + 5) != 4) AND (2 < 5)] OR ( 2 == y )]

int parseBoolOp(char *q, int o, QueryBoolOperation *boolOp)
{
  int orig_offset = o;
  int l = 0;
  char token[256];

  tok;
  if(!teq("(")) err("Expected '('");
  commit;

  tok;
  if(teq("("))
  {
    tokenBetweenParenExpress(q,o,token);
    if(isTokenType(token,bool_operation_tokens))
    {
           if(teq("AND")) boolOp->type = QUERY_BOOL_OPERATION_TYPE_AND;
      else if(teq("OR"))  boolOp->type = QUERY_BOOL_OPERATION_TYPE_OR;
      else err("Expected AND|OR");

      boolOp->a.type = QUERY_BOOL_EXPRESSION_TYPE_BOOL_OPERATION;
      boolOp->a.boolOp = malloc(sizeof(QueryBoolOperation));
      l = parseBoolOp(q,o,boolOp->a.boolOp);
      commit;
    }
    else if(isTokenType(t,bool_expression_tokens))
    {
      boolOp->type = QUERY_BOOL_OPERATION_TYPE_NONE;
      l = parseBoolExp(q,o,&boolOp->a);
      commit;
    }
    else if(isTokenType(t,val_expression_tokens))
    {
    }
    else err("Expected MiddleBit");
  }

  tok;
  commit;

  tok;
  if(teq("("))
  {
    tokenBetweenParenExpress(q,o,token);
    if(isTokenType(token,bool_operation_tokens))
    {
      boolOp->a.type = QUERY_BOOL_EXPRESSION_TYPE_BOOL_OPERATION;
      boolOp->a.boolOp = malloc(sizeof(QueryBoolOperation));
      l = parseBoolOp(q,o,boolOp->a.boolOp);
      commit;
    }
    else if(isTokenType(t,bool_expression_tokens))
    {
      l = parseBoolExp(q,o,&boolOp->a);
      commit;
    }
    else err("Expected MiddleBit");
  }


  return o-orig_offset;
}

Query parseQuery(char *q)
{
  Query query;
  char token[256];
  int o = 0;
  int l = 0;

  //MODE
  if(query.mode == QUERY_INIT_MODE_INVALID)
  {
    tok;
    if(teq("copy"))
    {
      query.mode = QUERY_INIT_MODE_COPY;
      commit;
    }
    else if(teq("new"))
    {
      query.mode = QUERY_INIT_MODE_NEW;
      commit; tok;
      if(!teq("(")) err("Expected '('");
      commit; tok;
      if(!intFromDec(token,&query.new_w)) err("Expected int");
      commit; tok;
      if(!teq(",")) err("Expected ','");
      commit; tok;
      if(!intFromDec(token,&query.new_h)) err("Expected int");
      if(!teq(")")) err("Expected ')'");
      commit; tok;
      if(!teq(";")) err("Expected ';'");
      commit;
    }
    else if(teq("select"))
    {
      query.mode = QUERY_INIT_MODE_NEW;
    }
    else err(query_usage);
  }

  //PROCEDURES
  int reading_procedures = 1;
  while(reading_procedures)
  {
    query.procedures = expand(query.procedures, query.nprocedures, sizeof(QueryProcedure));
    query.nprocedures++;
    QueryProcedure *pro = &query.procedures[query.nprocedures-1];

    //SELECTS
    int reading_selects = 1;
    while(reading_selects)
    {
      pro->selects = expand(pro->selects, pro->nselects, sizeof(QuerySelection));
      pro->nselects++;
      QuerySelection *sel = &pro->selects[pro->nselects-1];

      tok;
      if(teq("select"))
      {
        commit;
      }
      else
      {
        pro->nselects--;
        reading_selects = 0;
        continue;
      }

      tok;
      if(teq("IN"))
      {
        commit;
        sel->selecting = QUERY_SRC_IN;
      }
      else if(teq("OUT"))
      {
        commit;
        sel->selecting = QUERY_SRC_OUT;
      }
      else
      {
        sel->selecting = QUERY_SRC_IN;
      }

      tok;
      if(teq("FROM"))
      {
        commit; tok;
        if(teq("IN"))
        {
          commit;
          sel->reference = QUERY_SRC_IN;
        }
        else if(teq("OUT"))
        {
          commit;
          sel->reference = QUERY_SRC_OUT;
        }
        else
        {
          err("Expected 'SRC'");
        }
      }
      else
      {
        sel->reference = sel->selecting;
      }

      tok;
      if(!teq("WHERE"))
      {
        if(!teq(";")) err("Expected 'WHERE'");
        else
        {
          commit;
          continue;
        }
      }
      commit;

      //BOOL OPERATIONS
      //X
      //X AND Y
      //X AND Y AND Z
      parseBoolOp(query,o,&sel->boolOp);
    }

/*
[ COPY | NEW(w,h) | ] ;
SELECT [ {SRC} | ] [ FROM {SRC} | ]
WHERE [{SRC}. | ]{PROPERTY}
[ < | > | = ] [ [{SRC}. | ]{PROPERTY} | {CONSTANT} ]
AND ... ;
OPERATE [ R | G | B | A ] = ;
*/

      commit;
    }

    //OPERATIONS
    int reading_operations = 1;
    while(reading_operations)
    {
      pro->operations = expand(pro->operations, pro->noperations, sizeof(QueryOperation));
      pro->noperations++;
      QueryOperation *op = &pro->operations[pro->noperations-1];
    }
  }

  printf("token :%s",token);
  return query;
}

#endif

