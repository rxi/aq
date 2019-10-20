#include "../node.h"

static const char *mode_strings[] = { "softclip", "hardclip", "foldback", "sine",  NULL };
enum { SOFTCLIP, HARDCLIP, FOLDBACK, SINE };

typedef struct {
  Node node;
  int mode;
  NodePort in, gain; /* inlets */
  NodePort out;      /* outlets */
} ShaperNode;


#define process_loop(f)                        \
  for (int i = 0; i < NODE_BUFFER_SIZE; i++) { \
    float in = n->in.buf[i] * n->gain.buf[i];  \
    n->out.buf[i] = f(in);                     \
  }

#define softclip(in) (in / (1.0 + fabs(in)))
#define hardclip(in) clampf(in, -1.0, 1.0)
#define foldback(in) fabs(fabs(fmod(in - 1.0, 4.0)) - 2.0) - 1.0

static void process(Node *node) {
  ShaperNode *n = (ShaperNode*) node;

  switch (n->mode) {
    case SOFTCLIP : process_loop(softclip); break;
    case HARDCLIP : process_loop(hardclip); break;
    case FOLDBACK : process_loop(foldback); break;
    case SINE     : process_loop(sin);      break;
  }

  /* send output */
  node_process(node);
}


static int receive(Node *node, const char *msg, char *err) {
  ShaperNode *n = (ShaperNode*) node;
  char buf[16];

  if (sscanf(msg, "mode %15s", buf)) {
    int idx = string_to_enum(mode_strings, buf);
    if (idx < 0) { sprintf(err, "bad mode '%s'", buf); return -1; }
    n->mode = idx;
  } else {
    sprintf(err, "bad command"); return -1;
  }

  return 0;
}


Node* new_shaper_node(void) {
  ShaperNode *node = calloc(1, sizeof(ShaperNode));

  static const char *inlets[] = { "in", "gain", NULL };
  static const char *outlets[] = { "out", NULL };

  static NodeInfo info = {
    .name = "shaper",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, &node->in, &node->out);
  node_set(&node->node, "gain", 1.0);

  return &node->node;
}
