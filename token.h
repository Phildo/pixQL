#ifndef _TOKEN_H_
#define _TOKEN_H_

extern const char *query_init_tokens[];
extern const char *query_src_tokens[];
extern const char *query_operation_tokens[];
extern const int QUERY_NUM_OO_LEVELS;
extern const int query_operation_token_oo_lvls[];
extern char **query_operation_tokens_of_oo_lvl[];
extern const char *query_property_tokens[];

void initTokens();
int isTokenType(char *t, char **type);
int isTokenLevel(char *t, int l);
int readToken(char *s, int offset, char *buff);

#define tokinit int o = 0; int l = 0; char token[256];
#define teq(s) (cmpLower(token,s) == 0)
#define tok (l = readToken(q,o,token))
#define commit (o += l)

int closingParen(char *q, int s, int e);
int tokenAfterParenExpress(char *q, int s, int e);
int lastTokenLevelInRange(char *q, int s, int e, int level);

#endif

