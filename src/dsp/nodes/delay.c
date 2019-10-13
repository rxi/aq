#include "../node.h"

static const char *cmd_strings[] = { "wet", "dry", NULL };
enum { WET, DRY };

#define BUFFER_SIZE 65536
#define BUFFER_MASK (BUFFER_SIZE - 1)

typedef struct {
  Node node;
  int idx;
  float wet, dry;
  float buf[BUFFER_SIZE];
  NodePort in, time, feedback; /* inlets */
  NodePort out;                /* outlets */
} DelayNode;


static void process(Node *node) {
  DelayNode *n = (DelayNode*) node;

  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    /* read */
    double fidx = n->idx - fabs(n->time.buf[i]) * NODE_SAMPLERATE;
    float frac = fmod(fidx, 1.0);
    int idx1 = (int) fidx & BUFFER_MASK;
    int idx2 = (idx1 + 1) & BUFFER_MASK;
    float out = lerpf(n->buf[idx1], n->buf[idx2], frac);

    /* write */
    float in = n->in.buf[i];
    n->buf[n->idx] = in + out * n->feedback.buf[i];
    n->idx = (n->idx + 1) & BUFFER_MASK;

    /* output */
    n->out.buf[i] = out * n->wet + in * n->dry;
  }

  /* send output */
  node_process(node);
}


static int receive(Node *node, const char *msg, char *err) {
  DelayNode *n = (DelayNode*) node;

  char cmd[16] = "";
  float val = 0;

  sscanf(msg, "%15s %f", cmd, &val);
  int prm = string_to_enum(cmd_strings, cmd);
  if (prm < 0) { sprintf(err, "bad command '%s'", cmd); return -1; }
  val = clampf(val, 0.0, 1.0);

  switch (prm) {
    case WET : n->wet = val; break;
    case DRY : n->dry = val; break;
  }

  return 0;
}


Node* new_delay_node(void) {
  DelayNode *node = calloc(1, sizeof(DelayNode));

  static const char *inlets[] = { "in", "time", "feedback", NULL };
  static const char *outlets[] = { "out", NULL };

  static NodeInfo info = {
    .name = "delay",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, &node->in, &node->out);
  node->wet = 1.0;
  node->dry = 0.0;
  node_set(&node->node, "feedback", 0.5);
  node_set(&node->node, "time", 0.2);

  return &node->node;
}
