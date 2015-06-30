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
  PixImg *t = 0;
  int prow = 0;
  int pcol = 0;
  switch(m->target)
  {
    case QUERY_TARGET_IN: t = in; break;
    case QUERY_TARGET_OUT: t = out; break;
    case QUERY_TARGET_FALLBACK: t = target; break;
    case QUERY_TARGET_INVALID:
    default:
      //error
      break;
  }

  if(m->row) prow = evaluateExpression(m->row,col,row,target,in,out,err);
  else       prow = row;
  if(m->col) pcol = evaluateExpression(m->col,col,row,target,in,out,err);
  else       pcol = col;
  if(prow < 0) prow = 0; if(prow > t->width-1) prow = t->width-1;
  if(pcol < 0) pcol = 0; if(pcol > t->width-1) pcol = t->width-1;

  Pix *p;
  switch(m->type)
  {
    case QUERY_MEMBER_TYPE_COLOR:
      p = pixAt(t,pcol,prow);
      return (p->r << 24) + (p->g << 16) + (p->b << 8) + (p->a << 0);
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
    case QUERY_TARGET_FALLBACK: t = target; break;
    case QUERY_TARGET_INVALID:
    default:
      t = target; //to clear up warning
      //error
      break;
  }

  int val = evaluateExpression(&op->rval,col,row,t,in,out,err);

  QueryMember *lval = &op->lval;
  int prow = 0;
  int pcol = 0;

  if(lval->row) prow = evaluateExpression(lval->row,col,row,t,in,out,err);
  else          prow = row;
  if(lval->col) pcol = evaluateExpression(lval->col,col,row,t,in,out,err);
  else          pcol = col;
  if(prow < 0) prow = 0; if(prow > t->width-1) prow = t->width-1;
  if(pcol < 0) pcol = 0; if(pcol > t->width-1) pcol = t->width-1;

  int v;
  switch(lval->type)
  {
    case QUERY_MEMBER_TYPE_COLOR:
      v = (val & 0xff000000) >> 24;
      pixAt(out,pcol,prow)->r = v;
      v = (val & 0x00ff0000) >> 16;
      pixAt(out,pcol,prow)->g = v;
      v = (val & 0x0000ff00) >> 8;
      pixAt(out,pcol,prow)->b = v;
      v = (val & 0x000000ff) >> 0;
      pixAt(out,pcol,prow)->a = v;
      break;
    case QUERY_MEMBER_TYPE_R: pixAt(out,pcol,prow)->r = val; break;
    case QUERY_MEMBER_TYPE_G: pixAt(out,pcol,prow)->g = val; break;
    case QUERY_MEMBER_TYPE_B: pixAt(out,pcol,prow)->b = val; break;
    case QUERY_MEMBER_TYPE_A: pixAt(out,pcol,prow)->a = val; break;
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

  QueryInit *init = &query->init;
  uint32 init_width = evaluateExpression(&init->width,-1,-1,in_img,in_img,out_img,err);
  uint32 init_height = evaluateExpression(&init->height,-1,-1,in_img,in_img,out_img,err);

  if(!in_img->data)
  {
    uint32 color;
    switch(init->type)
    {
      case QUERY_INIT_TYPE_COPY: ERROR("Target init type COPY, but no input file specified"); break;
      case QUERY_INIT_TYPE_WHITE: color = 0xFFFFFFFF; break;
      case QUERY_INIT_TYPE_BLACK: color = 0x000000FF; break;
      case QUERY_INIT_TYPE_CLEAR: color = 0x00000000; break;
      default: color = 0xFFFFFFFF; break;
      /*
        color = 0xFF0000FF;
        color = 0x00FF00FF;
        color = 0x0000FFFF;
        color = 0x00FFFFFF;
        color = 0xFF00FFFF;
        color = 0xFFFF00FF;
      */
    }
    if(!init_width || !init_height) ERROR("No input file nor init dimensions specified");
    if(!initImg(in_img, init_width, init_height, color, err)) return ERR;
  }

  out_img->width = init_width;
  out_img->height = init_height;
  if(out_img->width  == 0) out_img->width = in_img->width;
  if(out_img->height == 0) out_img->height = in_img->height;

  out_img->data  = calloc(out_img->width*out_img->height*sizeof(Pix),1);
  if(!out_img->data) ERROR("Out of memory");
  selection_mask = calloc(out_img->width*out_img->height*sizeof(byte),1);
  if(!selection_mask) ERROR("Out of memory");

  switch(init->type)
  {
    case QUERY_INIT_TYPE_COPY:
      {
        for(int i = 0; i < out_img->height; i++)
          for(int j = 0; j < out_img->width; j++)
            *pixAt(out_img,j,i) = *pixAt(in_img,j,i);
      }
      break;
    case QUERY_INIT_TYPE_CLEAR:
      {
        for(int i = 0; i < out_img->height; i++)
          for(int j = 0; j < out_img->width; j++)
            set(pixAt(out_img,j,i),0,0,0,0);
      }
      break;
    case QUERY_INIT_TYPE_WHITE:
      {
        for(int i = 0; i < out_img->height; i++)
          for(int j = 0; j < out_img->width; j++)
            set(pixAt(out_img,j,i),255,255,255,255);
      }
      break;
    case QUERY_INIT_TYPE_BLACK:
      {
        for(int i = 0; i < out_img->height; i++)
          for(int j = 0; j < out_img->width; j++)
            set(pixAt(out_img,j,i),0,0,0,255);
      }
      break;
    case QUERY_INIT_TYPE_INVALID:
    default:
      //error
      break;
  }

  QueryProcedure *p = 0;
  QuerySelection *s = 0;
  QueryOperation *o = 0;
  PixImg *default_selecting = 0;
  PixImg *default_operating = 0;
  for(int i = 0; i < query->n_procedures; i++)
  {
    p = &query->procedures[i];

    for(int j = 0; j < out_img->width*out_img->height; j++)
      selection_mask[j] = 0;

    for(int j = 0; j < p->n_selections; j++)
    {
      s = &p->selections[j];
      switch(s->selecting)
      {
        case QUERY_TARGET_IN: default_selecting = in_img; break;
        case QUERY_TARGET_OUT: default_selecting = out_img; break;
        case QUERY_TARGET_FALLBACK:
        case QUERY_TARGET_INVALID:
        default:
          //error
          break;
      }

      for(int k = 0; k < out_img->height; k++)
        for(int l = 0; l < out_img->width; l++)
          if(evaluateExpression(&s->exp,l,k,default_selecting,in_img,out_img,err))
            selection_mask[(k*out_img->width)+l] = 1;
    }

    for(int j = 0; j < p->n_operations; j++)
    {
      o = &p->operations[j];
      switch(o->operating)
      {
        case QUERY_TARGET_IN: default_operating = in_img; break;
        case QUERY_TARGET_OUT: default_operating = out_img; break;
        case QUERY_TARGET_FALLBACK:
        case QUERY_TARGET_INVALID:
        default:
          //error
          break;
      }

      for(int k = 0; k < out_img->height; k++)
        for(int l = 0; l < out_img->width; l++)
          if(selection_mask[(k*out_img->width)+l])
            evaluateOperation(o,l,k,default_operating,in_img,out_img,err);
    }
  }

  free(selection_mask);

  return NO_ERR;
}

