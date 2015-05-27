#ifndef _QUERY_DEF_H_
#define _QUERY_DEF_H_

typedef enum
{
  QUERY_ERROR_TYPE_INVALID
} QUERY_ERROR_TYPE;
typedef enum
{
  QUERY_INIT_MODE_INVALID,
  QUERY_INIT_MODE_NEW,
  QUERY_INIT_MODE_COPY,
  QUERY_INIT_MODE_BLANK
} QUERY_INIT_MODE;
typedef enum
{
  QUERY_TARGET_INVALID,
  QUERY_TARGET_IN,
  QUERY_TARGET_OUT
} QUERY_TARGET;
typedef enum
{
  QUERY_VALUE_TYPE_INVALID,
  QUERY_VALUE_TYPE_ROW,
  QUERY_VALUE_TYPE_COL,
  QUERY_VALUE_TYPE_R,
  QUERY_VALUE_TYPE_G,
  QUERY_VALUE_TYPE_B,
  QUERY_VALUE_TYPE_A,
  QUERY_VALUE_TYPE_WIDTH,
  QUERY_VALUE_TYPE_HEIGHT,
  QUERY_VALUE_TYPE_CONSTANT
} QUERY_VALUE_TYPE;
typedef enum
{
  QUERY_OPERATION_TYPE_INVALID,
  QUERY_OPERATION_TYPE_AND,
  QUERY_OPERATION_TYPE_OR,
  QUERY_OPERATION_TYPE_NOT,
  QUERY_OPERATION_TYPE_EQ,
  QUERY_OPERATION_TYPE_NE,
  QUERY_OPERATION_TYPE_LT,
  QUERY_OPERATION_TYPE_LTE,
  QUERY_OPERATION_TYPE_GTE,
  QUERY_OPERATION_TYPE_GT,
  QUERY_OPERATION_TYPE_SUB,
  QUERY_OPERATION_TYPE_ADD,
  QUERY_OPERATION_TYPE_DIV,
  QUERY_OPERATION_TYPE_MUL,
  QUERY_OPERATION_TYPE_MOD,
  QUERY_OPERATION_TYPE_VALUE
} QUERY_OPERATION_TYPE;

typedef struct
{
  QUERY_ERROR_TYPE type;
  int at;
  char *message;
} QueryError;
typedef struct QueryExpression
{
  QUERY_OPERATION_TYPE type;
  union
  {
    struct
    {
      struct QueryExpression *a;
      struct QueryExpression *b;
    };
    struct
    {
      QUERY_VALUE_TYPE value_type;
      int value;
    };
  };
} QueryExpression;
typedef struct
{
  QUERY_TARGET selecting;
  QUERY_TARGET reference;
  QueryExpression exp;
} QueryOperation;
typedef struct
{
  QUERY_TARGET selecting;
  QUERY_TARGET reference;
  QueryExpression exp;
} QuerySelection;
typedef struct
{
  QuerySelection *selects;
  int nselects;
  QueryOperation *operations;
  int noperations;
} QueryProcedure;
typedef struct
{
  QUERY_INIT_MODE mode;
  int new_w;
  int new_h;
  QueryProcedure *procedures;
  int nprocedures;
} Query;

#endif

