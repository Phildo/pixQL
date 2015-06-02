#ifndef _MEAT_H_
#define _MEAT_H_

#include "pix.h"
#include "query.h"

int evaluateValue(QueryValue *v, int row, int col, PixImg *target, PixImg *in, PixImg *out, PixErr *err);
int evaluateExpression(QueryExpression *qexp, int row, int col, PixImg *target, PixImg *in, PixImg *out, PixErr *err);
void evaluateOperation(QueryOperation *op, int row, int col, PixImg *target, PixImg *in, PixImg *out, PixErr *err);
ERR_EXISTS executeQuery(Query *query, PixImg *in_img, PixImg *out_img, PixErr *err);

#endif

