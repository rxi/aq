#ifndef MIDI_H
#define MIDI_H

#include "common.h"

#define MIDI_TYPE_LIST\
  X( MIDI_NOTEOFF,        0x80,   3 )\
  X( MIDI_NOTEON,         0x90,   3 )\
  X( MIDI_POLYTOUCH,      0xa0,   3 )\
  X( MIDI_CONTROLCHANGE,  0xb0,   3 )\
  X( MIDI_PROGRAMCHANGE,  0xc0,   2 )\
  X( MIDI_AFTERTOUCH,     0xd0,   2 )\
  X( MIDI_PITCHWHEEL,     0xe0,   3 )\
  X( MIDI_SYSEX,          0xf0,   0 )\
  X( MIDI_QUARTERFRAME,   0xf1,   2 )\
  X( MIDI_SONGPOSITION,   0xf2,   3 )\
  X( MIDI_SONGSELECT,     0xf3,   2 )\
  X( MIDI_TUNEREQUEST,    0xf6,   1 )\
  X( MIDI_CLOCK,          0xf8,   1 )\
  X( MIDI_START,          0xfa,   1 )\
  X( MIDI_CONTINUE,       0xfb,   1 )\
  X( MIDI_STOP,           0xfc,   1 )\
  X( MIDI_ACTIVESENSING,  0xfe,   1 )\
  X( MIDI_RESET,          0xff,   1 )

enum {
  #define X(e, val, len) e = val,
  MIDI_TYPE_LIST
  #undef X
};

typedef union {
  unsigned char b[3];
  unsigned char status;
  struct { unsigned char status, note, velocity; } note;
  struct { unsigned char status, control, value; } cc;
  struct { unsigned char status, value; } progchange;
  struct { unsigned char status, value; } aftertouch;
} MidiMessage;

typedef void (*MidiMessageFn)(MidiMessage msg);

static inline int midi_type(MidiMessage msg) {
  return msg.status >= 0xf0 ? msg.status : msg.status & 0xf0;
}

static inline int midi_channel(MidiMessage msg) {
  return msg.status & 0xf;
}

void midi_init(MidiMessageFn fn);


#endif
