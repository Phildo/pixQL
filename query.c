#include "query.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "str.h"
#include "token.h"

// Query Error passing
#define QERROR(typ,...) ({ err->type = typ; err->position = o; sprintf(err->info, ##__VA_ARGS__); return -1; })
#define QERRORUP ({ err->type = QUERY_ERROR_TYPE_PARSE; return -1; })
#define QERRORPASS ({ return -1; })
#define QERRORCLEAN ({ err->type = QUERY_ERROR_TYPE_NONE; })

typedef enum
{
  QUERY_ERROR_TYPE_NONE,
  QUERY_ERROR_TYPE_OPTIONAL, //used when error might just be misinterpretation of optional syntax
  QUERY_ERROR_TYPE_PARSE
} QUERY_ERROR_TYPE;

typedef struct
{
  QUERY_ERROR_TYPE type;
  char info[256];
  int position;
} QueryError;

// One off helpers
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

static int parseIntoTarget(char *q, int s, int e, QUERY_TARGET *t, QueryError *err);
static int parseIntoExpression(char *q, int s, int e, int level, QueryExpression *qexp, QueryError *err);
static int parseIntoMember(char *q, int s, int e, QueryMember *m, QueryError *err);
static int parseIntoConstant(char *q, int s, int e, QueryConstant *c, QueryError *err);
static int parseIntoValue(char *q, int s, int e, QueryValue *v, QueryError *err);
static int parseIntoOperation(char *q, int s, int e, QueryOperation *op, QueryError *err);
static int parseOperations(char *q, int s, int e, QueryProcedure *pro, QueryError *err);
static int parseIntoSelection(char *q, int s, int e, QuerySelection *sel, QueryError *err);
static int parseSelection(char *q, int s, int e, QueryProcedure *pro, QueryError *err);
static int parseProcedures(char *q, int s, int e, Query *query, QueryError *err);
static int parseIntoInit(char *q, int s, int e, QueryInit *init, QueryError *err);
static int parseInit(char *q, int s, int e, Query *query, QueryError *err);
static int parseIntoQuery(char *q, Query *query, QueryError *err);


static int parseIntoTarget(char *q, int s, int e, QUERY_TARGET *t, QueryError *err)
{
  tokinit;

  tok;
       if(teq("in")) *t = QUERY_TARGET_IN;
  else if(teq("out")) *t = QUERY_TARGET_OUT;
  else QERROR(QUERY_ERROR_TYPE_OPTIONAL,"Error parsing target");
  commit;

  return o-s;
}

static int parseIntoExpression(char *q, int s, int e, int level, QueryExpression *qexp, QueryError *err)
{
  tokinit;

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
      l = lastTokenLevelInRange(q,s,e,level);
      if(l >= 0)
      {
        qexp->a = malloc(sizeof(QueryExpression));

        l = parseIntoExpression(q, o, o+l, level, qexp->a, err);
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }

        tok;
        if(!isTokenLevel(token, level))
        {
          QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing expression, expected operator of lvl %d",level); //should specify which operator...
        }
        else
        {
               if(teq("or")) qexp->type  = QUERY_EXPRESSION_TYPE_OR;
          else if(teq("and")) qexp->type = QUERY_EXPRESSION_TYPE_AND;
          else if(teq("=")) qexp->type   = QUERY_EXPRESSION_TYPE_EQ;
          else if(teq("!=")) qexp->type  = QUERY_EXPRESSION_TYPE_NE;
          else if(teq("<")) qexp->type   = QUERY_EXPRESSION_TYPE_LT;
          else if(teq("<=")) qexp->type  = QUERY_EXPRESSION_TYPE_LTE;
          else if(teq(">=")) qexp->type  = QUERY_EXPRESSION_TYPE_GTE;
          else if(teq(">")) qexp->type   = QUERY_EXPRESSION_TYPE_GT;
          else if(teq("-")) qexp->type   = QUERY_EXPRESSION_TYPE_SUB;
          else if(teq("+")) qexp->type   = QUERY_EXPRESSION_TYPE_ADD;
          else if(teq("/")) qexp->type   = QUERY_EXPRESSION_TYPE_DIV;
          else if(teq("*")) qexp->type   = QUERY_EXPRESSION_TYPE_MUL;
          else if(teq("%")) qexp->type   = QUERY_EXPRESSION_TYPE_MOD;
          else QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing expression, expected operator");

          commit;
          qexp->b = malloc(sizeof(QueryExpression));
          l = parseIntoExpression(q, o, e, level+1, qexp->b, err);
          switch(err->type)
          {
            case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
            case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
            case QUERY_ERROR_TYPE_NONE: commit; break;
          }
        }
      }
      else
      {
        l = parseIntoExpression(q, s, e, level+1, qexp, err);
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }
      }
    }
      break;
    case 2: // not
    {
      tok;
      if(teq("not"))
      {
        commit;
        qexp->type = QUERY_EXPRESSION_TYPE_NOT;
        qexp->a = malloc(sizeof(QueryExpression));
        l = parseIntoExpression(q, o, e, level, qexp->a, err);
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }
      }
      else
      {
        l = parseIntoExpression(q, o, e, level+1, qexp, err);
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }
      }
      break;
    }
    case 8: // SIN, COS, TAN, ABS
    {
      tok;
      if(isTokenLevel(token, level))
      {
        commit;
             if(teq("sin")) qexp->type = QUERY_EXPRESSION_TYPE_SIN;
        else if(teq("cos")) qexp->type = QUERY_EXPRESSION_TYPE_COS;
        else if(teq("tan")) qexp->type = QUERY_EXPRESSION_TYPE_TAN;
        else if(teq("abs")) qexp->type = QUERY_EXPRESSION_TYPE_ABS;
        qexp->a = malloc(sizeof(QueryExpression));
        l = parseIntoExpression(q, o, e, level, qexp->a, err);
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }
      }
      else
      {
        l = parseIntoExpression(q, o, e, level+1, qexp, err);
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }
      }
      break;
    }
    case 9: // ( )
    {
      tok;
      if(teq("("))
      {
        int closeparen = closingParen(q, o, e);
        commit;
        l = parseIntoExpression(q, o, o+closeparen, 0, qexp, err); //start level over
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }

        tok;
        if(!teq(")")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing expression, expected ')'");
        commit;
      }
      else
      {
        l = parseIntoExpression(q, o, e, level+1, qexp, err);
        switch(err->type)
        {
          case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
          case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
          case QUERY_ERROR_TYPE_NONE: commit; break;
        }
      }
      break;
    }
    case 10: // value
    {
      qexp->type = QUERY_EXPRESSION_TYPE_VALUE;
      l = parseIntoValue(q,o,e,&qexp->v,err);
      switch(err->type)
      {
        case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
        case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
        case QUERY_ERROR_TYPE_NONE: commit; break;
      }
    }
      break;
    default:
      break;
  }
  return o-s;
}

static int parseIntoMember(char *q, int s, int e, QueryMember *m, QueryError *err)
{
  tokinit;
  int tmp_pos = 0;

  l = parseIntoTarget(q,o,e,&m->target,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL:
      m->target = QUERY_TARGET_FALLBACK;
      QERRORCLEAN;
    break;
    case QUERY_ERROR_TYPE_NONE: commit; break;
  }

  tok;
  if(teq("("))
  {
    commit;

    tmp_pos = charPos(q,',',o);
    m->row = malloc(sizeof(QueryExpression));
    l = parseIntoExpression(q,o,tmp_pos,0,m->row,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
      case QUERY_ERROR_TYPE_NONE: commit; break;
    }

    tok;
    if(!teq(",")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing member, expected ','");
    commit;

    m->col = malloc(sizeof(QueryExpression));
    l = parseIntoExpression(q,o,e,0,m->col,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
      case QUERY_ERROR_TYPE_NONE: commit; break;
    }

    tok;
    if(!teq(")")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing member, expected ')'");
    commit;
    tok;
  }
  else
  {
    m->row = NULL;
    m->col = NULL;
  }

  if(o != s)
  {
    if(!teq(".")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing member, expected '.'");
    commit;
    tok;
  }

       if(teq("color")) m->type = QUERY_MEMBER_TYPE_COLOR;
  else if(teq("r")) m->type = QUERY_MEMBER_TYPE_R;
  else if(teq("g")) m->type = QUERY_MEMBER_TYPE_G;
  else if(teq("b")) m->type = QUERY_MEMBER_TYPE_B;
  else if(teq("a")) m->type = QUERY_MEMBER_TYPE_A;
  else if(teq("row")) m->type = QUERY_MEMBER_TYPE_ROW;
  else if(teq("col")) m->type = QUERY_MEMBER_TYPE_COL;
  else QERROR(QUERY_ERROR_TYPE_OPTIONAL,"Error parsing member"); //only optional if no parsing yet
  commit;

  return o-s;
}

static int parseIntoConstant(char *q, int s, int e, QueryConstant *c, QueryError *err)
{
  tokinit;

  tok;
       if(teq("width")) c->type = QUERY_CONSTANT_TYPE_WIDTH;
  else if(teq("height")) c->type = QUERY_CONSTANT_TYPE_HEIGHT;
  else if(teq("row")) c->type = QUERY_CONSTANT_TYPE_ROW;
  else if(teq("col")) c->type = QUERY_CONSTANT_TYPE_COL;
  else
  {
    if(!intFromDec(token,&c->value)) QERROR(QUERY_ERROR_TYPE_OPTIONAL,"Error parsing constant");
    c->type = QUERY_CONSTANT_TYPE_NUMBER;
  }
  commit;

  return o-s;
}

static int parseIntoValue(char *q, int s, int e, QueryValue *v, QueryError *err)
{
  basictokinit;

  l = parseIntoConstant(q,o,e,&v->constant,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL: break;
    case QUERY_ERROR_TYPE_NONE:
      v->type = QUERY_VALUE_TYPE_CONSTANT;
      commit;
      return o-s;
    break;
  }

  l = parseIntoMember(q,o,e,&v->member,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
    case QUERY_ERROR_TYPE_NONE:
      v->type = QUERY_VALUE_TYPE_MEMBER;
      commit;
      return o-s;
    break;
  }

  return -1; //success of const or member returns
}

static int parseIntoOperation(char *q, int s, int e, QueryOperation *op, QueryError *err)
{
  tokinit;

  tok;
  if(!teq("operate")) QERROR(QUERY_ERROR_TYPE_OPTIONAL,"Error parsing operation, expected 'OPERATE'");
  commit;

  l = parseIntoTarget(q,o,e,&op->operating,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL:
      op->operating = QUERY_TARGET_IN;
      QERRORCLEAN;
      break;
    case QUERY_ERROR_TYPE_NONE: commit; break;
  }

  tok;
  if(!teq("set")) QERROR(QUERY_ERROR_TYPE_OPTIONAL,"Error parsing operation, expected 'SET'");
  commit;

  l = parseIntoMember(q,o,e,&op->lval,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
    case QUERY_ERROR_TYPE_NONE: commit; break;
  }

  tok;
  if(!teq("=")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing operation, expected '='");
  commit;

  l = parseIntoExpression(q, o, e, 0, &op->rval, err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
    case QUERY_ERROR_TYPE_NONE: commit; break;
  }

  return o-s;
}

static int parseOperations(char *q, int s, int e, QueryProcedure *pro, QueryError *err)
{
  tokinit;

  int reading_operations = 1;
  while(reading_operations)
  {
    e = charPos(q,';',o);
    pro->operations = expand(pro->operations, pro->n_operations, sizeof(QueryOperation));
    pro->n_operations++;

    QueryOperation *op = &pro->operations[pro->n_operations-1];
    l = parseIntoOperation(q,o,e,op,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL:
        pro->n_operations--;
        if(pro->n_operations == 0) QERRORUP;
        QERRORCLEAN;
        reading_operations = 0;
        break;
      case QUERY_ERROR_TYPE_NONE:
        commit;
        tok;
        if(!teq(";")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing operation, expected ';'");
        commit;
        break;
    }
  }

  return o-s;
}

static int parseIntoSelection(char *q, int s, int e, QuerySelection *sel, QueryError *err)
{
  tokinit;

  tok;
  if(!teq("select")) QERROR(QUERY_ERROR_TYPE_OPTIONAL,"Error parsing selection, expected 'SELECT'");
  commit;

  l = parseIntoTarget(q,o,e,&sel->selecting,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL:
      sel->selecting = QUERY_TARGET_IN;
      QERRORCLEAN;
      break;
    case QUERY_ERROR_TYPE_NONE: commit; break;
  }

  tok;
  if(teq("where"))
  {
    commit;

    l = parseIntoExpression(q,o,e,0,&sel->exp,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL: QERRORPASS; break;
      case QUERY_ERROR_TYPE_NONE: commit; break;
    }
  }

  return o-s;
}

static int parseSelection(char *q, int s, int e, QueryProcedure *pro, QueryError *err)
{
  tokinit;
  e = charPos(q,';',o);

  QuerySelection *sel = &pro->selection;
  l = parseIntoSelection(q,o,e,sel,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL:
      sel->selecting = QUERY_TARGET_IN;
      //ridiculous...
      sel->exp.type = QUERY_EXPRESSION_TYPE_VALUE;
      sel->exp.v.type = QUERY_VALUE_TYPE_CONSTANT;
      sel->exp.v.constant.type = QUERY_CONSTANT_TYPE_NUMBER;
      sel->exp.v.constant.value = 1;
      QERRORCLEAN;
      break;
    case QUERY_ERROR_TYPE_NONE:
      commit;
      tok;
      if(!teq(";")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing selection, expected ';'");
      commit;
      break;
  }

  return o-s;
}

static int parseProcedures(char *q, int s, int e, Query *query, QueryError *err)
{
  tokinit;

  int reading_procedures = 1;
  while(reading_procedures)
  {
    query->procedures = expand(query->procedures, query->n_procedures, sizeof(QueryProcedure));
    query->n_procedures++;
    QueryProcedure *pro = &query->procedures[query->n_procedures-1];

    l = parseSelection(q,o,e,pro,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
      case QUERY_ERROR_TYPE_NONE: commit; break;
    }

    l = parseOperations(q,o,e,pro,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL:
        query->n_procedures--;
        if(query->n_procedures == 0) QERRORUP;
        QERRORCLEAN;
        reading_procedures = 0;
      break;
      case QUERY_ERROR_TYPE_NONE: commit; break;
    }

    tok;
    if(teq("")) reading_procedures = 0;
  }

  return o-s;
}

static int parseIntoInit(char *q, int s, int e, QueryInit *init, QueryError *err)
{
  tokinit;
  int tmp_pos = 0;

  tok;
       if(teq("copy")) init->type = QUERY_INIT_TYPE_COPY;
  else if(teq("clear")) init->type = QUERY_INIT_TYPE_CLEAR;
  else if(teq("white")) init->type = QUERY_INIT_TYPE_WHITE;
  else if(teq("black")) init->type = QUERY_INIT_TYPE_BLACK;
  else QERROR(QUERY_ERROR_TYPE_OPTIONAL,"Error parsing init mode type");
  commit;

  tok;
  if(teq("("))
  {
    commit;

    tmp_pos = charPos(q,',',o);
    l = parseIntoExpression(q,o,tmp_pos,0,&init->width,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
      case QUERY_ERROR_TYPE_NONE: commit; break;
    }

    tok;
    if(!teq(",")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing init, expected ','");
    commit;

    l = parseIntoExpression(q,o,e,0,&init->height,err);
    switch(err->type)
    {
      case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
      case QUERY_ERROR_TYPE_OPTIONAL: QERRORUP; break;
      case QUERY_ERROR_TYPE_NONE: commit; break;
    }

    tok;
    if(!teq(")")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing init, expected ')'");
    commit;
  }
  else
  {
    init->width.type = QUERY_EXPRESSION_TYPE_VALUE;
    init->width.v.type = QUERY_VALUE_TYPE_CONSTANT;
    init->width.v.constant.type = QUERY_CONSTANT_TYPE_NUMBER;
    init->width.v.constant.value = 0;
    init->height.type = QUERY_EXPRESSION_TYPE_VALUE;
    init->height.v.type = QUERY_VALUE_TYPE_CONSTANT;
    init->height.v.constant.type = QUERY_CONSTANT_TYPE_NUMBER;
    init->height.v.constant.value = 0;
  }

  return o-s;
}

static int parseInit(char *q, int s, int e, Query *query, QueryError *err)
{
  tokinit;

  l = parseIntoInit(q,o,s,&query->init,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL:
      query->init.type = QUERY_INIT_TYPE_COPY;
      //ridiculous...
      query->init.width.type = QUERY_EXPRESSION_TYPE_VALUE;
      query->init.width.v.type = QUERY_VALUE_TYPE_CONSTANT;
      query->init.width.v.constant.type = QUERY_CONSTANT_TYPE_NUMBER;
      query->init.width.v.constant.value = 0;
      query->init.height.type = QUERY_EXPRESSION_TYPE_VALUE;
      query->init.height.v.type = QUERY_VALUE_TYPE_CONSTANT;
      query->init.height.v.constant.type = QUERY_CONSTANT_TYPE_NUMBER;
      query->init.height.v.constant.value = 0;
      QERRORCLEAN;
      break;
    case QUERY_ERROR_TYPE_NONE:
      commit;
      tok;
      if(!teq(";")) QERROR(QUERY_ERROR_TYPE_PARSE,"Error parsing init, expected ';'");
      commit;
      break;
  }

  return o-s;
}

static int parseIntoQuery(char *q, Query *query, QueryError *err)
{
  int s = 0;
  int e = strLen(q);
  basictokinit;

  l = parseInit(q,o,e,query,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL: QERRORCLEAN; break;
    case QUERY_ERROR_TYPE_NONE: commit; break;
  }

  l = parseProcedures(q,o,e,query,err);
  switch(err->type)
  {
    case QUERY_ERROR_TYPE_PARSE: QERRORPASS; break;
    case QUERY_ERROR_TYPE_OPTIONAL: QERRORCLEAN; break;
    case QUERY_ERROR_TYPE_NONE: commit; break;
  }

  return o-s;
}

ERR_EXISTS parseQuery(char *q, Query *query, PixErr *err)
{
  initTokens();
  QueryError qerr = {0};
  int l = parseIntoQuery(q, query, &qerr);
  if(l == -1)
  {
    //format two-line error output
      //query error
    int i = 0;
    while(qerr.info[i] != '\0')
    {
      err->info[i] = qerr.info[i];
      i++;
    }
    err->info[i] = '\n';
    i++;
      //query
    int j = 0;
    while(q[j] != '\0')
    {
      err->info[i] = q[j];
      i++; j++;
    }
    err->info[i] = '\n';
    i++;
      //position
    int k = 0;
    while(k < qerr.position)
    {
      err->info[i] = ' ';
      i++; k++;
    }
    err->info[i] = '^';
    i++;
    err->info[i] = '\n';
    i++;
    err->info[i] = '\0';
    return ERR;
  }
  return NO_ERR;
}

