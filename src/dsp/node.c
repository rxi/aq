#include "node.h"


void node_init(Node *node, NodeInfo *info, NodeVtable *vtable, NodePort *inlets, NodePort *outlets) {
  memset(node, 0, sizeof(Node));
  node->info = info;
  node->vtable = vtable;
  node->inlets = inlets;
  node->outlets = outlets;
}


static int remove_link(NodePort *port, Node *node, int idx) {
  for (int i = 0; i < port->link_count; i++) {
    if (port->links[i].node == node && port->links[i].idx == idx) {
      /* pop the last link and replace this link with it */
      port->links[i] = port->links[--port->link_count];
      return 0;
    }
  }
  return -1;
}


void node_deinit(Node *node) {
  /* unlink all nodes linked to this node */
  for (int j = 0; node->info->inlets[j]; j++) {
    NodePort *inlet = &node->inlets[j];
    for (int i = 0; i < inlet->link_count; i++) {
      NodeLink *link = &inlet->links[i];
      remove_link(&link->node->outlets[link->idx], node, j);
    }
  }
}


void node_free(Node *node) {
  node_deinit(node);
  free(node);
}


int node_receive(Node *node, const char *str, char *err) {
  sprintf(err, "node does not support messages");
  return -1;
}


static void mix_buffer(float *dst, float *src, int len) {
  for (int i = 0; i < len; i++) {
    dst[i] += src[i];
  }
}


void node_process(Node *node) {
  /* send all audio from outlets to connected inlets */
  for (int j = 0; node->info->outlets[j]; j++) {
    NodePort *outlet = &node->outlets[j];

    for (int i = 0; i < outlet->link_count; i++) {
      NodeLink *link = &outlet->links[i];
      NodePort *inlet = &link->node->inlets[link->idx];

      /* replace audio if `replace` flag is set, otherwise mix */
      if (inlet->replace) {
        memcpy(inlet->buf, outlet->buf, sizeof(outlet->buf));
        inlet->replace = false;
      } else {
        mix_buffer(inlet->buf, outlet->buf, NODE_BUFFER_SIZE);
      }
    }
  }

  /* reset `replace` flag on all inlets -- causes the next outlet write to
  ** replace the buffer's existing audio instead of mixing to it */
  for (int i = 0; node->info->inlets[i]; i++) {
    node->inlets[i].replace = true;
  }
}


static int string_index(const char **arr, const char *name) {
  for (int i = 0; arr[i]; i++) {
    if (strcmp(arr[i], name) == 0) {
      return i;
    }
  }
  return -1;
}


int node_set(Node *node, const char *inlet, float value) {
  int idx = string_index(node->info->inlets, inlet);
  if (idx < 0) { return NODE_EBADINLET; }
  for (int i = 0; i < NODE_BUFFER_SIZE; i++) {
    node->inlets[idx].buf[i] = value;
  }
  return NODE_ESUCCESS;
}


int node_get(Node *node, const char *outlet, float *value) {
  int idx = string_index(node->info->outlets, outlet);
  if (idx < 0) { return NODE_EBADOUTLET; }
  *value = node->outlets[idx].buf[NODE_BUFFER_SIZE - 1];
  return NODE_ESUCCESS;
}


int node_link(Node *from, const char *outlet, Node *to, const char *inlet) {
  node_unlink(from, outlet, to, inlet);
  int idx1 = string_index(from->info->outlets, outlet);
  int idx2 = string_index(  to->info->inlets,  inlet );
  if (idx1 < 0) { return NODE_EBADOUTLET; }
  if (idx2 < 0) { return NODE_EBADINLET;  }

  NodePort *out = &from->outlets[idx1];
  NodePort *in = &to->inlets[idx2];
  if (out->link_count == NODE_MAX_LINKS) { return NODE_EMAXLINKS; }
  if (in->link_count  == NODE_MAX_LINKS) { return NODE_EMAXLINKS; }

  out->links[out->link_count++] = (NodeLink) { to,   idx2 };
  in ->links[in ->link_count++] = (NodeLink) { from, idx1 };

  return NODE_ESUCCESS;
}


int node_unlink(Node *from, const char *outlet, Node *to, const char *inlet) {
  int idx1 = string_index(from->info->outlets, outlet);
  int idx2 = string_index(  to->info->inlets,  inlet );
  if (idx1 < 0) { return NODE_EBADOUTLET; }
  if (idx2 < 0) { return NODE_EBADINLET;  }

  int err = remove_link(&from->outlets[idx1], to, idx2);
  if (err) { return NODE_EBADLINK; }

  /* this call should always succeed if the previous `remove_link` did */
  remove_link(&to->inlets[idx2], from, idx1);

  return NODE_ESUCCESS;
}
