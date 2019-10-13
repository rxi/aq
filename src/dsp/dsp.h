#ifndef DSP_H
#define DSP_H

#include "node.h"

typedef void (*DspTickFn)(void);

void dsp_init(DspTickFn fn);
void dsp_set_tick(double t);
int dsp_set_stream(const char *filename);
int dsp_new_node(const char *name);
int dsp_destroy_node(int id);
Node* dsp_get_node(int id);

#endif
