/* SPDX-License-Identifier: GPL-3.0-only */
/* lxr: the value text of MODEL, WAVE, CLICK, FREQ and FRQ2 (the stock formatter convention: fmt(raw 8.8, buf)).
 * FREQ and FRQ2 depend on the model of the track on screen: a modulator's pitch shows as semitones from the carrier
 * (position 48 = 0); a filter, a mix or a noise rate as the plain position 0..127. */
#include <stdint.h>

#define KIT        (*(uint8_t *const volatile *)0x43165028u)   /* the active kit: track t's sound at +0x34 + 0x450 t */
#define UI_TRACK   (*(volatile uint8_t *)0x42936825u)            /* the selected track, 0-15 */

enum { M_DRUM, M_SNARE, M_CYMBAL, M_HIHAT };

static const char *const models[] = { "DRUM", "SNARE", "CYMBAL", "HIHAT" };
static const char *const waves[] = { "SINE", "TRI", "SAW", "SQUARE", "PW25", "PW10", "NOISE" };
static const char *const clicks[] = { "OFF", "SNAP", "CLICK1", "CLICK2", "NOISE" };

static void put(char *buf, const char *s)
{
    while ((*buf++ = *s++) != 0)
        ;
}

static void put_list(char *buf, int32_t raw, const char *const *names, int n)
{
    int32_t k = raw >> 8;
    put(buf, k >= 0 && k < n ? names[k] : "ERR");
}

static void put_int(char *buf, int32_t v, int sign)
{
    char tmp[8];
    int i = 0;
    if (v < 0) {
        *buf++ = '-';
        v = -v;
    } else if (sign) {
        *buf++ = '+';
    }
    do {
        tmp[i++] = (char)('0' + v % 10);
        v /= 10;
    } while (v && i < 7);
    while (i)
        *buf++ = tmp[--i];
    *buf = 0;
}

static int track_model(void)
{
    const uint8_t *kit = KIT;
    uint32_t t = UI_TRACK;
    int32_t m;
    if (!kit || t > 15)
        return M_DRUM;
    m = *(const int16_t *)(kit + 0x34 + 0x450 * t + 0x14 + 2 * 26) >> 8;
    return m >= 0 && m <= M_HIHAT ? (int)m : M_DRUM;
}

void lxr_model_fmt(int32_t raw, char *buf) { put_list(buf, raw, models, 4); }
void lxr_wave_fmt(int32_t raw, char *buf) { put_list(buf, raw, waves, 7); }
void lxr_click_fmt(int32_t raw, char *buf) { put_list(buf, raw, clicks, 5); }

/* FREQ: modulator 1's pitch (DRUM, CYMBAL, HIHAT) or the SNARE's band-pass. */
void lxr_freq_fmt(int32_t raw, char *buf)
{
    int32_t k = raw >> 8;
    if (track_model() == M_SNARE)
        put_int(buf, k, 0);
    else
        put_int(buf, k - 48, 1);
}

/* FRQ2: modulator 2's pitch (CYMBAL, HIHAT), the DRUM's modulator mix (right of the centre) or the SNARE's noise rate. */
void lxr_frq2_fmt(int32_t raw, char *buf)
{
    int32_t k = raw >> 8;
    int m = track_model();
    if (m == M_CYMBAL || m == M_HIHAT)
        put_int(buf, k - 48, 1);
    else if (m == M_DRUM)
        put_int(buf, k > 48 ? k - 48 : 0, 0);      /* the mix: 0 up to the centre */
    else
        put_int(buf, k, 0);
}

/* ---- MODEL's knob, drawn as the Plaits UI 2.1 (Kari) draws Plaits' MODEL: the model's name in a box over a line with
 * a tick at its position. core_knobs gives this widget code 280; it draws the stock knob first (label, touch value) and
 * then the box over the knob's graphic. The widget's arguments are the stock widget's (knobs.s): view, bitmap, x, y,
 * code, value, five more. Coordinates: y = 0 is the bottom row. */
typedef void (*knob_fn)(void *, void *, int, int, int, int, int, int, int, int, int);
typedef void (*rect_fn)(void *bmp, int x0, int y0, int x1, int y1, int colour);
typedef int (*textc_fn)(void *bmp, const void *font, int x, int y, int align, const char *fmt, ...);
#define KNOB_STOCK ((knob_fn)0x40069b8eu)
#define FILLRECT   ((rect_fn)0x401131aeu)
#define TEXTC      ((textc_fn)0x40113dc8u)                  /* text with alignment (2: centred), as UI 2.1 uses it */
#define FONT_KNOB  ((const void *)0x44d2e70cu)              /* the font UI 2.1 draws Plaits' model names with */

static const char *const model_short[] = { "DRUM", "SNR", "CYM", "HAT" };

void lxr_knob(void *view, void *bmp, int x, int y, int code, int value, int a6, int a7, int a8, int a9, int a10)
{
    const int32_t *b = (const int32_t *)bmp;
    int k, pos;
    KNOB_STOCK(view, bmp, x, y, code, value, a6, a7, a8, a9, a10);
    if (code != 280 || !bmp || b[1] != 128 || b[2] != 64 || b[3] != 2 || !b[4])
        return;
    if (x < 3 || x - 3 > 103 || y < 0 || y + 2 > 40)
        return;
    k = value >> 8;
    if (k < 0)
        k = 0;
    if (k > 3)
        k = 3;
    FILLRECT(bmp, x - 3, y + 7, x + 21, y + 24, 0);
    TEXTC(bmp, FONT_KNOB, x + 9, y + 13, 2, "%s", model_short[k]);
    FILLRECT(bmp, x - 1, y + 23, x + 19, y + 23, 1);
    pos = x - 1 + (k * 20) / 3;
    FILLRECT(bmp, pos, y + 21, pos, y + 22, 1);
}
