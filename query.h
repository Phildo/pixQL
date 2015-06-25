#ifndef _QUERY_H_
#define _QUERY_H_

#include "err.h"

typedef enum
{
  QUERY_INIT_TYPE_INVALID,
  QUERY_INIT_TYPE_COPY,
  QUERY_INIT_TYPE_CLEAR,
  QUERY_INIT_TYPE_WHITE,
  QUERY_INIT_TYPE_BLACK,
} QUERY_INIT_TYPE;
typedef enum
{
  QUERY_TARGET_INVALID,
  QUERY_TARGET_IN,
  QUERY_TARGET_OUT,
  QUERY_TARGET_FALLBACK,
} QUERY_TARGET;
typedef enum
{
  QUERY_MEMBER_TYPE_INVALID,
  QUERY_MEMBER_TYPE_COLOR,
  QUERY_MEMBER_TYPE_R,
  QUERY_MEMBER_TYPE_G,
  QUERY_MEMBER_TYPE_B,
  QUERY_MEMBER_TYPE_A,
  QUERY_MEMBER_TYPE_ROW,
  QUERY_MEMBER_TYPE_COL,
} QUERY_MEMBER_TYPE;
typedef enum
{
  QUERY_CONSTANT_TYPE_INVALID,
  QUERY_CONSTANT_TYPE_WIDTH,
  QUERY_CONSTANT_TYPE_HEIGHT,
  QUERY_CONSTANT_TYPE_ROW,
  QUERY_CONSTANT_TYPE_COL,
  QUERY_CONSTANT_TYPE_NUMBER,
} QUERY_CONSTANT_TYPE;
typedef enum
{
  QUERY_VALUE_TYPE_INVALID,
  QUERY_VALUE_TYPE_MEMBER,
  QUERY_VALUE_TYPE_CONSTANT,
} QUERY_VALUE_TYPE;
typedef enum
{
  QUERY_EXPRESSION_TYPE_INVALID,
  QUERY_EXPRESSION_TYPE_OR,
  QUERY_EXPRESSION_TYPE_AND,
  QUERY_EXPRESSION_TYPE_NOT,
  QUERY_EXPRESSION_TYPE_EQ,
  QUERY_EXPRESSION_TYPE_NE,
  QUERY_EXPRESSION_TYPE_LT,
  QUERY_EXPRESSION_TYPE_LTE,
  QUERY_EXPRESSION_TYPE_GTE,
  QUERY_EXPRESSION_TYPE_GT,
  QUERY_EXPRESSION_TYPE_SUB,
  QUERY_EXPRESSION_TYPE_ADD,
  QUERY_EXPRESSION_TYPE_DIV,
  QUERY_EXPRESSION_TYPE_MUL,
  QUERY_EXPRESSION_TYPE_MOD,
  QUERY_EXPRESSION_TYPE_SIN,
  QUERY_EXPRESSION_TYPE_COS,
  QUERY_EXPRESSION_TYPE_TAN,
  QUERY_EXPRESSION_TYPE_ABS,
  QUERY_EXPRESSION_TYPE_NEG,
  QUERY_EXPRESSION_TYPE_VALUE,
} QUERY_EXPRESSION_TYPE;

struct QueryExpression;
typedef struct
{
  QUERY_CONSTANT_TYPE type;
  int value;
} QueryConstant;
typedef struct
{
  QUERY_MEMBER_TYPE type;
  QUERY_TARGET target;
  struct QueryExpression *row;
  struct QueryExpression *col;
} QueryMember;
typedef struct
{
  QUERY_VALUE_TYPE type;
  union
  {
    QueryConstant constant;
    QueryMember member;
  };
} QueryValue;
typedef struct QueryExpression
{
  QUERY_EXPRESSION_TYPE type;
  union
  {
    struct
    {
      struct QueryExpression *a;
      struct QueryExpression *b;
    };
    QueryValue v;
  };
} QueryExpression;
typedef struct
{
  QUERY_TARGET operating;
  QueryMember lval;
  QueryExpression rval;
} QueryOperation;
typedef struct
{
  QUERY_TARGET selecting;
  QueryExpression exp;
} QuerySelection;
typedef struct
{
  QuerySelection *selections;
  int n_selections;
  QueryOperation *operations;
  int n_operations;
} QueryProcedure;
typedef struct
{
  QUERY_INIT_TYPE type;
  QueryExpression width;
  QueryExpression height;
} QueryInit;
typedef struct
{
  long zero_pad;
  QueryInit init;
  QueryProcedure *procedures;
  int n_procedures;
} Query;


ERR_EXISTS parseQuery(char *q, Query *query, PixErr *err);
void freeQuery(Query *q);

#endif

