#ifndef ERR_H
#define ERR_H

#include <stdio.h> //sprintf
#define ERROR(...) ({ sprintf(err->info, ##__VA_ARGS__); return ERR; })

typedef enum
{
  ERR = 0,
  NO_ERR = 1
} ERR_EXISTS;

typedef struct
{
  long zero_pad;
  char info[1024];
} PixErr;

#endif

