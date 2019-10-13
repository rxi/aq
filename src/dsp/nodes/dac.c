#include "../node.h"


typedef struct {
  Node node;
  NodePort inl, inr;   /* inlets */
  NodePort outl, outr; /* outlets */
} DacNode;


static void process(Node *node) {
  DacNode *n = (DacNode*) node;

  /* copy inlet buffers to outlet buffers */
  memcpy(n->outl.buf, n->inl.buf, sizeof(n->outl.buf));
  memcpy(n->outr.buf, n->inr.buf, sizeof(n->outr.buf));

  /* send output */
  node_process(node);
}


Node* new_dac_node(void) {
  DacNode *node = calloc(1, sizeof(DacNode));

  static const char *inlets[] = { "left", "right", NULL };
  static const char *outlets[] = { "left", "right", NULL };

  static NodeInfo info = {
    .name = "dac",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = node_receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, &node->inl, &node->outl);
  return &node->node;
}
