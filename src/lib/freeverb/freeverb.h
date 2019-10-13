#ifndef FREEVERB_H
#define FREEVERB_H

/*
** freeverb v0.1
**
** Public domain C implementation of the original freeverb, with the addition of
** support for samplerates other than 44.1khz.
**
** Original C++ version written by Jezard at Dreampoint, June 2000
*/

#define FV_NUMCOMBS       8
#define FV_NUMALLPASSES   4
#define FV_MUTED          0.0
#define FV_FIXEDGAIN      0.015
#define FV_SCALEWET       3.0
#define FV_SCALEDRY       2.0
#define FV_SCALEDAMP      0.4
#define FV_SCALEROOM      0.28
#define FV_STEREOSPREAD   23
#define FV_OFFSETROOM     0.7
#define FV_INITIALROOM    0.5
#define FV_INITIALDAMP    0.5
#define FV_INITIALWET     (1.0 / FV_SCALEWET)
#define FV_INITIALDRY     0.0
#define FV_INITIALWIDTH   1.0
#define FV_INITIALMODE    0.0
#define FV_INITIALSR      44100.0
#define FV_FREEZEMODE     0.5


typedef struct {
  float feedback;
  float filterstore;
  float damp1, damp2;
  float buf[4096];
  int bufsize;
  int bufidx;
} fv_Comb;

typedef struct {
  float feedback;
  float buf[2048];
  int bufsize;
  int bufidx;
} fv_Allpass;

typedef struct {
  float mode;
  float gain;
  float roomsize, roomsize1;
  float damp, damp1;
  float wet, wet1, wet2;
  float dry;
  float width;
  fv_Comb combl[FV_NUMCOMBS];
  fv_Comb combr[FV_NUMCOMBS];
  fv_Allpass allpassl[FV_NUMALLPASSES];
  fv_Allpass allpassr[FV_NUMALLPASSES];
} fv_Context;


void fv_init(fv_Context *ctx);
void fv_mute(fv_Context *ctx);
void fv_process(fv_Context *ctx, float *buf, int n);
void fv_set_samplerate(fv_Context *ctx, float value);
void fv_set_mode(fv_Context *ctx, float value);
void fv_set_roomsize(fv_Context *ctx, float value);
void fv_set_damp(fv_Context *ctx, float value);
void fv_set_wet(fv_Context *ctx, float value);
void fv_set_dry(fv_Context *ctx, float value);
void fv_set_width(fv_Context *ctx, float value);

#endif
