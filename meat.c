#include "meat.h"

#include <stdlib.h>
#include <math.h>

typedef enum
{
  NUMBER_TYPE_INVALID,
  NUMBER_TYPE_INT,
  NUMBER_TYPE_REAL,
} NUMBER_TYPE;
typedef struct
{
  NUMBER_TYPE type;
  uint8 b;
  int64 i;
  double r;
} Number;

static ERR_EXISTS evaluateValue(QueryValue *v, int col, int row, PixImg *target, PixImg *in, PixImg *out, Number *n, PixErr *err);
static ERR_EXISTS evaluateExpression(QueryExpression *qexp, int col, int row, PixImg *target, PixImg *in, PixImg *out, Number *n, PixErr *err);
static ERR_EXISTS evaluateOperation(QueryOperation *op, int col, int row, PixImg *target, PixImg *in, PixImg *out, PixErr *err);

static ERR_EXISTS evaluateConstant(QueryConstant *c, int col, int row, PixImg *target, PixImg *in, PixImg *out, Number *n, PixErr *err)
{
  switch(c->type)
  {
    case QUERY_CONSTANT_TYPE_WIDTH: n->type = NUMBER_TYPE_INT; n->i = target->width; break;
    case QUERY_CONSTANT_TYPE_HEIGHT: n->type = NUMBER_TYPE_INT; n->i = target->height; break;
    case QUERY_CONSTANT_TYPE_ROW: n->type = NUMBER_TYPE_INT; n->i = row; break;
    case QUERY_CONSTANT_TYPE_COL: n->type = NUMBER_TYPE_INT; n->i = col; break;
    case QUERY_CONSTANT_TYPE_NUMBER: n->type = NUMBER_TYPE_INT; n->i = c->value; break;
    default:
      ERROR("Invalid constant type encountered");
      break;
  }
  return NO_ERR;
}

static ERR_EXISTS evaluateMember(QueryMember *m, int col, int row, PixImg *target, PixImg *in, PixImg *out, Number *n, PixErr *err)
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
      ERROR("Invalid target encountered");
      break;
  }

  Number na;
  if(m->row)
  {
    if(!evaluateExpression(m->row,col,row,target,in,out,&na,err)) return ERR;
    if(na.type != NUMBER_TYPE_INT) ERROR("Invalid row type");
    prow = na.i;
  }
  else prow = row;

  if(m->col)
  {
    if(!evaluateExpression(m->col,col,row,target,in,out,&na,err)) return ERR;
    if(na.type != NUMBER_TYPE_INT) ERROR("Invalid col type");
    pcol = na.i;
  }
  else pcol = col;

  if(prow < 0) prow = 0; if(prow > t->width-1) prow = t->width-1;
  if(pcol < 0) pcol = 0; if(pcol > t->width-1) pcol = t->width-1;

  Pix *p;
  switch(m->type)
  {
    case QUERY_MEMBER_TYPE_COLOR:
      p = pixAt(t,pcol,prow);
      n->type = NUMBER_TYPE_INT;
      n->i = (p->r << 24) + (p->g << 16) + (p->b << 8) + (p->a << 0);
      break;
    case QUERY_MEMBER_TYPE_R:   n->type = NUMBER_TYPE_INT; n->i = pixAt(t,pcol,prow)->r; break;
    case QUERY_MEMBER_TYPE_G:   n->type = NUMBER_TYPE_INT; n->i = pixAt(t,pcol,prow)->g; break;
    case QUERY_MEMBER_TYPE_B:   n->type = NUMBER_TYPE_INT; n->i = pixAt(t,pcol,prow)->b; break;
    case QUERY_MEMBER_TYPE_A:   n->type = NUMBER_TYPE_INT; n->i = pixAt(t,pcol,prow)->a; break;
    case QUERY_MEMBER_TYPE_ROW: n->type = NUMBER_TYPE_INT; n->i = prow; break;
    case QUERY_MEMBER_TYPE_COL: n->type = NUMBER_TYPE_INT; n->i = pcol; break;
    default:
      ERROR("Invalid member type encountered");
      break;
  }
  return NO_ERR;
}

static ERR_EXISTS evaluateValue(QueryValue *v, int col, int row, PixImg *target, PixImg *in, PixImg *out, Number *n, PixErr *err)
{
  switch(v->type)
  {
    case QUERY_VALUE_TYPE_MEMBER:   if(!evaluateMember(&v->member,col,row,target,in,out,n,err))     return ERR; break;
    case QUERY_VALUE_TYPE_CONSTANT: if(!evaluateConstant(&v->constant,col,row,target,in,out,n,err)) return ERR; break;
    default: ERROR("Invalid value type encountered"); break;
  }
  return NO_ERR;
}

static ERR_EXISTS evaluateExpression(QueryExpression *qexp, int col, int row, PixImg *target, PixImg *in, PixImg *out, Number *n, PixErr *err)
{
  Number na, nb;
  if(qexp->a) if(!evaluateExpression(qexp->a,col,row,target,in,out,&na,err)) return ERR;
  if(qexp->b) if(!evaluateExpression(qexp->b,col,row,target,in,out,&nb,err)) return ERR;

  switch(qexp->type)
  {
    case QUERY_EXPRESSION_TYPE_OR:
    {
      if(na.type == NUMBER_TYPE_REAL || nb.type == NUMBER_TYPE_REAL) ERROR("OR of real unsupported (cast to int)");
      n->type = NUMBER_TYPE_INT; n->i = na.i || nb.i;
    }
      break;
    case QUERY_EXPRESSION_TYPE_AND:
    {
      if(na.type == NUMBER_TYPE_REAL || nb.type == NUMBER_TYPE_REAL) ERROR("AND of real unsupported (cast to int)");
      n->type = NUMBER_TYPE_INT; n->i = na.i && nb.i;
    }
      break;
    case QUERY_EXPRESSION_TYPE_NOT:
    {
      switch(na.type)
      {
        case NUMBER_TYPE_INT:  n->type = NUMBER_TYPE_INT; n->i = (na.i == 0);    break;
        case NUMBER_TYPE_REAL: n->type = NUMBER_TYPE_INT; n->i = (na.r == 0.0f); break;
        default: break; //shouldn't happen
      }
    }
      break;
    case QUERY_EXPRESSION_TYPE_EQ:
    {
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.i == nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = ((double)na.i == nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.r == (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = (na.r == nb.r); }
    }
      break;
    case QUERY_EXPRESSION_TYPE_NE:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.i != nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = ((double)na.i != nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.r != (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = (na.r != nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_LT:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.i < nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = ((double)na.i < nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.r < (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = (na.r < nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_LTE:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.i <= nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = ((double)na.i <= nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.r <= (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = (na.r <= nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_GTE:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.i >= nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = ((double)na.i >= nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.r >= (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = (na.r >= nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_GT:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.i > nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = ((double)na.i > nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = (na.r > (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = (na.r > nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_SUB:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT;  n->i = (na.i - nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = ((double)na.i - nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_REAL; n->r = (na.r - (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = (na.r - nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_ADD:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT;  n->i = (na.i + nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = ((double)na.i + nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_REAL; n->r = (na.r + (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = (na.r + nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_DIV:
      if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)
      {
        if((na.i/nb.i)*nb.i == na.i) //aka "if integer division is lossless"
        {
          n->type = NUMBER_TYPE_INT;
          n->i = (na.i / nb.i);
        }
        else
        {
          n->type = NUMBER_TYPE_REAL;
          n->r = ((float)na.i / (float)nb.i);
        }
      }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = ((double)na.i / nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_REAL; n->r = (na.r / (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = (na.r / nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_MUL:
           if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT;  n->i = (na.i * nb.i); }
      else if(na.type == NUMBER_TYPE_INT  && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = ((double)na.i * nb.r); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_REAL; n->r = (na.r * (double)nb.i); }
      else if(na.type == NUMBER_TYPE_REAL && nb.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = (na.r * nb.r); }
      break;
    case QUERY_EXPRESSION_TYPE_MOD:
      if(na.type == NUMBER_TYPE_REAL || nb.type == NUMBER_TYPE_REAL) ERROR("Modulus of real unsupported (cast to int)");
      n->type = NUMBER_TYPE_INT; n->i = (na.i % nb.i);
      break;
    case QUERY_EXPRESSION_TYPE_SIN:
      if(na.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_REAL; n->r = sin((double)na.i); }
      if(na.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = sin(na.r); }
      break;
    case QUERY_EXPRESSION_TYPE_COS:
      if(na.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_REAL; n->r = cos((double)na.i); }
      if(na.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = cos(na.r); }
      break;
    case QUERY_EXPRESSION_TYPE_TAN:
      if(na.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_REAL; n->r = tan((double)na.i); }
      if(na.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = tan(na.r); }
      break;
    case QUERY_EXPRESSION_TYPE_ABS:
      if(na.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT;  n->i = abs((int)na.i); }
      if(na.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = fabs(na.r); }
      break;
    case QUERY_EXPRESSION_TYPE_INT_CAST:
      if(na.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT; n->i = na.i; }
      if(na.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_INT; n->i = (int)na.r; }
      break;
    case QUERY_EXPRESSION_TYPE_NEG:
      if(na.type == NUMBER_TYPE_INT)  { n->type = NUMBER_TYPE_INT;  n->i = -1 * na.i; }
      if(na.type == NUMBER_TYPE_REAL) { n->type = NUMBER_TYPE_REAL; n->r = -1.0f * na.r; }
      break;
    case QUERY_EXPRESSION_TYPE_VALUE:
      if(!evaluateValue(&qexp->v,col,row,target,in,out,n,err)) return ERR;
      break;
    case QUERY_EXPRESSION_TYPE_INVALID:
    default:
      ERROR("Invalid query expression type");
      break;
  }
  return NO_ERR;
}

static ERR_EXISTS evaluateOperation(QueryOperation *op, int col, int row, PixImg *target, PixImg *in, PixImg *out, PixErr *err)
{
  PixImg *t;
  switch(op->operating)
  {
    case QUERY_TARGET_IN: t = in; break;
    case QUERY_TARGET_OUT: t = out; break;
    default:
      ERROR("Invalid target encountered");
      break;
  }

  Number rval;
  if(!evaluateExpression(&op->rval,col,row,t,in,out,&rval,err)) return ERR;

  QueryMember *lval = &op->lval;
  int prow = 0;
  int pcol = 0;

  Number na;
  if(lval->row)
  {
    if(!evaluateExpression(lval->row,col,row,t,in,out,&na,err)) return ERR;
    if(na.type != NUMBER_TYPE_INT) ERROR("Invalid row type");
    prow = na.i;
  }
  else prow = row;

  if(lval->col)
  {
    if(!evaluateExpression(lval->col,col,row,t,in,out,&na,err)) return ERR;
    if(na.type != NUMBER_TYPE_INT) ERROR("Invalid col type");
    pcol = na.i;
  }
  else pcol = col;

  if(prow < 0) prow = 0; if(prow > out->height-1) prow = out->height-1;
  if(pcol < 0) pcol = 0; if(pcol > out->width-1)  pcol = out->width-1;

  int v;
  int val;
  switch(lval->type)
  {
    case QUERY_MEMBER_TYPE_COLOR:
      if(rval.type == NUMBER_TYPE_REAL) ERROR("Invalid rvalue for setting color");
      val = rval.i;
      v = (val & 0xff000000) >> 24;
      pixAt(out,pcol,prow)->r = v;
      v = (val & 0x00ff0000) >> 16;
      pixAt(out,pcol,prow)->g = v;
      v = (val & 0x0000ff00) >> 8;
      pixAt(out,pcol,prow)->b = v;
      v = (val & 0x000000ff) >> 0;
      pixAt(out,pcol,prow)->a = v;
      break;
    case QUERY_MEMBER_TYPE_R:
      if(rval.type == NUMBER_TYPE_REAL) val = (int)rval.r;
      else                              val = rval.i;
      pixAt(out,pcol,prow)->r = val;
      break;
    case QUERY_MEMBER_TYPE_G:
      if(rval.type == NUMBER_TYPE_REAL) val = (int)rval.r;
      else                              val = rval.i;
      pixAt(out,pcol,prow)->g = val;
      break;
    case QUERY_MEMBER_TYPE_B:
      if(rval.type == NUMBER_TYPE_REAL) val = (int)rval.r;
      else                              val = rval.i;
      pixAt(out,pcol,prow)->b = val;
      break;
    case QUERY_MEMBER_TYPE_A:
      if(rval.type == NUMBER_TYPE_REAL) val = (int)rval.r;
      else                              val = rval.i;
      pixAt(out,pcol,prow)->a = val;
      break;
    case QUERY_MEMBER_TYPE_ROW:
    case QUERY_MEMBER_TYPE_COL:
      //can of worms
    default:
      ERROR("Invalid attribute for lvalue");
      break;
  }
  return NO_ERR;
}

ERR_EXISTS executeQuery(Query *query, PixImg *in_img, PixImg *out_img, PixErr *err)
{
  byte *selection_mask;

  QueryInit *init = &query->init;
  Number n;

  uint32 init_width;
  if(!evaluateExpression(&init->width,-1,-1,in_img,in_img,out_img,&n,err)) return ERR;
  if(n.type == NUMBER_TYPE_REAL) ERROR("Invalid width type");
  init_width = n.i;

  uint32 init_height;
  if(!evaluateExpression(&init->height,-1,-1,in_img,in_img,out_img,&n,err)) return ERR;
  if(n.type == NUMBER_TYPE_REAL) ERROR("Invalid height type");
  init_height = n.i;

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
        case QUERY_TARGET_IN:  default_selecting = in_img; break;
        case QUERY_TARGET_OUT: default_selecting = out_img; break;
        case QUERY_TARGET_FALLBACK:
        case QUERY_TARGET_INVALID:
        default:
          //error
          break;
      }

      for(int k = 0; k < out_img->height; k++)
      {
        for(int l = 0; l < out_img->width; l++)
        {
          if(!evaluateExpression(&s->exp,l,k,default_selecting,in_img,out_img,&n,err)) return ERR;

               if(n.type == NUMBER_TYPE_REAL && n.r != 0.0f) selection_mask[(k*out_img->width)+l] = 1;
          else if(n.type == NUMBER_TYPE_INT  && n.i != 0)    selection_mask[(k*out_img->width)+l] = 1;
        }
      }
    }

    for(int j = 0; j < p->n_operations; j++)
    {
      o = &p->operations[j];
      switch(o->operating)
      {
        case QUERY_TARGET_IN:  default_operating = in_img; break;
        case QUERY_TARGET_OUT: default_operating = out_img; break;
        case QUERY_TARGET_FALLBACK:
        case QUERY_TARGET_INVALID:
        default: ERROR("Invalid target"); break;
      }

      for(int k = 0; k < out_img->height; k++)
        for(int l = 0; l < out_img->width; l++)
          if(selection_mask[(k*out_img->width)+l])
            if(!evaluateOperation(o,l,k,default_operating,in_img,out_img,err)) return ERR;
    }
  }

  free(selection_mask);

  return NO_ERR;
}

