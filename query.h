#ifndef _QUERY_H_
#define _QUERY_H_

#include "err.h"
#include "query_def.h"

ERR_EXISTS parseQuery(char *q, Query *query, PixErr *err);
void freeQuery(Query *q);

#endif

