#include "../node.h"


#define MAX_POINTS 64

typedef struct {
  float value, time;
} Point;

typedef struct {
  Node node;
  Point points[MAX_POINTS];
  int point_count;
  int point_idx, counter;
  double cur, step;
  bool active;
  NodePort out; /* outlets */
} LineNode;


static void handle_next_point(LineNode *n) {
  if (n->point_idx >= n->point_count) {
    n->active = false;
    return;
  }

  Point p = n->points[n->point_idx];
  n->step = (p.value - n->cur) * NODE_SAMPLETIME / p.time;
  n->counter = p.time * NODE_SAMPLERATE;
}


static void process(Node *node) {
  LineNode *n = (LineNode*) node;

  /* update */
  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    n->out.buf[i] = n->cur;
    if (!n->active) { continue; }

    n->cur += n->step;

    if (n->counter-- == 0) {
      n->cur = n->points[n->point_idx].value;
      n->point_idx++;
      handle_next_point(n);
    }
  }

  /* send output */
  node_process(node);
}


static int receive(Node *node, const char *msg, char *err) {
  LineNode *n = (LineNode*) node;

  int i = 0;
  sscanf(msg, "begin%n", &i);
  if (i == 0) { sprintf(err, "bad command"); return -1; }
  msg += i;

  float value, time;
  n->active = true;
  n->point_count = 0;
  n->point_idx = 0;

  while (sscanf(msg, "%f %f%n", &value, &time, &i) == 2) {
    n->points[n->point_count].value = value;
    n->points[n->point_count].time = time;
    if (n->point_count++ >= MAX_POINTS) {
      sprintf(err, "too many points"); return -1;
    }
    msg += i;
  }
  if (!string_is_empty(msg)) {
    sprintf(err, "invalid or missing number"); return -1;
  }

  handle_next_point(n);
  return 0;
}


Node* new_line_node(void) {
  LineNode *node = calloc(1, sizeof(LineNode));

  static const char *inlets[] = { NULL };
  static const char *outlets[] = { "out", NULL };

  static NodeInfo info = {
    .name = "line",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, NULL, &node->out);
  node->active = false;

  return &node->node;
}
