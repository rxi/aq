#include <SDL2/SDL.h>
#include "ui.h"
#include "renderer.h"


typedef struct { char name[32]; bool down, pressed; } KeyState;

static KeyState keys[16];
static      int key_count;


static KeyState* get_key_state(const char *name, bool create) {
  for (int i = 0; i < key_count; i++) {
    if (string_equal_nocase(keys[i].name, name)) {
      return &keys[i];
    }
  }
  if (!create) { return NULL; }
  strcpy(keys[key_count].name, name);
  return &keys[key_count++];
}


static void remove_key_state(const char *name) {
  KeyState *ks = get_key_state(name, false);
  if (ks) { *ks = keys[--key_count]; }
}


static const char button_map[256] = {
  [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
  [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
  [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
  [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
  [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};


static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
  return r_get_text_height();
}


mu_Context* ui_init(const char *title) {
  /* init renderer */
  r_init(title);

  /* init microui */
  mu_Context *ctx = malloc(sizeof(mu_Context));
  mu_init(ctx);
  ctx->text_width = text_width;
  ctx->text_height = text_height;
  return ctx;
}


static void apply_renderer_scale(int *x, int *y) {
  int scale = r_get_scale();
  *x /= scale;
  *y /= scale;
}


void ui_begin_frame(mu_Context *ctx) {
  /* reset key states */
  for (int i = 0; i < key_count; i++) {
    keys[i].pressed = false;
  }

  /* handle SDL events */
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_QUIT: exit(EXIT_SUCCESS); break;
      case SDL_MOUSEMOTION:
        apply_renderer_scale(&e.motion.x, &e.motion.y);
        mu_input_mousemove(ctx, e.motion.x, e.motion.y);
        break;
      case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
      case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
        int b = button_map[e.button.button & 0xff];
        apply_renderer_scale(&e.button.x, &e.button.y);
        if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
        if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
      }

      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        int c = key_map[e.key.keysym.sym & 0xff];

        if (e.type == SDL_KEYDOWN) {
          if (c) { mu_input_keydown(ctx, c); }
          KeyState *ks = get_key_state(SDL_GetKeyName(e.key.keysym.sym), true);
          ks->down = true;
          ks->pressed = true;

        } else if (e.type == SDL_KEYUP) {
          if (c) { mu_input_keyup(ctx, c); }
          remove_key_state(SDL_GetKeyName(e.key.keysym.sym));
        }
        break;
      }
    }
  }

  mu_begin(ctx);
}


void ui_end_frame(mu_Context *ctx) {
  mu_end(ctx);

  /* render */
  r_clear(ctx->style->colors[MU_COLOR_WINDOWBG]);

  mu_Command *cmd = NULL;
  while (mu_next_command(ctx, &cmd)) {
    switch (cmd->type) {
      case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
      case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
      case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
      case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
    }
  }

  r_present();
}


bool ui_key_down(const char *name) {
  KeyState *ks = get_key_state(name, false);
  return ks && ks->down;
}


bool ui_key_pressed(const char *name) {
  KeyState *ks = get_key_state(name, false);
  return ks && ks->pressed;
}
