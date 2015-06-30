#ifndef TOKEN_H
#define TOKEN_H

extern int tokens_init;
extern const char *query_operation_tokens[];
enum { QUERY_NUM_OO_LEVELS = 9 }; //ugh
extern const int query_operation_token_oo_lvls[];
extern char **query_operation_tokens_of_oo_lvl[];
extern const char *query_property_tokens[];

void initTokens();
int isTokenType(char *t, char **type);
int isTokenLevel(char *t, int l);
int readToken(char *s, int offset, char *buff);

#define tokinit int o = s; int l = 0; char token[256];
#define basictokinit int o = s; int l = 0;
#define teq(s) (cmpLower(token,s) == 0)
#define tok (l = readToken(q,o,token))
#define commit (o += l)

int closingParen(char *q, int s, int e);
int tokenAfterParenExpress(char *q, int s, int e);
int lastTokenLevelInRange(char *q, int s, int e, int level);

#endif

