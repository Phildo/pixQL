#include "meat.h"

#include <stdlib.h>
#include <math.h>

int evaluateConstant(QueryConstant *c, int col, int row, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  switch(c->type)
  {
    case QUERY_CONSTANT_TYPE_WIDTH: return target->width; break;
    case QUERY_CONSTANT_TYPE_HEIGHT: return target->height; break;
    case QUERY_CONSTANT_TYPE_ROW: return row; break;
    case QUERY_CONSTANT_TYPE_COL: return col; break;
    case QUERY_CONSTANT_TYPE_NUMBER: return c->value; break;
    default:
      //error!
      return -1;
      break;
  }
}

int evaluateMember(QueryMember *m, int col, int row, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  PixImg *t;
  int prow;
  int pcol;
  switch(m->target)
  {
    case QUERY_TARGET_IN: t = in; break;
    case QUERY_TARGET_OUT: t = out; break;
    default: t = target; break;
  }

  if(m->row) prow = evaluateExpression(m->row,col,row,target,in,out,err);
  else       prow = row;
  if(m->col) pcol = evaluateExpression(m->col,col,row,target,in,out,err);
  else       pcol = col;

  switch(m->type)
  {
    case QUERY_MEMBER_TYPE_COLOR: return pixAt(t,pcol,prow)->r; break; //actually return color
    case QUERY_MEMBER_TYPE_R: return pixAt(t,pcol,prow)->r;break;
    case QUERY_MEMBER_TYPE_G: return pixAt(t,pcol,prow)->g;break;
    case QUERY_MEMBER_TYPE_B: return pixAt(t,pcol,prow)->b;break;
    case QUERY_MEMBER_TYPE_A: return pixAt(t,pcol,prow)->a;break;
    case QUERY_MEMBER_TYPE_ROW: return prow; break;
    case QUERY_MEMBER_TYPE_COL: return pcol; break;
    default:
      //error
      return -1;
      break;
  }
}

int evaluateValue(QueryValue *v, int col, int row, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  switch(v->type)
  {
    case QUERY_VALUE_TYPE_MEMBER:
      return evaluateMember(&v->member,col,row,target,in,out,err);
    break;
    case QUERY_VALUE_TYPE_CONSTANT:
      return evaluateConstant(&v->constant,col,row,target,in,out,err);
    break;
    default:
      //error
      return -1;
      break;
  }
}

int evaluateExpression(QueryExpression *qexp, int col, int row, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  switch(qexp->type)
  {
    case QUERY_EXPRESSION_TYPE_INVALID:
      return 0;
      break;
    case QUERY_EXPRESSION_TYPE_OR:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) || evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_AND:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) && evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_NOT:
      return !evaluateExpression(qexp->a,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_EQ:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) == evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_NE:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) != evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_LT:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) < evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_LTE:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) <= evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_GTE:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) >= evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_GT:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) > evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_SUB:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) - evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_ADD:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) + evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_DIV:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) / evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_MUL:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) * evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_MOD:
      return evaluateExpression(qexp->a,col,row,target,in,out,err) % evaluateExpression(qexp->b,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_SIN:
      return sin(evaluateExpression(qexp->a,col,row,target,in,out,err));
      break;
    case QUERY_EXPRESSION_TYPE_COS:
      return cos(evaluateExpression(qexp->a,col,row,target,in,out,err));
      break;
    case QUERY_EXPRESSION_TYPE_TAN:
      return tan(evaluateExpression(qexp->a,col,row,target,in,out,err));
      break;
    case QUERY_EXPRESSION_TYPE_ABS:
      return abs(evaluateExpression(qexp->a,col,row,target,in,out,err));
      break;
    case QUERY_EXPRESSION_TYPE_NEG:
      return -1 * evaluateExpression(qexp->a,col,row,target,in,out,err);
      break;
    case QUERY_EXPRESSION_TYPE_VALUE:
      return evaluateValue(&qexp->v,col,row,target,in,out,err);
      break;
    default:
      //error
      return -1;
      break;
  }
}

void evaluateOperation(QueryOperation *op, int col, int row, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  PixImg *t;
  switch(op->operating)
  {
    case QUERY_TARGET_IN: t = in; break;
    case QUERY_TARGET_OUT: t = out; break;
    default: t = target; break;
  }

  int val = evaluateExpression(&op->rvalue,col,row,target,in,out,err);

  QueryMember *lval = op->lval;
  int prow;
  int pcol;

  if(lval->row) prow = evaluateExpression(lval->row,col,row,target,in,out,err);
  else          prow = row;
  if(lval->col) pcol = evaluateExpression(lval->col,col,row,target,in,out,err);
  else          pcol = col;

  switch(lval->type)
  {
    case QUERY_MEMBER_TYPE_COLOR: pixAt(t,pcol,prow)->r = val; break; //actually use color
    case QUERY_MEMBER_TYPE_R: pixAt(t,pcol,prow)->r = val; break;
    case QUERY_MEMBER_TYPE_G: pixAt(t,pcol,prow)->g = val; break;
    case QUERY_MEMBER_TYPE_B: pixAt(t,pcol,prow)->b = val; break;
    case QUERY_MEMBER_TYPE_A: pixAt(t,pcol,prow)->a = val; break;
    case QUERY_MEMBER_TYPE_ROW: break; //can of worms
    case QUERY_MEMBER_TYPE_COL: break; //can of worms
    default:
      //error
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
  QueryInit *init = query->init;
  //actually implement init types
  switch(init->type)
  {
    case QUERY_INIT_TYPE_COPY:
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

    s = &p->selection;
    switch(s->selecting)
    {
      case QUERY_TARGET_IN:  op_selection = in_img;  break;
      case QUERY_TARGET_OUT: op_selection = out_img; break;
      case QUERY_TARGET_INVALID:
      default:
        op_selection = in_img;
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

