/* LXR for the Digitakt II: the render (see lxr.h).
 *
 * Selache's C: int-to-float only through lxm_itof, unions read through a cast pointer, tables as words.
 *
 * SPDX-License-Identifier: GPL-3.0-only */
#include "lxr.h"

#define LXM_INV_256 0.00390625f
#define LXM_INV_7F00 (1.0f / 32512.0f)

enum { M_DRUM, M_SNARE, M_CYMBAL, M_HIHAT };
enum { T_OFF, T_SNAP, T_CLICK1, T_CLICK2, T_NOISE };

static float lxm_itof(int i)
{
    union { int i; float f; } u;
    u.i = 0x4B000000 | i;
    return *(float *)(void *)&u - 8388608.0f;
}

static float lxm_word_float(unsigned int w)
{
    union { unsigned int i; float f; } u;
    u.i = w;
    return *(float *)(void *)&u;
}

/* 2^x for -32 < x < 32: the integer part into the exponent bits, the fraction by a polynomial (about 0.2 cent). */
static float lxm_exp2(float x)
{
    union { int i; float f; } u;
    float y = x + 32.0f;
    int i;
    float fr;
    float p;
    if (y < 0.0f) {
        y = 0.0f;
    }
    i = (int)y;
    fr = y - lxm_itof(i);
    p = 1.0f + fr * (0.6931472f + fr * (0.2402265f + fr * (0.0555041f + fr * 0.0096181f)));
    u.i = (i - 32 + 127) << 23;
    return p * *(float *)(void *)&u;
}

/* The fractional part of x >= -8. */
static float lxm_wrap(float x)
{
    float y = x + 8.0f;
    return y - lxm_itof((int)y);
}

/* A knob word (0 .. 0x7f00) as 0 .. 1. */
static float lxm_unit(float w)
{
    float u = w * LXM_INV_7F00;
    if (u < 0.0f) {
        u = 0.0f;
    }
    if (u > 1.0f) {
        u = 1.0f;
    }
    return u;
}

/* A knob word as its position 0 .. n - 1. */
static int lxm_index(float w, int n)
{
    int k;
    if (w < 0.0f) {
        return 0;
    }
    k = (int)(w * LXM_INV_256 + 0.5f);
    if (k > n - 1) {
        k = n - 1;
    }
    return k;
}

/* A FREQ knob as a frequency ratio: position 48 is 1, one semitone per step (1/16 .. 96). */
static float lxm_semis(float w)
{
    int k = lxm_index(w, 128);
    return lxm_exp2((lxm_itof(k) - 48.0f) * (1.0f / 12.0f));
}

static float lxm_sin(float phase)
{
    float x = phase * 512.0f;
    int i = (int)x;
    float fr = x - lxm_itof(i);
    float a;
    float b;
    i = i & 511;
    a = lxm_word_float(lxm_sine[i]);
    b = lxm_word_float(lxm_sine[i + 1]);
    return a + (b - a) * fr;
}

/* The polyBLEP residual at phase t (0 .. 1) for an increment dt. */
static float lxm_blep(float t, float dt)
{
    if (t < dt) {
        t = t / dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

static float lxm_noise(LxmVoice *v)
{
    unsigned int r = v->rnd;
    r = r ^ (r << 13);
    r = r ^ (r >> 17);
    r = r ^ (r << 5);
    v->rnd = r;
    return lxm_itof((int)(r >> 9)) * (1.0f / 4194304.0f) - 1.0f;
}

/* Sample k (0 .. 2204) of click sample s (0, 1), -1 .. 1. */
static float lxm_click_sample(int s, int k)
{
    unsigned int w = lxm_transients[s * LXM_SAMPLE_WORDS + (k >> 2)];
    int b = (int)((w >> ((k & 3) << 3)) & 255u);
    if (b > 127) {
        b = b - 256;
    }
    if (b < 0) {
        return -lxm_itof(-b) * (1.0f / 128.0f);
    }
    return lxm_itof(b) * (1.0f / 128.0f);
}

/* The carrier: waveform w at phase ph with increment dt. */
static float lxm_wave(LxmVoice *v, int w, float ph, float dt)
{
    float s;
    float duty;
    if (w == 0) {
        return lxm_sin(ph);
    }
    if (w == 1) {
        return ph < 0.5f ? 4.0f * ph - 1.0f : 3.0f - 4.0f * ph;
    }
    if (w == 2) {
        return 2.0f * ph - 1.0f - lxm_blep(ph, dt);
    }
    if (w == 6) {
        return lxm_noise(v);
    }
    duty = w == 3 ? 0.5f : (w == 4 ? 0.25f : 0.1f);
    s = ph < duty ? 1.0f : -1.0f;
    return s + lxm_blep(ph, dt) - lxm_blep(lxm_wrap(ph + 1.0f - duty), dt);
}

static float lxm_cap(float dt)
{
    return dt > 0.45f ? 0.45f : dt;
}

int lxm_render(LxmVoice *v, const LxmFrame *f, int trig, float *out, int n, int rate)
{
    float frate = lxm_itof(rate);
    float inv_rate = 1.0f / frate;
    float note = f->pitch * LXM_INV_256 + (f->tune * LXM_INV_256 - 64.0f);
    float f0 = 440.0f * lxm_exp2((note - 69.0f) * (1.0f / 12.0f)) * inv_rate;
    float sweep = lxm_unit(f->sweep);
    float depth = sweep * sweep * 16.0f;
    float tau = 0.003f * lxm_exp2(lxm_unit(f->sdec) * 8.0f);     /* 3 ms .. 768 ms */
    float pcoef = 1.0f - inv_rate / tau;
    float scoef = 1.0f - inv_rate / 0.0015f;                    /* the snap and the noise burst: 1.5 ms */
    float tstep = 44100.0f * inv_rate;
    int wave = lxm_index(f->wave, LXM_WAVES);
    float mod = lxm_unit(f->mod);
    float mod2 = lxm_unit(f->mod2);
    float frq2u = lxm_unit(f->frq2);
    float r1 = lxm_semis(f->freq);
    float r2 = lxm_semis(f->frq2);
    float mixu = (lxm_itof(lxm_index(f->frq2, 128)) - 48.0f) * (1.0f / 79.0f);   /* DRUM: 0 up to the centre (48) */
    float clvl = lxm_unit(f->clvl) * 2.0f;                      /* the transient's level, 1 at the default 0x4000 */
    float cpit = 0.25f + 1.5f * lxm_unit(f->cpit);              /* the transient's pitch, 0.25 .. 1.75 (the LXR's) */
    float fmi = mod * mod * 6.0f * 0.1591549f;                  /* DRUM: the FM index, in cycles */
    float nrate = 100.0f * lxm_exp2(frq2u * 9.0f) * inv_rate;   /* SNARE: the noise's sample rate, 100 Hz .. 51 kHz */
    float fc = 80.0f * lxm_exp2(lxm_unit(f->freq) * 8.0f) * inv_rate;   /* SNARE: the band-pass, 80 Hz .. 20 kHz */
    float damp = 2.0f - 1.85f * mod2;                           /* SNARE: its resonance (the filter's k) */
    float g;
    float a1;
    float a2;
    float a3;
    int bad = 0;
    float gain;
    int k;

    if (mixu < 0.0f) {
        mixu = 0.0f;
    }
    if (fc > 0.45f) {
        fc = 0.45f;
    }
    /* SNARE: a topology-preserving (zero-delay) state-variable band-pass, stable at every cutoff and resonance:
     * g = tan(pi fc / fs) (sin over cos from the table), the coefficients once per block. */
    g = lxm_sin(fc * 0.5f) / lxm_sin(fc * 0.5f + 0.25f);
    a1 = 1.0f / (1.0f + g * (g + damp));
    a2 = g * a1;
    a3 = g * a2;
    if (trig) {
        v->phase = 0.0f;
        v->m1 = 0.0f;
        v->m2 = 0.0f;
        v->peg = 1.0f;
        v->snap = 1.0f;
        v->tpos = 0.0f;
        v->vel = lxm_unit(f->velocity);
        v->ttype = lxm_index(f->click, LXM_CLICKS);
        v->model = lxm_index(f->model, LXM_MODELS);
        if (v->rnd == 0) {
            v->rnd = 0x9e3779b9u;
        }
    }
    if (!f->active) {
        for (k = 0; k < n; ++k) {
            out[k] = 0.0f;
        }
        return 0;
    }
    gain = 0.4f + 0.6f * v->vel;
    if (v->model == M_CYMBAL) {
        gain = gain * 0.35f;
    } else if (v->model == M_HIHAT) {
        gain = gain * 0.6f;
    } else {
        gain = gain * 0.5f;
    }

    for (k = 0; k < n; ++k) {
        float peg = v->peg;
        float pf = 1.0f + depth * peg;
        float dt;
        float ph;
        float s;
        float tr = 0.0f;
        float m;
        if (v->ttype == T_SNAP) {
            pf = pf + 8.0f * clvl * v->snap;
        }
        dt = lxm_cap(f0 * pf);
        ph = v->phase;

        if (v->model == M_DRUM) {
            m = lxm_sin(v->m1);
            if (fmi > 0.0f) {
                ph = lxm_wrap(ph + fmi * peg * m);
            }
            s = lxm_wave(v, wave, ph, dt);
            s = s + mixu * (m - s) * 0.5f;                      /* FRQ2: the modulator as audio */
            if (mod2 > 0.0f) {
                s = s + mod2 * (lxm_noise(v) - s);              /* MOD2: a crossfade to noise */
            }
            v->m1 = lxm_wrap(v->m1 + lxm_cap(f0 * pf * r1));
        } else if (v->model == M_SNARE) {
            float hp;
            v->nph = v->nph + nrate;
            if (v->nph >= 1.0f) {
                v->nph = lxm_wrap(v->nph);
                v->nval = lxm_noise(v);
            }
            hp = v->nval - v->low;                              /* v->band, v->low: the filter's two states */
            m = a1 * v->band + a2 * hp;                         /* the band-pass output */
            hp = v->low + a2 * v->band + a3 * hp;               /* the low-pass output */
            v->band = m + m - v->band;
            v->low = hp + hp - v->low;
            s = lxm_wave(v, wave, ph, dt);
            s = s + mod * (m * 1.5f - s);                       /* MOD: carrier .. band-passed noise */
        } else {
            float a1 = v->m1 < 0.5f ? 1.0f : -1.0f;
            float a2 = v->m2 < 0.5f ? 1.0f : -1.0f;
            float idx = v->model == M_HIHAT ? 1.0f : 2.0f;
            m = idx * (mod * a1 + mod2 * a2);
            ph = lxm_wrap(ph + m);
            s = lxm_wave(v, wave, ph, dt);
            v->m1 = lxm_wrap(v->m1 + lxm_cap(f0 * pf * r1));
            v->m2 = lxm_wrap(v->m2 + lxm_cap(f0 * pf * r2));
            if (v->model == M_HIHAT) {
                v->hp1 = v->hp1 + 0.33f * (s - v->hp1);          /* two one-pole high-passes at about 3 kHz */
                s = s - v->hp1;
                v->hp2 = v->hp2 + 0.33f * (s - v->hp2);
                s = s - v->hp2;
            }
        }

        if (v->ttype == T_CLICK1 || v->ttype == T_CLICK2) {
            int i = (int)v->tpos;
            if (i < LXM_SAMPLE_LENGTH) {
                tr = 0.7f * clvl * lxm_click_sample(v->ttype - T_CLICK1, i);
                v->tpos = v->tpos + tstep * cpit;
            }
        } else if (v->ttype == T_NOISE) {
            tr = 0.8f * clvl * v->snap * lxm_noise(v);
        }
        s = (s + tr) * gain;
        if (!(s > -4.0f && s < 4.0f)) {                         /* out of range or NaN: never into the mix */
            s = 0.0f;
            bad = 1;
        }
        out[k] = s;

        v->phase = lxm_wrap(v->phase + dt);
        v->peg = peg * pcoef;
        v->snap = v->snap * scoef;
    }
    if (bad || !(v->band > -1.0e4f && v->band < 1.0e4f) || !(v->low > -1.0e4f && v->low < 1.0e4f)
        || !(v->hp1 > -1.0e4f && v->hp1 < 1.0e4f) || !(v->hp2 > -1.0e4f && v->hp2 < 1.0e4f)
        || !(v->phase >= 0.0f && v->phase < 1.0f) || !(v->m1 >= 0.0f && v->m1 < 1.0f) || !(v->m2 >= 0.0f && v->m2 < 1.0f)) {
        /* a voice that went wrong starts again silent instead of feeding the mix (a NaN there silences every track) */
        v->phase = 0.0f;
        v->m1 = 0.0f;
        v->m2 = 0.0f;
        v->band = 0.0f;
        v->low = 0.0f;
        v->hp1 = 0.0f;
        v->hp2 = 0.0f;
        v->nph = 0.0f;
        v->nval = 0.0f;
        for (k = 0; k < n; ++k) {
            out[k] = 0.0f;
        }
    }
    return 1;
}

static LxmVoice lxm_voices[16]; /* zero at load (loaded data) */

int lxm_track(int track, const LxmFrame *f, int trig, float *out, int n, int rate)
{
    return lxm_render(&lxm_voices[track & 15], f, trig, out, n, rate);
}
