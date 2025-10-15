#include <SDL2/SDL.h>
#include "midi.h"


MidiMessageFn midi_callback;

static void midi_platform_init(void);
static void midi_platform_send(MidiMessage msg);


void midi_init(MidiMessageFn fn) {
  midi_callback = fn;
  midi_platform_init();
}


void midi_send(MidiMessage msg) {
  midi_platform_send(msg);
}


static void send_message(MidiMessage msg) {
  if (midi_callback) { midi_callback(msg); }
}



#ifdef __linux__

#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

static const int sizes[] = {
  #define X(e, val, len) [val] = len,
  MIDI_TYPE_LIST
  #undef X
};

typedef struct { int fd; } MidiInput;
#define MAX_MIDI_IN_OUTS 16
static MidiInput midi_inputs[MAX_MIDI_IN_OUTS];
static FILE *midi_outputs[MAX_MIDI_IN_OUTS];


static int midi_thread(void *udata) {
  for (;;) {
    /* prepare readset for select */
    fd_set readset;
    int max_fd = 0;
    FD_ZERO(&readset);
    
    for (int i = 0; i < MAX_MIDI_IN_OUTS && midi_inputs[i].fd >= 0; i++) {
      int fd = midi_inputs[i].fd;
      FD_SET(fd, &readset);
      if (fd > max_fd) { max_fd = fd; }
    }
    
    /* If no devices are open, sleep to prevent a high-CPU busy-loop */
    if (max_fd == 0) {
      SDL_Delay(100);
      continue;
    }

    select(max_fd + 1, &readset, NULL, NULL, NULL);

    for (int i = 0; i < MAX_MIDI_IN_OUTS && midi_inputs[i].fd >= 0; i++) {
      int fd = midi_inputs[i].fd;
      if (FD_ISSET(fd, &readset)) {
        unsigned char raw_buf[128];
        int bytes_read = read(fd, raw_buf, sizeof(raw_buf));
        int head = 0;

        while (head < bytes_read) {
          MidiMessage msg;
          msg.b[0] = raw_buf[head];
          int msg_len = sizes[midi_type(msg)];
          
          if (head + msg_len > bytes_read) { break; }

          for (int j = 1; j < msg_len; j++) {
            msg.b[j] = raw_buf[head + j];
          }
          send_message(msg);
          head += msg_len;
        }
      }
    }
  }
  return 0;
}


static void midi_platform_init(void) {
  char filename[32];
  int input_count = 0;
  
  for (int i = 0; i < MAX_MIDI_IN_OUTS; i++) {
    midi_inputs[i].fd = -1;
  }

  for (int i = 0; i < MAX_MIDI_IN_OUTS; i++) {
    sprintf(filename, "/dev/midi%d", i);
    int fd = open(filename, O_RDONLY | O_NONBLOCK); 
    if (fd >= 0) {
      midi_inputs[input_count].fd = fd;
      input_count++;
    }
  }

  for (int i = 1; i < MAX_MIDI_IN_OUTS; i++) {
    sprintf(filename, "/dev/midi%d", i);
    FILE *fp = fopen(filename, "wb");
    if (fp) { 
      midi_outputs[i - 1] = fp;
    }
  }
  
  SDL_CreateThread(midi_thread, "Midi Input", NULL);
}


static void midi_platform_send(MidiMessage msg) {
  int sz = sizes[midi_type(msg)];
  for (int i = 0; i < MAX_MIDI_IN_OUTS; i++) {
    if (midi_outputs[i]) {
      fwrite(&msg, sz, 1, midi_outputs[i]);
      fflush(midi_outputs[i]);
    }
  }
}

void midi_platform_send_sysex(unsigned char *data, int len) {
  for (int i = 0; i < MAX_MIDI_IN_OUTS; i++) {
    if (midi_outputs[i]) {
      fwrite(data, 1, len, midi_outputs[i]);
      fflush(midi_outputs[i]);
    }
  }
}
#endif



#ifdef _WIN32

#include <windows.h>

static HMIDIOUT midi_outputs[32];

static void CALLBACK midi_input_callback(HMIDIIN hMidiIn, UINT wMsg,
  DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
  if (wMsg == MIM_DATA) {
    MidiMessage msg;
    memcpy(&msg, &dwParam1, sizeof(msg));
    send_message(msg);
  }
}


static void midi_platform_init(void) {
  /* init all midi in devices */
  int n = midiInGetNumDevs();
  for (int i = 0; i < n; i++) {
    HMIDIIN dev = NULL;
    int res = midiInOpen(&dev, i, (DWORD_PTR) midi_input_callback, i, CALLBACK_FUNCTION);
    expect(res == MMSYSERR_NOERROR);
    midiInStart(dev);
  }

  /* init all midi out devices */
  n = midiOutGetNumDevs();
  for (int i = 1; i < n; i++) {
    HMIDIOUT dev;
    int res = midiOutOpen(&dev, i, 0, 0, CALLBACK_NULL);
    expect(res == MMSYSERR_NOERROR);
    midi_outputs[i - 1] = dev;
  }
}


static void midi_platform_send(MidiMessage msg) {
  for (int i = 0; midi_outputs[i]; i++) {
    midiOutShortMsg(midi_outputs[i], *((DWORD*) &msg));
  }
}

#endif
