#ifndef _QUERY_H_
#define _QUERY_H_

typedef enum
{
  QUERY_INIT_MODE_NEW,
  QUERY_INIT_MODE_COPY,
  QUERY_INIT_MODE_COUNT
} QUERY_INIT_MODE;
typedef enum
{
  QUERY_SRC_IN,
  QUERY_SRC_OUT,
  QUERY_SRC_COUNT
} QUERY_SRC;
typedef enum
{
  QUERY_BOOL_OPERATION_TYPE_AND,
  QUERY_BOOL_OPERATION_TYPE_OR,
  QUERY_BOOL_OPERATION_TYPE_NOT,
  QUERY_BOOL_OPERATION_TYPE_NONE,
  QUERY_BOOL_OPERATION_TYPE_COUNT
} QUERY_BOOL_OPERATION_TYPE;
typedef enum
{
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
  QUERY_VAL_EXPRESSION_TYPE_VALUE,
  QUERY_VAL_EXPRESSION_TYPE_ADD,
  QUERY_VAL_EXPRESSION_TYPE_SUB,
  QUERY_VAL_EXPRESSION_TYPE_MUL,
  QUERY_VAL_EXPRESSION_TYPE_DIV,
  QUERY_VAL_EXPRESSION_TYPE_MOD,
  QUERY_VAL_EXPRESSION_TYPE_COUNT
} QUERY_VAL_EXPRESSION_TYPE;
typedef enum
{
  QUERY_VALUE_TYPE_VAL_EXPRESSION,
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
  QUERY_VAL_EXPRESSION_TYPE type;
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
typedef struct Query
{
  QUERY_INIT_MODE mode;
  int new_w;
  int new_h;
  QuerySelection *selects;
  QueryOperation *operations;
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
      case '+':
      case '-':
      case '=':
      case '>':
      case '<':
      case '!':
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
      default:
        buff[i] = c;
        i++;
        break;
    }
  }
  return 0; //to shut the compiler up
}

Query parseQuery(char *q)
{
  Query query;
  char token[256];
  int o = 0;
  int l = strLen(q);

  o += readToken(q,o,token);
  o += readToken(q,o,token);

  printf("token :%s",token);
  printf("len %d, rd %d",l,o);
  return query;
}

#endif

