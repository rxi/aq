#ifndef NODE_H
#define NODE_H

#include <math.h>
#include "common.h"

#define NODE_SAMPLERATE  44100
#define NODE_SAMPLETIME  (1.0 / NODE_SAMPLERATE)
#define NODE_BUFFER_SIZE 64
#define NODE_MAX_LINKS   32
#define NODE_MAX_ERROR   128

enum {
  NODE_ESUCCESS   =  0,
  NODE_EFAILURE   = -1,
  NODE_EBADINLET  = -2,
  NODE_EBADOUTLET = -3,
  NODE_EBADLINK   = -4,
  NODE_EMAXLINKS  = -5,
};

typedef struct Node Node;
typedef Node* (*NodeConstructor)(void);

typedef struct { Node *node; int idx; } NodeLink;

typedef struct {
  float buf[NODE_BUFFER_SIZE];
  NodeLink links[NODE_MAX_LINKS];
  int link_count;
  bool replace;
} NodePort;

typedef struct {
  int (*receive)(Node *node, const char *str, char *err);
  void (*process)(Node *node);
  void (*free)(Node *node);
} NodeVtable;

typedef struct {
  const char *name;
  const char **inlets;
  const char **outlets;
} NodeInfo;

struct Node {
  NodeInfo *info;
  NodeVtable *vtable;
  NodePort *inlets;
  NodePort *outlets;
};

void node_init(Node *node, NodeInfo *info, NodeVtable *vtable, NodePort *inlets, NodePort *outlets);
void node_deinit(Node *node);
void node_free(Node *node);
void node_process(Node *node);
int node_receive(Node *node, const char *str, char *err);
int node_set(Node *node, const char *inlet, float value);
int node_get(Node *node, const char *outlet, float *value);
int node_link(Node *from, const char *outlet, Node *to, const char *inlet);
int node_unlink(Node *from, const char *outlet, Node *to, const char *inlet);

#endif
