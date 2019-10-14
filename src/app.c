#include <SDL2/SDL.h>
#include <setjmp.h>
#include <unistd.h>
#include "dsp/dsp.h"
#include "midi.h"
#include "app.h"

App app;


static void tick_callback(void) {
  app_fe_push();
  app_do_string("(if on-tick (on-tick))");
  app_fe_pop();
}


static void midi_callback(MidiMessage msg) {
  const char *type;;
  switch (midi_type(msg)) {
    case MIDI_NOTEON        : type = "note-on";  break;
    case MIDI_NOTEOFF       : type = "note-off"; break;
    case MIDI_CONTROLCHANGE : type = "cc";       break;
    default: return;
  }
  char str[128];
  int chan = midi_channel(msg);
  sprintf(str, "(if on-midi (on-midi '%s %d %d %d))", type, chan, msg.b[1], msg.b[2]);

  app_fe_push();
  app_do_string(str);
  app_fe_pop();
}


static mu_Container console_win;


void app_init(int argc, char **argv) {
  if (argc > 1) { expect( chdir(argv[1]) == 0 ); }

  SDL_Init(SDL_INIT_EVERYTHING);
#if _WIN32
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif
  app.fe_lock = SDL_CreateMutex();

  /* init ui */
  app.mu_ctx = ui_init(APP_TITLE);
  app.mu_ctx->style->title_height = 22;
  app.mu_ctx->style->padding = 3;
  app.mu_ctx->style->size.y = 14;
  app.mu_ctx->style->colors[MU_COLOR_WINDOWBG    ] = mu_color(20, 20, 20, 255);
  app.mu_ctx->style->colors[MU_COLOR_TEXT        ] = mu_color(216, 210, 190, 255);
  app.mu_ctx->style->colors[MU_COLOR_BASE        ] = mu_color(0, 0, 0, 0);
  app.mu_ctx->style->colors[MU_COLOR_BASEHOVER   ] = mu_color(255, 255, 255, 20);
  app.mu_ctx->style->colors[MU_COLOR_BASEFOCUS   ] = mu_color(255, 255, 255, 30);
  app.mu_ctx->style->colors[MU_COLOR_BUTTON      ] = mu_color(0, 0, 0, 0);
  app.mu_ctx->style->colors[MU_COLOR_BUTTONHOVER ] = mu_color(255, 255, 255, 20);
  app.mu_ctx->style->colors[MU_COLOR_BUTTONFOCUS ] = mu_color(255, 255, 255, 28);
  app.mu_ctx->style->colors[MU_COLOR_TITLEBG     ] = mu_color(0, 0, 0, 0);
  app.mu_ctx->style->colors[MU_COLOR_TITLETEXT   ] = mu_color(255, 255, 255, 70);
  app.mu_ctx->style->colors[MU_COLOR_BORDER      ] = mu_color(65, 65, 65, 255);
  app.mu_ctx->style->colors[MU_COLOR_SCROLLBASE  ] = mu_color(0, 0, 0, 0);
  app.mu_ctx->style->colors[MU_COLOR_SCROLLTHUMB ] = mu_color(255, 255, 255, 20);

  /* init console window */
  mu_init_window(app.mu_ctx, &console_win, 0);
  console_win.rect = mu_rect(300, 40, 400, 230);
  console_win.zindex = 0xffffff;
  console_win.open = false;

  /* init `fe` */
  int bytes = 1024 * 256;
  app.fe_ctx = fe_open(malloc(bytes), bytes);

  extern fex_Reg api_core []; fex_register_funcs(app.fe_ctx, api_core );
  extern fex_Reg api_ui   []; fex_register_funcs(app.fe_ctx, api_ui   );
  extern fex_Reg api_dsp  []; fex_register_funcs(app.fe_ctx, api_dsp  );

  /* init dsp and midi */
  dsp_init(tick_callback);
  midi_init(midi_callback);

  /* init scripts */
  app_fe_push();
  app_do_file("main.fe");
  app_fe_pop();
}



static void console_window(mu_Context *ctx) {
  /* toggle console */
  bool just_opened_console = false;
  if (ui_key_pressed("escape")) {
    console_win.open ^= true;
    just_opened_console = true;
  }

  if (mu_begin_window(ctx, &console_win, "Console")) {
    /* output text panel */
    static mu_Container panel;
    mu_layout_row(ctx, 1, (int[]) { -1 }, -25);
    mu_begin_panel(ctx, &panel);
    mu_layout_row(ctx, 1, (int[]) { -1 }, -1);
    mu_text(ctx, app.log.buf);
    mu_end_panel(ctx);
    if (app.log.updated) {
      panel.scroll.y = panel.content_size.y;
      app.log.updated = false;
    }

    /* input textbox + submit button */
    static char buf[128];
    bool submitted = false;
    mu_layout_row(ctx, 2, (int[]) { -70, -1 }, 0);
    if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
      mu_set_focus(ctx, ctx->last_id);
      submitted = true;
    }
    if (just_opened_console) { mu_set_focus(ctx, ctx->last_id); }
    if (mu_button(ctx, "Submit")) { submitted = true; }
    if (submitted) {
      char buf2[140];
      sprintf(buf2, "> %s", buf);
      app_log(buf2);
      app_fe_push();
      fe_Object *obj = app_do_string(buf);
      if (obj) {
        fe_tostring(app.fe_ctx, obj, buf, sizeof(buf));
        app_log(buf);
      }
      app_fe_pop();
      buf[0] = '\0';
    }

    mu_end_window(ctx);
  }
}


static void process_frame(mu_Context *ctx) {
  static mu_Container win;
  const int opt = MU_OPT_NOTITLE | MU_OPT_NOFRAME | MU_OPT_AUTOSIZE;

  if (mu_begin_window_ex(app.mu_ctx, &win, "Main", opt)) {
    app_fe_push();
    app_do_string("(if on-frame (on-frame))");
    app_fe_pop();
    mu_end_window(app.mu_ctx);
  }

  int w, h;
  r_get_size(&w, &h);

  /* make window large enough to fit all content */
  if (w < win.rect.w || h < win.rect.h) {
    r_set_size(win.rect.w + 60, win.rect.h + 60);
  }

  /* center content in window */
  win.rect.x = (w - win.rect.w) / 2;
  win.rect.y = (h - win.rect.h) / 2;

  console_window(ctx);

  /* fullscreen toggle */
  if (ui_key_pressed("f11")) {
    static bool fullscreen = false;
    fullscreen ^= true;
    r_set_fullscreen(fullscreen);
  }
}


void app_run(void) {
  /* main loop */
  for (;;) {
    ui_begin_frame(app.mu_ctx);
    process_frame(app.mu_ctx);
    ui_end_frame(app.mu_ctx);
  }
}


void app_log(const char *str) {
  printf("%s\n", str);
  int len = strlen(str) + 2;
  expect(len < sizeof(app.log.buf));
  if (app.log.idx + len >= sizeof(app.log.buf)) {
    memmove(app.log.buf, app.log.buf + len, app.log.idx - len);
    app.log.idx -= len;
  }
  const char *fmt = (app.log.idx == 0) ? "%s" : "\n%s";
  app.log.idx += sprintf(&app.log.buf[app.log.idx], fmt, str);
  app.log.updated = true;
}


void app_log_error(const char *str) {
  console_win.open = true;
  app_log(str);

}


static jmp_buf error_buf;

static void error_handler(fe_Context *ctx, const char *msg, fe_Object *cl) {
  char buf[1024];
  char buf2[64];
  sprintf(buf, "error: %s", msg);
  app_log_error(buf);
  while (!fe_isnil(ctx, cl)) {
    fe_tostring(ctx, fe_car(ctx, cl), buf2, sizeof(buf2));
    sprintf(buf, "=> %s", buf2);
    app_log(buf);
    cl = fe_cdr(ctx, cl);
  }
  longjmp(error_buf, 1);
}


static int gc = 0;

void app_fe_push(void) {
  SDL_LockMutex(app.fe_lock);
  expect(gc == 0);
  gc = fe_savegc(app.fe_ctx);
}


void app_fe_pop(void) {
  fe_restoregc(app.fe_ctx, gc);
  gc = 0;
  SDL_UnlockMutex(app.fe_lock);
}


static fe_Object* do_(
  fe_Object* (*fn)(fe_Context*, const char *str),
  const char *str, const char *err
) {
  fe_Object *res = NULL;
  fe_ErrorFn oldfn = fe_handlers(app.fe_ctx)->error;
  fe_handlers(app.fe_ctx)->error = error_handler;
  if (setjmp(error_buf) == 0) {
    res = fn(app.fe_ctx, str);
    if (!res) { fe_error(app.fe_ctx, err); }
  }
  fe_handlers(app.fe_ctx)->error = oldfn;
  return res;
}


fe_Object* app_do_string(const char *str) {
  expect(gc);
  return do_(fex_do_string, str, "failed to do string");
}


fe_Object* app_do_file(const char *filename) {
  expect(gc);
  char err_buf[128];
  sprintf(err_buf, "failed to do file '%.64s'", filename);
  return do_(fex_do_file, filename, err_buf);
}
