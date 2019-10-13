#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include "common.h"
#include "fex.h"
#include "ui.h"
#include "renderer.h"

#define APP_TITLE "aq"

typedef struct {
  mu_Context *mu_ctx;
  fe_Context *fe_ctx;
  SDL_mutex *fe_lock;
  struct { char buf[4096]; int idx; bool updated; } log;
} App;

extern App app;

void app_init(int argc, char **argv);
void app_run(void);
void app_log(const char *str);
void app_log_error(const char *str);
void app_fe_push(void);
void app_fe_pop(void);
fe_Object* app_do_string(const char *str);
fe_Object* app_do_file(const char *filename);

#endif
