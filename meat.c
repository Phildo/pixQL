#include "meat.h"

#include <stdlib.h>

int evaluateValue(QueryValue *v, int row, int col, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  PixImg *t;
  switch(v->target)
  {
    case QUERY_TARGET_IN:  t = in; break;
    case QUERY_TARGET_OUT: t = out; break;
    case QUERY_TARGET_INVALID:
    default:
      t = target;
      break;
  }
  switch(v->type)
  {
    case QUERY_VALUE_TYPE_INVALID:
      return 0;
      break;
    case QUERY_VALUE_TYPE_ROW:
      return row;
      break;
    case QUERY_VALUE_TYPE_COL:
      return col;
      break;
    case QUERY_VALUE_TYPE_R:
      return pixAt(t, col, row)->r;
      break;
    case QUERY_VALUE_TYPE_G:
      return pixAt(t, col, row)->g;
      break;
    case QUERY_VALUE_TYPE_B:
      return pixAt(t, col, row)->b;
      break;
    case QUERY_VALUE_TYPE_A:
      return pixAt(t, col, row)->a;
      break;
    case QUERY_VALUE_TYPE_WIDTH:
      return t->width;
      break;
    case QUERY_VALUE_TYPE_HEIGHT:
      return t->height;
      break;
    case QUERY_VALUE_TYPE_CONSTANT:
      return v->value;
      break;
  }
}

int evaluateExpression(QueryExpression *qexp, int row, int col, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  switch(qexp->type)
  {
    case QUERY_EXPRESSION_TYPE_INVALID:
      return 0;
      break;
    case QUERY_EXPRESSION_TYPE_OR:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) || evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_AND:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) && evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_NOT:
      return !evaluateExpression(qexp->a, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_EQ:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) == evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_NE:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) != evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_LT:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) < evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_LTE:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) <= evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_GTE:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) >= evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_GT:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) > evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_SUB:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) - evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_ADD:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) + evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_DIV:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) / evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_MUL:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) * evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_MOD:
      return evaluateExpression(qexp->a, row, col, target, in, out, err) % evaluateExpression(qexp->b, row, col, target, in, out, err);
      break;
    case QUERY_EXPRESSION_TYPE_VALUE:
      return evaluateValue(&qexp->v, row, col, target, in, out, err);
      break;
  }
}

void evaluateOperation(QueryOperation *op, int row, int col, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  int val = evaluateExpression(&op->rvalue, row, col, target, in, out, err);

  switch(op->lvalue.type)
  {
    case QUERY_VALUE_TYPE_R:
      pixAt(target, col, row)->r = val;
      break;
    case QUERY_VALUE_TYPE_G:
      pixAt(target, col, row)->g = val;
      break;
    case QUERY_VALUE_TYPE_B:
      pixAt(target, col, row)->b = val;
      break;
    case QUERY_VALUE_TYPE_A:
      pixAt(target, col, row)->a = val;
      break;
    case QUERY_VALUE_TYPE_INVALID:
    case QUERY_VALUE_TYPE_ROW:
    case QUERY_VALUE_TYPE_COL:
    case QUERY_VALUE_TYPE_WIDTH:
    case QUERY_VALUE_TYPE_HEIGHT:
    case QUERY_VALUE_TYPE_CONSTANT:
      break;
  }
}

ERR_EXISTS executeQuery(Query *query, PixImg *in_img, PixImg *out_img, PixErr *err)
{
  byte *selection_mask;

  out_img->width  = in_img->width;
  out_img->height = in_img->height;
  out_img->data  = malloc(out_img->width*out_img->height*sizeof(Pix));
  selection_mask = malloc(out_img->width*out_img->height*sizeof(byte));

  //MODIFY
  switch(query->mode)
  {
    case QUERY_INIT_MODE_COPY:
    default:
    {
      for(int i = 0; i < in_img->height; i++)
        for(int j = 0; j < in_img->width; j++)
          *pixAt(out_img,j,i) = *pixAt(in_img,j,i);
    }
      break;
  }

  QueryProcedure *p;
  QuerySelection *s;
  QueryOperation *o;
  PixImg *op_selection;
  PixImg *sel_reference;
  for(int i = 0; i < query->nprocedures; i++)
  {
    p = &query->procedures[i];

    for(int j = 0; j < in_img->width*in_img->height; j++)
      selection_mask[j] = 0;

    s = &p->select;
    switch(s->selecting)
    {
      case QUERY_TARGET_IN:  op_selection = in_img;  break;
      case QUERY_TARGET_OUT: op_selection = out_img; break;
      case QUERY_TARGET_INVALID:
      default:
        op_selection = in_img;
        break;
    }
    switch(s->reference)
    {
      case QUERY_TARGET_IN:  sel_reference = in_img;  break;
      case QUERY_TARGET_OUT: sel_reference = out_img; break;
      case QUERY_TARGET_INVALID:
      default:
        sel_reference = in_img;
        break;
    }

    for(int k = 0; k < in_img->height; k++)
    {
      for(int l = 0; l < in_img->width; l++)
      {
        if(evaluateExpression(&s->exp, l, k, sel_reference, in_img, out_img, err))
          selection_mask[(k*in_img->width)+l] = 1;
      }
    }

    for(int j = 0; j < p->noperations; j++)
    {
      o = &p->operations[j];

      for(int k = 0; k < in_img->height; k++)
      {
        for(int l = 0; l < in_img->width; l++)
        {
          evaluateOperation(o, l, k, sel_reference, in_img, out_img, err);
        }
      }
    }
  }

  return NO_ERR;
}

