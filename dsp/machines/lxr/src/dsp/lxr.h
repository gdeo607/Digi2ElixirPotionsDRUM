/* LXR for the Digitakt II: the four voice models of the LXR drum machine in one machine (plain C99, float only).
 *
 * After the voices of the LXR drum machine's open-source firmware (DrumVoice.c, Snare.c, CymbalVoice.c, HiHat.c,
 * Oscillator.c, transientGenerator.c; Copyright (C) 2012-2026 Erica Synths, GPL-3.0-only). MODEL picks the voice:
 *
 *   DRUM    a carrier (WAVE) phase-modulated by a sine at FREQ semitones from it, the FM depth following the pitch
 *           sweep as on the LXR; MOD2 crossfades to noise, FRQ2 right of its centre mixes the modulator in as audio (the LXR's mix mode)
 *   SNARE   a carrier and a sample-and-hold noise (rate FRQ2) through a band-pass (cutoff FREQ, resonance MOD2);
 *           MOD crossfades from the carrier to the noise (the LXR's snare mix)
 *   CYMBAL  a carrier phase-modulated by two square modulators (MOD/FREQ, MOD2/FRQ2: amount and semitones)
 *   HIHAT   as CYMBAL at half the FM depth and through a fixed high-pass, as the LXR's hat
 *
 * Every model has the LXR's pitch sweep (SWEEP depth, SDEC decay) and transient (CLICK: OFF, SNAP, CLICK1, CLICK2,
 * NOISE; level CLVL, pitch CPIT). The Digitakt II's FILTER and AMP pages do the filter and the amp envelope, its LFOs
 * the LFO, its overdrive the distortion.
 *
 * Built twice from the same source: on the host (tests, gcc) and by Selache's C compiler for the SHARC+, where
 * src/dsp/lxm_entry.s calls lxm_track from the core-dt2 2.1 DSP voice hook. Selache's C: no optimiser, no inlining,
 * no register-to-register int-to-float conversion, unions read through a cast pointer, tables as words.
 *
 * SPDX-License-Identifier: GPL-3.0-only (the LXR firmware's licence) */
#ifndef LXR_H
#define LXR_H

/* The controls of one block, in the order lxm_entry.s writes them (16 words). Knob words are 8.8 (0 .. 0x7f00); list
 * knobs hold index << 8. */
typedef struct {
    float pitch;     /* frame halfword 1 + t: note << 8 */
    float tune;      /* page 1 A, TUNE (stock): 0x4000 = no transpose, 0x100 per semitone */
    float model;     /* page 1 B, MODEL: DRUM, SNARE, CYMBAL, HIHAT */
    float wave;      /* page 1 C, WAVE: SINE, TRI, SAW, SQUARE, PW25, PW10, NOISE */
    float sweep;     /* page 1 E, SWEEP: the pitch sweep's depth */
    float sdec;      /* page 1 F, SDEC: the pitch sweep's decay */
    float mod;       /* page 1 G, MOD */
    float velocity;  /* frame halfword 0x1a + t: velocity << 8 */
    int active;      /* the voice record's ACTIVE byte */
    int released;    /* the DSP's release pulse (unused) */
    float freq;      /* page 1 H, FREQ */
    float clvl;      /* page 1 D, CLVL: the transient's level */
    float click;     /* page 2 A, CLICK: OFF, SNAP, CLICK1, CLICK2, NOISE */
    float mod2;      /* page 2 B, MOD2 */
    float frq2;      /* page 2 C, FRQ2 */
    float cpit;      /* page 2 D, CPIT: the transient's pitch */
} LxmFrame;

typedef struct {
    float phase;     /* the carrier, 0 .. 1 */
    float m1;        /* modulator 1 */
    float m2;        /* modulator 2 */
    float peg;       /* the pitch sweep, 1 at a note-on */
    float snap;      /* the snap / noise-burst envelope */
    float tpos;      /* the click sample's position (44.1 kHz samples) */
    float vel;       /* the note-on's velocity, 0 .. 1 */
    float nph;       /* the snare noise's sample-and-hold phase */
    float nval;      /* its held value */
    float low;       /* the snare's band-pass */
    float band;
    float hp1;       /* the hat's high-pass (two one-poles: their low-pass states) */
    float hp2;
    int ttype;       /* the transient, latched at the note-on */
    int model;       /* the model, latched at the note-on */
    unsigned int rnd;
} LxmVoice;

#define LXM_MODELS 4
#define LXM_WAVES 7
#define LXM_CLICKS 5
#define LXM_SAMPLE_LENGTH 2205
#define LXM_SAMPLE_WORDS 552

extern const unsigned int lxm_sine[513];
extern const unsigned int lxm_transients[2 * LXM_SAMPLE_WORDS];
int lxm_tables_words(void);

int lxm_render(LxmVoice *v, const LxmFrame *f, int trig, float *out, int n, int rate);
int lxm_track(int track, const LxmFrame *f, int trig, float *out, int n, int rate);

#endif
