#ifndef _QUERY_H_
#define _QUERY_H_

#include <string.h>

typedef enum
{
  QUERY_INIT_MODE_INVALID,
  QUERY_INIT_MODE_NEW,
  QUERY_INIT_MODE_COPY,
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

int strLen(char *s)
{
  int i = 0;
  while(s[i] != '\0') i++;
  return i;
}
char toLower(char c)
{
  if(c > 'A' && c < 'Z')
    return c - ('A'-'a');
  return c;
}
int cmpLower(char *a, char *b)
{
  int i = 0;
  char ca;
  char cb;
  int d;
  while(a[i] != '\0' && b[i] != '\0')
  {
    ca = toLower(a[i]);
    cb = toLower(b[i]);
    d = ca-cb;
    if(d != 0) return d;
    i++;
  }
  if(a[i] == b[i]) return 0;
  if(a[i] == '\0') return -1;
  if(b[i] == '\0') return 1;
  return 0; //shut up compiler
}
int intFromDec(char *s, int *d)
{
  int i = 0;
  *d = 0;
  while(s[i] != '\0')
  {
    if(s[i] < '0' || s[i] > '9') return 0;
    *d *= 10;
    *d += s[i]-'0';
    i++;
  }
  return 1;
}
int readToken(char *s, int offset, char *buff)
{
  int i = 0;
  char c;
  while(1)
  {
    c = s[offset+i];
    switch(c)
    {
      //EOS
      case '\0':
      {
        buff[i] = '\0';
        return i;
      }
        break;
      //whitespace
      case ' ':
      case '\t':
      case '\n':
      {
        buff[i] = '\0';
        return i+1;
      }
        break;
      //delimeter
      case ';':
      case '.':
      case ',':
      case '(':
      case ')':
      {
        if(i == 0)
        {
          buff[i] = c;
          i++;
          buff[i] = '\0';
          return i;
        }
        else
        {
          buff[i] = '\0';
          return i;
        }
      }
        return i;
      //ambiguous delimeter
      case '=':
      case '+':
      case '-':
      case '>':
      case '<':
      case '!':
      {
        if(i == 0)
        {
          buff[i] = c;
          i++;
          if(s[offset+i] == '=')
          {
            buff[i] = '=';
            i++;
          }
          buff[i] = '\0';
          return i;
        }
        else
        {
          buff[i] = '\0';
          return i;
        }
      }
      default:
        buff[i] = c;
        i++;
        break;
    }
  }
  return 0; //to shut the compiler up
}

/*
[ COPY | NEW(w,h) | ] ;
SELECT {SRC} [ FROM {SRC} | ]
WHERE [{SRC}. | ]{PROPERTY}
[ < | > | = ] [ [{SRC}. | ]{PROPERTY} | {CONSTANT} ]
AND ... ;
OPERATE [ R | G | B | A ] = ;
*/

const char *query_usage = "WHAT";

#define teq(s) (cmpLower(token,s) == 0)
#define tok (l = readToken(q,o,token))
#define commit (o += l)
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
      }
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

