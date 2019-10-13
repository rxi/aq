#ifndef FEX_H
#define FEX_H

#include "common.h"
#include "lib/fe/fe.h"

typedef struct { const char *name; fe_CFunc fn; } fex_Reg;

void fex_register_funcs(fe_Context *ctx, fex_Reg *t);
fe_Object* fex_read_string(fe_Context *ctx, const char *str);
fe_Object* fex_do_string(fe_Context *ctx, const char *str);
fe_Object* fex_do_file(fe_Context *ctx, const char *filename);

#endif
