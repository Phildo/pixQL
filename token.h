#ifndef _TOKEN_H_
#define _TOKEN_H_

const char *init_tokens
[
  "NEW",
  "COPY",
  "BLANK"
];
const char *src_tokens
[
  "IN",
  "OUT",
  ""
];
const char *bool_operation_tokens
[
  "AND",
  "OR",
  "NOT",
  ""
];
const char *bool_expression_tokens
[
  "<",
  "<=",
  "=",
  "!=",
  ">=",
  ">",
  ""
];
const char *val_expression_tokens
[
  "+",
  "-",
  "*",
  "/",
  "%",
  ""
];
const char *property_tokens
[
  "row",
  "col",
  "r",
  "g",
  "b",
  "a",
  ""
];

int isTokenType(char *t, char **type)
{
  int i;
  while(*type[i] != '\0')
  {
    if(cmpLower(t,type[i]) == 0)
      return 1;
  }
  return 0;
}
int isMiddleBit(char *t)
{
  return
    isTokenType(t,bool_operation_tokens) ||
    isTokenType(t,bool_expression_tokens) ||
    isTokenType(t,val_expression_tokens);
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

/*
//defines assume:
int o = 0; //current offset
int l = 0; //length of read
char token[256];
*/
#define teq(s) (cmpLower(token,s) == 0)
#define tok (l = readToken(q,o,token))
#define commit (o += l)

#endif

