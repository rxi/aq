#include "../node.h"

static const char *mode_strings[] = { "phase", "sine", "saw", "pulse", "noise", NULL };
enum { PHASE, SINE, SAW, PULSE, NOISE };

typedef struct {
  Node node;
  int mode;
  double autophase;
  NodePort phase, freq; /* inlets */
  NodePort out;         /* outlets */
} OscNode;


static void update_phase(OscNode *n) {
  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    n->autophase += fabs(n->freq.buf[i]) * NODE_SAMPLETIME;
    if (n->autophase >= 1.0) { n->autophase -= floor(n->autophase); }
    n->phase.buf[i] = n->autophase;
  }
}


static void process(Node *node) {
  OscNode *n = (OscNode*) node;

  /* auto-update phase if we don't have links to the phase inlet */
  if (n->phase.link_count == 0) {
    update_phase(n);
  }

  /* write oscillator output */
  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    float phase = clampf(n->phase.buf[i], 0.0, 1.0);
    switch (n->mode) {
      case PHASE : n->out.buf[i] = phase;                                   break;
      case SINE  : n->out.buf[i] = sin(phase * 3.141592 * 2);               break;
      case SAW   : n->out.buf[i] = 1.0 - 2.0 * phase;                       break;
      case PULSE : n->out.buf[i] = phase < 0.5 ? -1.0 : 1.0;                break;
      case NOISE : n->out.buf[i] = 1.0 - 2.0 * (rand() / (float) RAND_MAX); break;
    }
  }

  /* send output */
  node_process(node);
}


static int receive(Node *node, const char *msg, char *err) {
  OscNode *n = (OscNode*) node;
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


Node* new_osc_node(void) {
  OscNode *node = calloc(1, sizeof(OscNode));

  static const char *inlets[] = { "phase", "freq", NULL };
  static const char *outlets[] = { "out", NULL };

  static NodeInfo info = {
    .name = "osc",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, &node->phase, &node->out);
  node_set(&node->node, "freq", 440.0);
  node->mode = SINE;

  return &node->node;
}
