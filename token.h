#ifndef _TOKEN_H_
#define _TOKEN_H_

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
  "and",
  "or",
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

void init()
{
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
}

int isTokenType(char *t, char **type)
{
  int i = 0;
  while(*type[i] != '\0')
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
  int i = 0;
  char c;
  while(1)
  {
    c = s[offset+i];
    switch(c)
    {
      //EOS
      case '\0':
      {
        buff[i] = '\0';
        return i;
      }
        break;
      //whitespace
      case ' ':
      case '\t':
      case '\n':
      {
        buff[i] = '\0';
        return i+1;
      }
        break;
      //delimeter
      case ';':
      case '.':
      case ',':
      case '(':
      case ')':
      {
        if(i == 0)
        {
          buff[i] = c;
          i++;
          buff[i] = '\0';
          return i;
        }
        else
        {
          buff[i] = '\0';
          return i;
        }
      }
        return i;
      //ambiguous delimeter
      case '=':
      case '+':
      case '-':
      case '>':
      case '<':
      case '!':
      {
        if(i == 0)
        {
          buff[i] = c;
          i++;
          if(s[offset+i] == '=')
          {
            buff[i] = '=';
            i++;
          }
          buff[i] = '\0';
          return i;
        }
        else
        {
          buff[i] = '\0';
          return i;
        }
      }
      default:
        buff[i] = c;
        i++;
        break;
    }
  }
  return 0; //to shut the compiler up
}

#define tokinit int o = 0; int l = 0; char token[256];
#define teq(s) (cmpLower(token,s) == 0)
#define tok (l = readToken(q,o,token))
#define commit (o += l)

int tokenAfterParenExpress(char *q, int s, int e)
{
  tokinit;
  o = s;

  tok;
  if(!(teq("("))) return -1;//err("Expected '('");
  commit;

  int n_parens = 1;
  while(n_parens > 0 && o < e)
  {
    tok;
    if(teq("(")) n_parens++;
    if(teq(")")) n_parens--;
    commit;
  }
  return o;
}

int lastTokenLevelInRange(char *q, int s, int e, int level)
{
  tokinit;
  o = s;

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

  return last;
}

#endif

