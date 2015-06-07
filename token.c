#include "token.h"
#include <stdlib.h>
#include "str.h"

int tokens_init = 0;
void initTokens()
{
  if(tokens_init) return;

  //for brevety
  const int *ls = query_operation_token_oo_lvls;
  char ***lls = &query_operation_tokens_of_oo_lvl[0];

  int c  = 0;
  int si = 0;
  int ci = 0;
  for(c = 0; c < QUERY_NUM_OO_LEVELS; c++)
  {
    while(ls[ci] == c) ci++;

    lls[ls[si]] = malloc(((ci-si)+1)*sizeof(const char *));
    for(int j = 0; j < ci-si; j++)
      lls[ls[si]][j] = (char *)query_operation_tokens[si+j];
    lls[ls[si]][ci-si] = 0;

    si = ci;
  }

  tokens_init = 1;
}

const char *query_init_tokens[] =
{
  "new",
  "copy",
  "blank"
};
const char *query_src_tokens[] =
{
  "in",
  "out",
  ""
};
const char *query_operation_tokens[] =
{
  "or",
  "and",
  "not",
  "=",
  "!=",
  "<",
  "<=",
  ">=",
  ">",
  "-",
  "+",
  "/",
  "*",
  "%",
  ""
};
const int QUERY_NUM_OO_LEVELS = 8;
const int query_operation_token_oo_lvls[] = //manually keep in sync!
{
  0, //"or",
  1, //"and",
  2, //"not",
  3, //"=",
  3, //"!=",
  4, //"<",
  4, //"<=",
  4, //">=",
  4, //">",
  5, //"-",
  5, //"+",
  6, //"/",
  6, //"*",
  7, //"%",
  0  //""
};
char **query_operation_tokens_of_oo_lvl[QUERY_NUM_OO_LEVELS];
const char *query_property_tokens[] =
{
  "row",
  "col",
  "r",
  "g",
  "b",
  "a",
  ""
};

int isTokenType(char *t, char **type)
{
  int i = 0;
  while(type[i] != 0)
  {
    if(cmpLower(t,type[i]) == 0) return 1;
    i++;
  }
  return 0;
}

int isTokenLevel(char *t, int l)
{
  return isTokenType(t, query_operation_tokens_of_oo_lvl[l]);
}

int readToken(char *s, int offset, char *buff)
{
  int buff_i = 0;
  int str_i = 0;
  char c;

  c = '0'; //something other than null
  while(c != '\0') //blow past leading whitespace
  {
    c = s[offset+str_i];
    switch(c)
    {
      //whitespace
      case ' ':
      case '\t':
      case '\n':
        str_i++;
        break;
      default:
        c = '\0';
        break;
    }
  }
  c = '0'; //something other than null
  while(c != '\0')
  {
    c = s[offset+str_i];
    switch(c)
    {
      //EOS
      case '\0':
      {
        buff[buff_i] = '\0';
        c = '\0';
      }
        break;
      //whitespace
      case ' ':
      case '\t':
      case '\n':
      {
        buff[buff_i] = '\0';
        c = '\0';
      }
        break;
      //delimeter
      case ';':
      case '.':
      case ',':
      case '(':
      case ')':
      {
        if(buff_i == 0)
        {
          buff[buff_i++] = c;
          str_i++;
        }
        buff[buff_i] = '\0';
        c = '\0';
      }
        break;
      //ambiguous delimeter
      case '=':
      case '+':
      case '-':
      case '>':
      case '<':
      case '!':
      {
        if(buff_i == 0)
        {
          buff[buff_i++] = c;
          str_i++;
          if(s[offset+str_i+1] == '=')
          {
            buff[buff_i++] = '=';
            str_i++;
          }
        }
        buff[buff_i] = '\0';
        c = '\0';
      }
        break;
      default:
        buff[buff_i++] = c;
        str_i++;
        break;
    }
  }
  c = '0'; //some non-null
  while(c != '\0') //blow past ending whitespace
  {
    c = s[offset+str_i];
    switch(c)
    {
      //whitespace
      case ' ':
      case '\t':
      case '\n':
        str_i++;
        break;
      default:
        c = '\0';
        break;
    }
  }
  return str_i;
}

int closingParen(char *q, int s, int e)
{
  tokinit;

  tok;
  if(!(teq("("))) return -1;//err("Expected '('");

  int n_parens = 1;
  while(n_parens > 0 && o < e)
  {
    commit;
    tok;
    if(teq("(")) n_parens++;
    if(teq(")")) n_parens--;
  }
  return o-s;
}

int tokenAfterParenExpress(char *q, int s, int e)
{
  tokinit;

  l = closingParen(q,o,e);
  commit;
  tok;
  if(!(teq(")"))) return -1;//err("Expected ')'");
  commit;

  return o-s;
}

int lastTokenLevelInRange(char *q, int s, int e, int level)
{
  tokinit;

  int last = -1;

  while(o < e)
  {
    tok;
    if(teq("("))
    {
      l = tokenAfterParenExpress(q,o,e);
      commit;
    }
    else
    {
      if(isTokenType(token, query_operation_tokens_of_oo_lvl[level]))
      last = o;
      commit;
    }
  }

  return last-s;
}

