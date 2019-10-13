#include "../node.h"

static const char *op_strings[] = { "+", "*", "/", "-", "^", "min", "max", NULL };
enum { ADD, MUL, DIV, SUB, POW, MIN, MAX, SET };

#define MAX_OPS 16

typedef struct { int op, inlet; float value; } Op;

typedef struct {
  Node node;
  Op ops[MAX_OPS];
  int op_count;
  NodePort in, in2, in3; /* inlets */
  NodePort out;          /* outlets */
} MathNode;


#define set(a, b) (b)
#define add(a, b) ((a) + (b))
#define sub(a, b) ((a) - (b))
#define mul(a, b) ((a) * (b))
#define div(a, b) ((a) / (b))

#define op_loop(f)                                \
  if (op.inlet >= 0) {                            \
    float *buf = node->inlets[op.inlet].buf;      \
    for (int i = 0; i < NODE_BUFFER_SIZE; i++) {  \
      n->out.buf[i] = f(n->out.buf[i], buf[i]);   \
    }                                             \
  } else {                                        \
    for (int i = 0; i < NODE_BUFFER_SIZE; i++) {  \
      n->out.buf[i] = f(n->out.buf[i], op.value); \
    }                                             \
  }

static void process(Node *node) {
  MathNode *n = (MathNode*) node;

  for (int j = 0; j < n->op_count; j++) {
    const Op op = n->ops[j];
    switch (op.op) {
      case SET : op_loop(set);  break;
      case ADD : op_loop(add);  break;
      case SUB : op_loop(sub);  break;
      case MUL : op_loop(mul);  break;
      case DIV : op_loop(div);  break;
      case POW : op_loop(pow);  break;
      case MIN : op_loop(minf); break;
      case MAX : op_loop(maxf); break;
    }
  }

  /* send output */
  node_process(node);
}


static int push_op(MathNode *n, int op_enum, char *val, char *err) {
  Op *op = &n->ops[n->op_count];
  op->op = op_enum;
  op->inlet = string_to_enum(n->node.info->inlets, val);

  if (op->inlet < 0) {
    char *endptr;
    op->value = strtod(val, &endptr);
    if (endptr == val) {
      sprintf(err, "expected inlet or number, got '%s'", val); return -1;
    }
  }

  if (n->op_count++ >= MAX_OPS) {
    sprintf(err, "too many operations"); return -1;
  }
  return 0;
}


static int receive(Node *node, const char *msg, char *err) {
  MathNode *n = (MathNode*) node;

  int i = 0;
  sscanf(msg, "set%n", &i);
  if (i == 0) { sprintf(err, "bad command"); return -1; }
  msg += i;

  char opstr[8], valstr[32] = "";
  n->op_count = 0;

  sscanf(msg, "%31s%n", valstr, &i);
  if (push_op(n, SET, valstr, err)) { return -1; }
  msg += i;

  while (sscanf(msg, "%7s %31s%n", opstr, valstr, &i) == 2) {
    int op_enum = string_to_enum(op_strings, opstr);
    if (op_enum < 0) { sprintf(err, "bad op '%s'", opstr); return -1; }
    if (push_op(n, op_enum, valstr, err)) { return -1; }
    msg += i;
  }
  if (!string_is_empty(msg)) {
    sprintf(err, "missing number or inlet"); return -1;
  }

  return 0;
}


Node* new_math_node(void) {
  MathNode *node = calloc(1, sizeof(MathNode));

  static const char *inlets[] = { "in", "in2", "in3", NULL };
  static const char *outlets[] = { "out", NULL };

  static NodeInfo info = {
    .name = "math",
    .inlets = inlets,
    .outlets = outlets,
  };

  static NodeVtable vtable = {
    .process = process,
    .receive = receive,
    .free = node_free,
  };

  node_init(&node->node, &info, &vtable, &node->in, &node->out);
  node->node.vtable->receive(&node->node, "set in", NULL);

  return &node->node;
}
