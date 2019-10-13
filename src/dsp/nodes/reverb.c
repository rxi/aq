#include "lib/freeverb/freeverb.h"
#include "../node.h"

const char *cmd_strings[] = { "roomsize", "damp", "wet", "dry", "width", NULL };
enum { ROOMSIZE, DAMP, WET, DRY, WIDTH };

typedef struct {
  Node node;
  fv_Context fv;
  float buf[NODE_BUFFER_SIZE * 2];
  NodePort inl, inr;   /* inlets */
  NodePort outl, outr; /* outlets */
} ReverbNode;


static void process(Node *node) {
  ReverbNode *n = (ReverbNode*) node;

  /* copy inlets to buffer */
  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    n->buf[i*2+0] = n->inl.buf[i];
    n->buf[i*2+1] = n->inr.buf[i];
  }

  /* process */
  fv_process(&n->fv, n->buf, NODE_BUFFER_SIZE * 2);

  /* copy buffer to outlets */
  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    n->outl.buf[i] = n->buf[i*2+0];
    n->outr.buf[i] = n->buf[i*2+1];
  }

  /* send output */
  node_process(node);
}


static int receive(Node *node, const char *msg, char *err) {
  ReverbNode *n = (ReverbNode*) node;

  char cmd[16] = "";
  float val = 0;

  sscanf(msg, "%15s %f", cmd, &val);
  int prm = string_to_enum(cmd_strings, cmd);
  if (prm < 0) { sprintf(err, "bad command '%s'", cmd); return -1; }
  val = clampf(val, 0.0, 1.0);

  switch (prm) {
    case ROOMSIZE : fv_set_roomsize (&n->fv, val); break;
    case DAMP     : fv_set_damp     (&n->fv, val); break;
    case WET      : fv_set_wet      (&n->fv, val); break;
    case DRY      : fv_set_dry      (&n->fv, val); break;
    case WIDTH    : fv_set_width    (&n->fv, val); break;
  }

  return 0;
}


Node* new_reverb_node(void) {
  ReverbNode *node = calloc(1, sizeof(ReverbNode));

  static const char *inlets[] = { "left", "right", NULL };
  static const char *outlets[] = { "left", "right", NULL };

  static NodeInfo info = {
    .name = "reverb",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, &node->inl, &node->outl);
  fv_init(&node->fv);
  fv_set_samplerate(&node->fv, NODE_SAMPLERATE);

  return &node->node;
}
