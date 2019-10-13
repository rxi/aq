#include "dsp/dsp.h"
#include "app.h"


/* keep in sync with `node.h` error enums */
static const char *node_error_strings[] = {
  "success",
  "failure",
  "bad inlet",
  "bad outlet",
  "bad link",
  "max links exceeded",
};


static void check_node_error(fe_Context *ctx, int err) {
  if (err) {
    fe_error(ctx, node_error_strings[-err]);
  }
}


static Node* get_node(fe_Context *ctx, int id) {
  Node *node = dsp_get_node(id);
  if (node == NULL) {
    fe_error(ctx, "bad node id");
  }
  return node;
}


static fe_Object* f_set_tick(fe_Context *ctx, fe_Object *arg) {
  float n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  dsp_set_tick(n);
  return fe_bool(ctx, false);
}


static fe_Object* f_set_stream(fe_Context *ctx, fe_Object *arg) {
  char str[256];
  char *filename;
  if (!fe_isnil(ctx, arg)) {
    fe_tostring(ctx, fe_nextarg(ctx, &arg), str, sizeof(str));
    filename = str;
  } else {
    filename = NULL;
  }
  int err = dsp_set_stream(filename);
  if (err) { fe_error(ctx, "failed to open stream"); }
  return fe_bool(ctx, false);
}


static fe_Object* f_new(fe_Context *ctx, fe_Object *arg) {
  char name[128];
  fe_tostring(ctx, fe_nextarg(ctx, &arg), name, sizeof(name));
  int id = dsp_new_node(name);
  if (id < 0) { fe_error(ctx, "bad node name"); }
  return fe_number(ctx, id);
}


static fe_Object* f_destroy(fe_Context *ctx, fe_Object *arg) {
  int id = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  int err = dsp_destroy_node(id);
  if (err) { fe_error(ctx, "bad node id"); }
  return fe_bool(ctx, false);
}


static fe_Object* f_link(fe_Context *ctx, fe_Object *arg) {
  char inlet[64], outlet[64];
  Node *node1 = get_node(ctx, fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), outlet, sizeof(outlet));
  Node *node2 = get_node(ctx, fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), inlet, sizeof(inlet));

  check_node_error(ctx, node_link(node1, outlet, node2, inlet));
  return fe_bool(ctx, false);
}


static fe_Object* f_unlink(fe_Context *ctx, fe_Object *arg) {
  char inlet[64], outlet[64];
  Node *node1 = get_node(ctx, fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), outlet, sizeof(outlet));
  Node *node2 = get_node(ctx, fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), inlet, sizeof(inlet));

  check_node_error(ctx, node_link(node1, outlet, node2, inlet));
  return fe_bool(ctx, false);
}


static fe_Object* f_set(fe_Context *ctx, fe_Object *arg) {
  char inlet[64];
  Node *node = get_node(ctx, fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), inlet, sizeof(inlet));
  float value = fe_tonumber(ctx, fe_nextarg(ctx, &arg));

  check_node_error(ctx, node_set(node, inlet, value));
  return fe_bool(ctx, false);
}


static fe_Object* f_get(fe_Context *ctx, fe_Object *arg) {
  float res;
  char outlet[64];
  Node *node = get_node(ctx, fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), outlet, sizeof(outlet));
  check_node_error(ctx, node_get(node, outlet, &res));
  return fe_number(ctx, res);
}


static fe_Object* f_send(fe_Context *ctx, fe_Object *arg) {
  char str[1024];
  char err_buf[NODE_MAX_ERROR];
  Node *node = get_node(ctx, fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  fe_tostring(ctx, fe_nextarg(ctx, &arg), str, sizeof(str));
  int err = node->vtable->receive(node, str, err_buf);
  if (err) { fe_error(ctx, err_buf); }
  return fe_bool(ctx, false);
}


fex_Reg api_dsp[] = {
  { "dsp:set-tick",   f_set_tick   },
  { "dsp:set-stream", f_set_stream },
  { "dsp:new",        f_new        },
  { "dsp:destroy",    f_destroy    },
  { "dsp:link",       f_link       },
  { "dsp:unlink",     f_unlink     },
  { "dsp:set",        f_set        },
  { "dsp:get",        f_get        },
  { "dsp:send",       f_send       },
  {},
};
