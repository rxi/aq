#include "dsp/dsp.h"
#include "app.h"


static fe_Object* f_key_down(fe_Context *ctx, fe_Object *arg) {
  char name[32];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), name, sizeof(name));
  return fe_bool(ctx, ui_key_down(name));
}


static fe_Object* f_key_pressed(fe_Context *ctx, fe_Object *arg) {
  char name[32];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), name, sizeof(name));
  return fe_bool(ctx, ui_key_pressed(name));
}


static fe_Object* f_set_color(fe_Context *ctx, fe_Object *arg) {
  mu_Color c = { .a = 255 };
  int idx = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  c.r     = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  c.g     = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  c.b     = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  switch (idx) {
    case 0: app.mu_ctx->style->colors[MU_COLOR_WINDOWBG ] = c; break;
    case 1: app.mu_ctx->style->colors[MU_COLOR_BORDER   ] = c;
            app.mu_ctx->style->colors[MU_COLOR_TITLETEXT] = c; break;
    case 2: app.mu_ctx->style->colors[MU_COLOR_TEXT     ] = c; break;
    default: fe_error(ctx, "invalid color index");
  }
  return fe_bool(ctx, false);
}


static fe_Object* f_row(fe_Context *ctx, fe_Object *arg) {
  int widths[MU_MAX_WIDTHS];
  int count = 0;
  int height = 0;

  /* get widths */
  fe_Object *x = fe_nextarg(ctx, &arg);
  while (!fe_isnil(ctx, x)) {
    widths[count++] = fe_tonumber(ctx, fe_car(ctx, x));
    x = fe_cdr(ctx, x);
  }

  /* get height */
  if (!fe_isnil(ctx, arg)) {
    height = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  }

  mu_layout_row(app.mu_ctx, count, widths, height);
  return fe_bool(ctx, false);
}


static fe_Object *f_begin_column(fe_Context *ctx, fe_Object *arg) {
  mu_layout_begin_column(app.mu_ctx);
  return fe_bool(ctx, false);
}


static fe_Object *f_end_column(fe_Context *ctx, fe_Object *arg) {
  mu_layout_end_column(app.mu_ctx);
  return fe_bool(ctx, false);
}


static fe_Object *f_push_id(fe_Context *ctx, fe_Object *arg) {
  char str[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), str, sizeof(str));
  mu_push_id(app.mu_ctx, str, strlen(str));
  return fe_bool(ctx, false);
}


static fe_Object *f_pop_id(fe_Context *ctx, fe_Object *arg) {
  mu_pop_id(app.mu_ctx);
  return fe_bool(ctx, false);
}


static fe_Object *f_highlight(fe_Context *ctx, fe_Object *arg) {
  mu_Rect r = app.mu_ctx->last_rect;
  r.x -= 1; r.y -= 1;
  r.w += 2; r.h += 2;
  mu_draw_box(app.mu_ctx, r, app.mu_ctx->style->colors[MU_COLOR_TEXT]);
  return fe_bool(ctx, false);
}


static fe_Object* f_label(fe_Context *ctx, fe_Object *arg) {
  char label[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), label, sizeof(label));
  mu_label(app.mu_ctx, label);
  return fe_bool(ctx, false);
}


static fe_Object* f_button(fe_Context *ctx, fe_Object *arg) {
  char label[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), label, sizeof(label));
  int res = mu_button(app.mu_ctx, label);
  return fe_bool(ctx, res);
}


static fe_Object* f_slider(fe_Context *ctx, fe_Object *arg) {
  static float value;
  char id[128];
  float lo = 0.0, hi = 1.0;

  fe_tostring(ctx, fe_nextarg(ctx, &arg), id, sizeof(id));
  value = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  if (!fe_isnil(ctx, arg)) { lo = fe_tonumber(ctx, fe_nextarg(ctx, &arg)); }
  if (!fe_isnil(ctx, arg)) { hi = fe_tonumber(ctx, fe_nextarg(ctx, &arg)); }

  mu_push_id(app.mu_ctx, id, strlen(id));
  mu_slider(app.mu_ctx, &value, lo, hi);
  mu_pop_id(app.mu_ctx);

  return fe_number(ctx, value);
}


static fe_Object* f_number(fe_Context *ctx, fe_Object *arg) {
  static float value;
  char id[128];
  float step = 1.0;

  fe_tostring(ctx, fe_nextarg(ctx, &arg), id, sizeof(id));
  value = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  if (!fe_isnil(ctx, arg)) { step = fe_tonumber(ctx, fe_nextarg(ctx, &arg)); }

  mu_push_id(app.mu_ctx, id, strlen(id));
  mu_number(app.mu_ctx, &value, step);
  mu_pop_id(app.mu_ctx);

  return fe_number(ctx, value);
}


static fe_Object* f_meter(fe_Context *ctx, fe_Object *arg) {
  float val = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  val = clampf(val, 0.0, 1.0);
  mu_Rect r2, r1 = mu_layout_next(app.mu_ctx);
  r1.x -= 1; r1.w += 2;
  r1.y -= 1; r1.h += 2;

  if (r1.w >= r1.h) {
    /* horizontal */
    r1.y += r1.h / 2 - 1; r1.h = 1;
    r2 = r1;
    r2.w *= val;
  } else {
    /* vertical */
    r1.x += r1.w / 2 - 1; r1.w = 1;
    r2 = r1;
    r2.h *= val;
    r2.y += r1.h - r2.h;
  }

  mu_draw_rect(app.mu_ctx, r1, app.mu_ctx->style->colors[MU_COLOR_BORDER]);
  mu_draw_rect(app.mu_ctx, r2, app.mu_ctx->style->colors[MU_COLOR_TEXT]);

  return fe_bool(ctx, false);
}


static fe_Object* f_scope(fe_Context *ctx, fe_Object *arg) {
  char outlet[64];
  Node *node = dsp_get_node(fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), outlet, sizeof(outlet));
  int idx = string_to_enum(node->info->outlets, outlet);

  if (!node  ) { fe_error(ctx, "bad node id"); }
  if (idx < 0) { fe_error(ctx, "bad outlet");  }

  mu_Rect r = mu_layout_next(app.mu_ctx);
  app.mu_ctx->draw_frame(app.mu_ctx, r, MU_COLOR_BASE);

  float *buf = node->outlets[idx].buf;
  for (int i = 0; i < r.w; i++) {
    float p = i * (NODE_BUFFER_SIZE - 1) / (float) r.w;
    int n = p;
    float val = lerpf(buf[n], buf[n + 1], p - n);
    int h = clampf(fabs(val * r.h / 2), 1, r.h / 2);
    int y = r.h / 2 - (val < 0 ? h : 0);
    mu_Rect r2 = { r.x + i, r.y + y, 1, h };
    mu_draw_rect(app.mu_ctx, r2, app.mu_ctx->style->colors[MU_COLOR_TEXT]);
  }

  return fe_bool(ctx, false);
}


fex_Reg api_ui[] = {
  { "ui:key-down",      f_key_down      },
  { "ui:key-pressed",   f_key_pressed   },
  { "ui:set-color",     f_set_color     },
  { "ui:row",           f_row           },
  { "ui:begin-column",  f_begin_column  },
  { "ui:end-column",    f_end_column    },
  { "ui:push-id",       f_push_id       },
  { "ui:pop-id",        f_pop_id        },
  { "ui:highlight",     f_highlight     },
  { "ui:label",         f_label         },
  { "ui:button",        f_button        },
  { "ui:slider",        f_slider        },
  { "ui:number",        f_number        },
  { "ui:meter",         f_meter         },
  { "ui:scope",         f_scope         },
  {},
};
