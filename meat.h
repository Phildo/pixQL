#ifndef MEAT_H
#define MEAT_H

#include "pix.h"
#include "query.h"

ERR_EXISTS executeQuery(Query *query, PixImg *in_img, PixImg *out_img, PixErr *err);

#endif

