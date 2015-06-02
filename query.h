#ifndef _QUERY_H_
#define _QUERY_H_

#include "err.h"
#include "query_def.h"

void *expand(void *src, int cur_n, int size);
int parseTarget(char *q, int s, int e, QUERY_TARGET *t, QueryError *err);
int parseValue(char *q, int s, int e, QueryValue *v, QueryError *err);
int parseOperation(char *q, int s, int e, QueryOperation *op, QueryError *err);
int parseExpression(char *q, int s, int e, int level, QueryExpression *qexp, QueryError *err);
int parseSelection(char *q, int s, int e, QuerySelection *sel, QueryError *err);
int parseMode(char *q, int s, int e, Query *query, QueryError *err);
ERR_EXISTS parseQuery(char *q, Query *query, PixErr *err);

#endif

