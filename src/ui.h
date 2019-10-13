#ifndef UI_H
#define UI_H

#include "common.h"
#include "lib/microui/microui.h"

mu_Context* ui_init(const char *title);
void ui_begin_frame(mu_Context *ctx);
void ui_end_frame(mu_Context *ctx);
bool ui_key_down(const char *name);
bool ui_key_pressed(const char *name);

#endif
