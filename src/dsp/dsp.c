#include <SDL2/SDL.h>
#include "common.h"
#include "dsp.h"

#define MAX_NODES 10000

static Node *nodes[MAX_NODES];
static int max_node;

static FILE *stream_fp;

static DspTickFn tick_callback;
static double tick_interval = 1;
static double tick_timer;

static SDL_mutex *lock;
static SDL_AudioDeviceID dev;


Node* new_dac_node(void);
Node* new_osc_node(void);
Node* new_svf_node(void);
Node* new_math_node(void);
Node* new_line_node(void);
Node* new_delay_node(void);
Node* new_reverb_node(void);

static struct { const char *name; NodeConstructor fn; } node_table[] = {
  { "dac",    new_dac_node    },
  { "osc",    new_osc_node    },
  { "svf",    new_svf_node    },
  { "math",   new_math_node   },
  { "line",   new_line_node   },
  { "reverb", new_reverb_node },
  { "delay",  new_delay_node  },
  { },
};


static int next_free_id(void) {
  for (int i = 0; i < MAX_NODES; i++) {
    if (nodes[i] == NULL) {
      if (i > max_node) { max_node = i; }
      return i;
    }
  }
  panic("exhausted nodes array");
  return 0; /* never reached */
}


int dsp_new_node(const char *name) {
  for (int i = 0; node_table[i].name; i++) {
    if (strcmp(node_table[i].name, name) == 0) {
      Node *node = node_table[i].fn();
      SDL_LockMutex(lock);
      int id = next_free_id();
      nodes[id] = node;
      SDL_UnlockMutex(lock);
      return id;
    }
  }
  return -1;
}


int dsp_destroy_node(int id) {
  Node *node = dsp_get_node(id);
  if (!node) { return -1; }
  SDL_LockMutex(lock);
  nodes[id] = NULL;
  node->vtable->free(node);
  SDL_UnlockMutex(lock);
  return 0;
}


Node* dsp_get_node(int id) {
  if (id < 0 || id > max_node) { return NULL; }
  return nodes[id];
}


void process_nodes(float *buf) {
  /* process all nodes */
  for (int i = 0; i <= max_node; i++) {
    if (!nodes[i]) { continue; }
    nodes[i]->vtable->process(nodes[i]);
  }

  /* reset output buffer */
  memset(buf, 0, sizeof(float) * NODE_BUFFER_SIZE * 2);

  /* copy dac outlet buffers to provided buffer */
  for (int i = 0; i <= max_node; i++) {
    if (!nodes[i]) { continue; }
    if (strcmp(nodes[i]->info->name, "dac") == 0) {
      for (int j = 0; j < NODE_BUFFER_SIZE; j++) {
        buf[j*2+0] += nodes[i]->outlets[0].buf[j];
        buf[j*2+1] += nodes[i]->outlets[1].buf[j];
      }
    }
  }
}

static void process(float *buf, int len) {
  static float temp_buf[NODE_BUFFER_SIZE * 2];
  static int   temp_buf_idx = 0;

  for (int i = 0; i < len; i++) {
    /* copy from internal buffer to provided buffer */
    buf[i] = temp_buf[temp_buf_idx++];

    /* refill internal buffer if its been exhaused */
    if (temp_buf_idx == NODE_BUFFER_SIZE * 2) {
      SDL_LockMutex(lock);
      process_nodes(temp_buf);
      SDL_UnlockMutex(lock);
      temp_buf_idx = 0;
    }

    /* handle tick timer */
    if (temp_buf_idx == 0) {
      tick_timer -= NODE_SAMPLETIME * NODE_BUFFER_SIZE;
      while (tick_timer < 0) {
        if (tick_callback) { tick_callback(); }
        tick_timer += tick_interval;
      }
    }
  }
}


static void audio_callback(void *udata, uint8_t *buf, int len) {
  process((float*) buf, len / sizeof(float));
  if (stream_fp) { fwrite(buf, len, 1, stream_fp); }
}


void dsp_init(DspTickFn tickfn) {
  tick_callback = tickfn;
  lock = SDL_CreateMutex();
  SDL_AudioSpec fmt = {
    .freq = 44100,
    .format = AUDIO_F32,
    .channels = 2,
    .samples = 1024,
    .callback = audio_callback,
  };
  dev = SDL_OpenAudioDevice(NULL, 0, &fmt, NULL, 0);
  expect(dev);
  SDL_PauseAudioDevice(dev, 0);
}


void dsp_set_tick(double t) {
  tick_interval = t;
}


int dsp_set_stream(const char *filename) {
  if (stream_fp) { fclose(stream_fp); }
  if (filename) {
    stream_fp = fopen(filename, "wb");
    if (!stream_fp) { return -1; }
  }
  return 0;
}
