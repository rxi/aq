#include <math.h>
#include <SDL2/SDL.h>
#include "midi.h"
#include "fex.h"
#include "app.h"


static fe_Object* f_exit(fe_Context *ctx, fe_Object *arg) {
  exit(EXIT_SUCCESS);
  return NULL; /* never reached */
}


static fe_Object* f_echo(fe_Context *ctx, fe_Object *arg) {
  char buf[1024];
  int n = 0;
  while (!fe_isnil(ctx, arg)) {
    n += fe_tostring(ctx, fe_nextarg(ctx, &arg), &buf[n], sizeof(buf) - n);
  }
  app_log(buf);
  return fe_bool(ctx, false);
}


static fe_Object* f_error(fe_Context *ctx, fe_Object *arg) {
  char str[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), str, sizeof(str));
  fe_error(ctx, str);
  return NULL; /* never reached */
}


static fe_Object* f_clock(fe_Context *ctx, fe_Object *arg) {
  return fe_number(ctx, SDL_GetTicks() / 1000.);
}


static fe_Object* f_rand(fe_Context *ctx, fe_Object *arg) {
  if (!fe_isnil(ctx, arg)) {
    int n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
    if (n == 0) { return fe_number(ctx, 0); }
    return fe_number(ctx, rand() % n);
  }
  return fe_number(ctx, rand() / (float) RAND_MAX);
}


static fe_Object* f_floor(fe_Context *ctx, fe_Object *arg) {
  fe_Number n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  return fe_number(ctx, floor(n));
}


static fe_Object* f_mod(fe_Context *ctx, fe_Object *arg) {
  int a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  int b = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  if (b == 0) { fe_error(ctx, "expected non-zero divisor"); }
  return fe_number(ctx, a % b);
}


static fe_Object* f_pow(fe_Context *ctx, fe_Object *arg) {
  float a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  float b = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  return fe_number(ctx, pow(a, b));
}


static fe_Object* f_string(fe_Context *ctx, fe_Object *arg) {
  char buf[1024];
  int n = 0;
  while (!fe_isnil(ctx, arg)) {
    n += fe_tostring(ctx, fe_nextarg(ctx, &arg), &buf[n], sizeof(buf) - n);
  }
  return fe_string(ctx, buf);
}


static fe_Object* f_read(fe_Context *ctx, fe_Object *arg) {
  char filename[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), filename, sizeof(filename));
  FILE *fp = fopen(filename, "rb");
  if (!fp) { return fe_bool(ctx, false); }
  fe_Object *res = fe_readfp(ctx, fp);
  fclose(fp);
  return res;
}


static void writefp(fe_Context *ctx, void *udata, char chr) {
  fputc(chr, udata);
}


static fe_Object* f_write(fe_Context *ctx, fe_Object *arg) {
  char filename[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), filename, sizeof(filename));
  fe_Object *obj = fe_nextarg(ctx, &arg);
  FILE *fp = fopen(filename, "wb");
  if (!fp) { return fe_bool(ctx, false); }
  fe_write(ctx, obj, writefp, fp, true);
  fclose(fp);
  return fe_bool(ctx, true);
}


static fe_Object* f_do_file(fe_Context *ctx, fe_Object *arg) {
  char filename[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), filename, sizeof(filename));
  fe_Object *res = fex_do_file(ctx, filename);
  return res ? res : fe_bool(ctx, false);
}


static fe_Object* f_send_midi(fe_Context *ctx, fe_Object *arg) {
  const char* type_strings[] = { "note-on", "note-off", "cc", NULL };
  const int types[] = { MIDI_NOTEON, MIDI_NOTEOFF, MIDI_CONTROLCHANGE };
  MidiMessage msg;
  char type[32];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), type, sizeof(type));
  int chan = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  msg.b[1] = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  msg.b[2] = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  int idx = string_to_enum(type_strings, type);
  if (idx < 0) { fe_error(ctx, "invalid midi type"); }
  msg.status = types[idx] | chan;
  midi_send(msg);
  return fe_bool(ctx, false);
}



fex_Reg api_core[] = {
  { "exit",      f_exit      },
  { "echo",      f_echo      },
  { "error",     f_error     },
  { "clock",     f_clock     },
  { "rand",      f_rand      },
  { "floor",     f_floor     },
  { "mod",       f_mod       },
  { "pow",       f_pow       },
  { "string",    f_string    },
  { "read",      f_read      },
  { "write",     f_write     },
  { "do-file",   f_do_file   },
  { "send-midi", f_send_midi },
  {},
};
