#ifndef _STR_H_
#define _STR_H_

#include "str.h"

/*
this is a crippled string class for simplification purposes-
NOT INTENDED FOR RE-USE
*/

int strLen(char *s)
{
  int i = 0;
  while(s[i] != '\0') i++;
  return i;
}
int charPos(char *s, char c, int o)
{
  while(s[o] != c && s[o] != '\0')
    o++;
  if(s[o] == '\0') return -1;
  return o;
}
char toLower(char c)
{
  if(c >= 'A' && c <= 'Z')
    return c - ('A'-'a');
  return c;
}
int cmp(char *a, char *b)
{
  int i = 0;
  int d;
  while(a[i] != '\0' && b[i] != '\0')
  {
    d = a[i]-b[i];
    if(d != 0) return d;
    i++;
  }
  if(a[i] == b[i]) return 0;
  if(a[i] == '\0') return -1;
  if(b[i] == '\0') return 1;
  return 0; //shut up compiler
}
int cmpLower(char *a, char *b)
{
  int i = 0;
  char ca;
  char cb;
  int d;
  while(a[i] != '\0' && b[i] != '\0')
  {
    ca = toLower(a[i]);
    cb = toLower(b[i]);
    d = ca-cb;
    if(d != 0) return d;
    i++;
  }
  if(a[i] == b[i]) return 0;
  if(a[i] == '\0') return -1;
  if(b[i] == '\0') return 1;
  return 0; //shut up compiler
}
int intFromDec(char *s, int *d)
{
  int i = 0;
  *d = 0;
  while(s[i] != '\0')
  {
    if(s[i] < '0' || s[i] > '9') return 0;
    *d *= 10;
    *d += s[i]-'0';
    i++;
  }
  return 1;
}

#endif

