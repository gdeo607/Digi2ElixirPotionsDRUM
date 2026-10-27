// machines/lxr: the voice routine of the LXR machine type (core-dt2 2.1's DSP voice hook), the shim between the hook
// and the C voice (lxr.c). The PLAITS shim (modules/machines/plaits/src/dsp/plaits_entry.s) in mode 1: 32 samples at
// 48 kHz into the voice's output (I2), the odd voice a copy of the even one. The frame's machine slots (halfword
// 0x6d + 0x30 t + k) into _lm_frame (LxmFrame, lxr.h) in this order: k 0 TUNE, 1 MODEL, 2 WAVE, 6 SWEEP, 7 SDEC,
// 8 MOD, then velocity, ACTIVE, RELEASED, then k 12 FREQ (core 2.2 moves value 34 there and keeps halfword 9, the level, at 100), 13 CLVL, 14 CLICK, 15 MOD2 (values 28-30: the stock builder zeroes halfwords 3-5, core 2.3 copies them to 13-15), 10 FRQ2, 11 CPIT (k 10 and 11:
// values 69 and 70, written by core 2.1). Register and stack rules: see the PLAITS shim.
#define PLAITS_MODE 1
#ifndef PLAITS_MODE
#error PLAITS_MODE is not defined: list the mode file before plaits_entry.s in the object's sources
#endif

.SECTION/PM seg_pmco;
.GLOBAL _lm_render;
.EXTERN _di_frame_hw;
.EXTERN _di_trig;
.EXTERN _di_released;
.EXTERN _di_return_work;
.EXTERN _di_return_out;
.EXTERN lxm_track.;

// Every instruction in its normal 48-bit form; no indirect jumps.
.NOCOMPRESS;

_lm_render:
    R8 = PASS R0;                         // v
    R9 = PASS R2;                         // t
    R10 = PASS R4;                        // the voice record
    R0 = 1;
    R0 = R8 AND R0;
    IF NE JUMP _lm_odd;

    // ---- the even voice: the block's controls into _lm_frame (I5 walks it)
    R0 = _lm_frame;
    I5 = R0;
    R0 = R9 + 1;                          // halfword 1 + t: pitch
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R11 = LSHIFT R9 BY 5;
    R1 = LSHIFT R9 BY 4;
    R11 = R11 + R1;
    R1 = 0x6d;
    R11 = R11 + R1;                       // halfword 0x6d + 0x30 t: machine slot 0
    R0 = PASS R11;                        // TUNE
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R0 = R11 + 1;                         // MODEL (mirror 26, page 1 B)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 2;
    R0 = R11 + R1;                        // WAVE (mirror 27, page 1 C)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 6;
    R0 = R11 + R1;                        // SWEEP (mirror 31, page 1 E)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 7;
    R0 = R11 + R1;                        // SDEC (mirror 32, page 1 F)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 8;
    R0 = R11 + R1;                        // MOD (mirror 33, page 1 G)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 0x1a;
    R0 = R9 + R1;                         // halfword 0x1a + t: velocity << 8
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 0x1b8;
    R1 = R10 + R1;
    I4 = R1;
    R0 = DM(I4, M5);                      // the word at +0x1b8: ACTIVE is its low byte
    R1 = 0xff;
    R0 = R0 AND R1;
    DM(I5, M6) = R0;                      // LxmFrame.active
    R0 = PASS R8;
    CALL _di_released;
    DM(I5, M6) = R0;                      // LxmFrame.released (unused)
    R1 = 12;
    R0 = R11 + R1;                        // FREQ (value 34, page 1 H: halfword 12, core 2.2 moves it there)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;                      // LxmFrame.ratio
    R1 = 13;
    R0 = R11 + R1;                        // CLVL (value 28: halfword 13, core 2.3)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 14;
    R0 = R11 + R1;                        // CLICK (value 29: halfword 14, core 2.3)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 15;
    R0 = R11 + R1;                        // MOD2 (value 30: halfword 15, core 2.3)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 10;
    R0 = R11 + R1;                        // FRQ2 (mirror 69: frame halfword 10, core 2.1)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R1 = 11;
    R0 = R11 + R1;                        // CPIT (mirror 70: frame halfword 11, core 2.1)
    CALL _di_frame_hw;
    F0 = FLOAT R0;
    DM(I5, M6) = R0;
    R0 = PASS R8;
    CALL _di_trig;
    R12 = PASS R0;                        // trig (the third argument)

    // ---- the C call on our own stack
    R0 = _lm_save;
    I3 = R0;
    R0 = I6;
    DM(I3, M6) = R0;
    R0 = I7;
    DM(I3, M6) = R0;
    // The firmware runs its stack as a circular buffer (B6 = B7, L6 = L7 non-zero, set at SW 0x1c0f64..0x1c0f76):
    // on our stack every push and frame MODIFY would wrap by L. Linear for the C call, restored after it.
    // Order (interrupts): L6/L7 = 0 first, while I7 is still the firmware's; only then I7 = _lm_stack.
    R0 = L6;
    DM(I3, M6) = R0;
    R0 = L7;
    DM(I3, M6) = R0;
    L6 = 0;
    L7 = 0;
    R0 = _lm_stack;
    R1 = 4092;                            // the stack's last word (_lm_stack is 4096 bytes)
    R0 = R0 + R1;
    I7 = R0;
#if PLAITS_MODE == 1
    R0 = 48000;                           // rate
    DM(I7, M7) = R0;
    R0 = 32;                              // n
    DM(I7, M7) = R0;
    R0 = I2;                              // out: the voice's output
    DM(I7, M7) = R0;
#else
    R0 = 96000;
    DM(I7, M7) = R0;
    R0 = 64;
    DM(I7, M7) = R0;
    R0 = I1;                              // out: the work buffer
    DM(I7, M7) = R0;
#endif
    R4 = PASS R9;                         // track
    R8 = _lm_frame;                       // &frame
    R2 = I6;
    CJUMP lxm_track. (DB);
    DM(I7, M7) = R2;
    DM(I7, M7) = _lm_back - 1;
_lm_back:
    // Order (interrupts): I6, I7 back to the firmware's stack first (L = 0 there is still safe), then L6/L7.
    R0 = _lm_save;
    I3 = R0;
    R0 = DM(I3, M6);
    I6 = R0;
    R0 = DM(I3, M6);
    I7 = R0;
    R0 = DM(I3, M6);
    L6 = R0;
    R0 = DM(I3, M6);
    L7 = R0;
    JUMP _lm_return;

    // ---- the odd voice: a copy of the even voice's block
_lm_odd:
#if PLAITS_MODE == 1
    R0 = I2;
    R1 = -128;                            // the even voice's output: 0x80 bytes below
    R0 = R0 + R1;
    I3 = R0;
    LCNTR = 32, DO _lm_copy UNTIL LCE;    // a hardware loop: no counter, branch or taken-branch stall per word
    R0 = DM(I3, M6);
_lm_copy:
    DM(I2, M6) = R0;
#else
    R0 = PASS R10;
    R1 = -468;                            // the even voice's work buffer: its record (0x1d8 bytes below) + 4
    R0 = R0 + R1;
    I3 = R0;
    LCNTR = 64, DO _lm_copy UNTIL LCE;
    R0 = DM(I3, M6);
_lm_copy:
    DM(I1, M6) = R0;
#endif

_lm_return:
    // The return: a direct jump (type 8a) to the hook's continuation for this mode.
#if PLAITS_MODE == 1
    JUMP _di_return_out;
#else
    JUMP _di_return_work;
#endif

.SECTION/DM seg_dmda;
.PKDATA _lm_frame, 64;                    // LxmFrame: 16 words
.PKDATA _lm_save, 16;                     // the renderer's I6, I7, L6 and L7 during the C call
.PKBSS _lm_stack, 4096;                   // the C stack (written before it is read)
