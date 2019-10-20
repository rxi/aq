#include "../node.h"

static const char *mode_strings[] = { "lowpass", "highpass", "bandpass", "notch", NULL };
enum { LOWPASS, HIGHPASS, BANDPASS, NOTCH };

typedef struct {
  Node node;
  int mode;
  float d1, d2;
  NodePort in, freq, q; /* inlets */
  NodePort out;         /* outlets */
} SvfNode;


static void process(Node *node) {
  SvfNode *n = (SvfNode*) node;
  const float passes = 3;

  float max_freq = NODE_SAMPLERATE * 0.130 * passes;
  float f1, q1, in, hp;
  float bp = n->d1;
  float lp = n->d2;

  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    q1 = 1.0 / maxf(n->q.buf[i], 0.5);
    f1 = minf(fabs(n->freq.buf[i]), max_freq) / passes;
    f1 = 2 * 3.141592 * f1 * NODE_SAMPLETIME;
    in = n->in.buf[i];

    for (int i = 0; i < passes; i++) {
      lp = lp + f1 * bp;
      hp = in - lp - q1 * bp;
      bp = f1 * hp + bp;
    }

    switch (n->mode) {
      case LOWPASS  : n->out.buf[i] = lp;      break;
      case HIGHPASS : n->out.buf[i] = hp;      break;
      case BANDPASS : n->out.buf[i] = bp;      break;
      case NOTCH    : n->out.buf[i] = hp + lp; break;
    }
  }

  n->d1 = bp;
  n->d2 = lp;

  /* send output */
  node_process(node);
}


static int receive(Node *node, const char *msg, char *err) {
  SvfNode *n = (SvfNode*) node;
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


Node* new_svf_node(void) {
  SvfNode *node = calloc(1, sizeof(SvfNode));

  static const char *inlets[] = { "in", "freq", "q", NULL };
  static const char *outlets[] = { "out", NULL };

  static NodeInfo info = {
    .name = "svf",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, &node->in, &node->out);
  node_set(&node->node, "freq", 440.0);
  node_set(&node->node, "q", 1.0);

  return &node->node;
}
