#include "fex.h"


void fex_register_funcs(fe_Context *ctx, fex_Reg *t) {
  int gc = fe_savegc(ctx);
  for (int i = 0; t[i].name; i++) {
    fe_set(ctx, fe_symbol(ctx, t[i].name), fe_cfunc(ctx, t[i].fn));
    fe_restoregc(ctx, gc);
  }
}


static char read_str(fe_Context *ctx, void *udata) {
  char **p = udata;
  if (!**p) { return '\0'; }
  return *(*p)++;
}


fe_Object* fex_read_string(fe_Context *ctx, const char *str) {
  char *p = (char*) str;
  return fe_read(ctx, read_str, &p);
}


fe_Object* fex_do_string(fe_Context *ctx, const char *str) {
  char *p = (char*) str;
  fe_Object *obj = NULL;
  int gc = fe_savegc(ctx);
  for (;;) {
    fe_restoregc(ctx, gc);
    fe_Object *tmp = fe_read(ctx, read_str, &p);
    if (!tmp) { break; }
    obj = fe_eval(ctx, tmp);
  }
  if (obj) { fe_pushgc(ctx, obj); }
  return obj;
}


fe_Object* fex_do_file(fe_Context *ctx, const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) { return NULL; }
  fe_Object *obj = NULL;
  int gc = fe_savegc(ctx);
  for (;;) {
    fe_restoregc(ctx, gc);
    fe_Object* tmp = fe_readfp(ctx, fp);
    if (!tmp) { break; }
    obj = fe_eval(ctx, tmp);
  }
  fclose(fp);
  if (obj) { fe_pushgc(ctx, obj); }
  return obj;
}
