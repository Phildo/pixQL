#ifndef _QUERY_H_
#define _QUERY_H_

#include <string.h>
#include "str.h"
#include "token.h"

#include "query_def.h"

void *expand(void *src, int cur_n, int size)
{
  void *tmp = malloc((cur_n+1)*size);
  if(cur_n > 0)
  {
    memcpy(tmp, src, cur_n*size);
    free(src);
  }
  return tmp;
}

int parseExpression(char *q, int s, int e, int level, QueryExpression *qexp, QueryError *err)
{
  tokinit;
  o = s;

  switch(level)
  {
    case 0: // or
    case 1: // and
    case 3: // = | !=
    case 4: // < | <= | >= | >
    case 5: // - | +
    case 6: // / | *
    case 7: // %
    {
      int parsed = 0;
      int tokloc = lastTokenLevelInRange(q,s,e,level);
      if(tokloc != -1)
      {
        qexp->a = malloc(sizeof(QueryExpression));
        l = parseExpression(q, o, tokloc, level, qexp->a, err);
        commit;
        tok;
        if(!isTokenLevel(token, level))
        {
          free(qexp->a);
          qexp->a = 0;
          o = s;
          l = 0;
        }
        else
        {
          commit;
          qexp->b = malloc(sizeof(QueryExpression));
          l = parseExpression(q, o, e, level+1, qexp->b, err);
          commit;

          parsed = 1;
        }
      }

      if(!parsed)
      {
        l = parseExpression(q, s, e, level+1, qexp, err);
        commit;
      }
    }
      break;
    case 2: // not
    {
      tok;
      if(teq("not"))
      {
        commit;
        qexp->type = QUERY_OPERATION_TYPE_NOT;
        qexp->a = malloc(sizeof(QueryExpression));
        l = parseExpression(q, o, e, level+1, qexp->a, err);
        commit;
      }
      else
      {
        l = parseExpression(q, o, e, level+1, qexp, err);
        commit;
      }
      break;
    }
    case 8: // ( )
    {
      tok;
      if(teq("("))
      {
        int closeparen = closingParen(q, o, e);
        commit;
        l = parseExpression(q, o, closeparen, 0, qexp, err); //start level over
        commit;
        tok;
        if(!teq(")")) return -1; //err("Expected ')'");
        commit;
      }
      else
      {
        l = parseExpression(q, o, e, level+1, qexp, err);
        commit;
      }
      break;
    }
    case 9: // value
    {
      qexp->type = QUERY_OPERATION_TYPE_VALUE;
      tok;
           if(teq("row"))    qexp->value_type = QUERY_VALUE_TYPE_ROW;
      else if(teq("col"))    qexp->value_type = QUERY_VALUE_TYPE_COL;
      else if(teq("r"))      qexp->value_type = QUERY_VALUE_TYPE_R;
      else if(teq("g"))      qexp->value_type = QUERY_VALUE_TYPE_G;
      else if(teq("b"))      qexp->value_type = QUERY_VALUE_TYPE_B;
      else if(teq("a"))      qexp->value_type = QUERY_VALUE_TYPE_A;
      else if(teq("width"))  qexp->value_type = QUERY_VALUE_TYPE_WIDTH;
      else if(teq("height")) qexp->value_type = QUERY_VALUE_TYPE_HEIGHT;
      else
      {
        qexp->value_type = QUERY_VALUE_TYPE_CONSTANT;
        int a;
        if(intFromDec(token,&a))
          qexp->value = a;
      }
    }
      break;
    default:
      break;
  }
  return o;
}

int parseSelection(char *q, int s, int e, QuerySelection *sel, QueryError *err)
{
  tokinit;
  o = s;

  tok;
  if(!teq("select")) return -1;
  commit;

  tok;
  if(teq("IN"))
  {
    sel->selecting = QUERY_TARGET_IN;
    commit;
  }
  else if(teq("OUT"))
  {
    sel->selecting = QUERY_TARGET_OUT;
    commit;
  }
  else
  {
    sel->selecting = QUERY_TARGET_IN;
  }

  tok;
  if(teq("FROM"))
  {
    commit; tok;
    if(teq("IN"))
    {
      sel->reference = QUERY_TARGET_IN;
      commit;
    }
    else if(teq("OUT"))
    {
      sel->reference = QUERY_TARGET_OUT;
      commit;
    }
    else
    {
      return -1; //err("Expected 'TARGET'");
    }
  }
  else
  {
    sel->reference = sel->selecting;
  }

  tok;
  if(!teq("WHERE"))
  {
    if(!teq(";")) return -1; //err("Expected 'WHERE'");
    else
    {
      commit;
      return o;
    }
  }
  commit;

  l = parseExpression(q, o, e, 0, &sel->exp, err);
  commit;
  return o;
}

int parseMode(char *q, int s, int e, Query *query, QueryError *err)
{
  tokinit;
  o = s;

  tok;
  if(teq("copy"))
  {
    query->mode = QUERY_INIT_MODE_COPY;
    commit;
    return o;
  }

  if(teq("new"))
  {
    query->mode = QUERY_INIT_MODE_NEW;
    commit; tok;
    if(!teq("(")) return -1;//err("Expected '('");
    commit; tok;
    if(!intFromDec(token,&query->new_w)) return -1;//err("Expected int");
    commit; tok;
    if(!teq(",")) return -1;//err("Expected ','");
    commit; tok;
    if(!intFromDec(token,&query->new_h)) return -1;//err("Expected int");
    if(!teq(")")) return -1;//err("Expected ')'");
    commit; tok;
    if(!teq(";")) return -1;//err("Expected ';'");
    commit;
    return o;
  }

  if(teq("select"))
  {
    query->mode = QUERY_INIT_MODE_NEW;
    return 0;
  }

  return -1;
}

int parseQuery(char *q, Query *query, QueryError *err)
{
  tokinit;
  int qlen = strLen(q);

  l = parseMode(q, o, qlen, query, err);
  commit;

  //PROCEDURES
  int reading_procedures = 1;
  while(reading_procedures)
  {
    query->procedures = expand(query->procedures, query->nprocedures, sizeof(QueryProcedure));
    query->nprocedures++;
    QueryProcedure *pro = &query->procedures[query->nprocedures-1];

    //SELECTS
    int reading_selects = 1;
    while(reading_selects)
    {
      pro->selects = expand(pro->selects, pro->nselects, sizeof(QuerySelection));
      pro->nselects++;
      QuerySelection *sel = &pro->selects[pro->nselects-1];

      l = parseSelection(q, o, qlen, sel, err);
      if(l == -1)
      {
        pro->nselects--;
        reading_selects = 0;
      }
      else commit;
    }

    //to get rid of 'unused' compiler warning
    tok;
    commit;

/*
    //OPERATIONS
    int reading_operations = 1;
    while(reading_operations)
    {
      pro->operations = expand(pro->operations, pro->noperations, sizeof(QueryOperation));
      pro->noperations++;
      QueryOperation *op = &pro->operations[pro->noperations-1];
    }
*/
  }

  return o;
}

#endif

