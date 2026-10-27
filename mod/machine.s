| SPDX-License-Identifier: GPL-3.0-only
| ELIXIR on core-dt2 2.4: four drum voice models after the open-source LXR drum machine firmware (GPL-3.0; the DSP is
| the digikit-format module dsp/machines/lxr) as one machine, type 12, with eleven parameters (codes 280-290) given to core's tables. MODEL picks DRUM, SNARE, CYMBAL or
| HIHAT; the value text of the list knobs and of FREQ / FRQ2 is in fmt.c.
| SRC page 1: A TUNE (stock), B MODEL, C WAVE, D CLVL, E SWEEP, F SDEC, G MOD, H FREQ.
| SRC page 2 (press SRC again): A CLICK, B MOD2, C FRQ2, D CPIT.

        .equ    P_MODEL, 280
        .equ    P_WAVE, 281
        .equ    P_CLVL, 282
        .equ    P_SWEEP, 283
        .equ    P_SDEC, 284
        .equ    P_MOD, 285
        .equ    P_FREQ, 286
        .equ    P_CLICK, 287
        .equ    P_MOD2, 288
        .equ    P_FRQ2, 289
        .equ    P_CPIT, 290

        .section .rodata
        .balign 4
| struct core_machine_dt2 (core-dt2 2.1, machines.s)
        .globl  lxr_machine
lxr_machine:
        .long   12                      | type
        .long   6                       | clone_of: MANUAL SLICE
        .long   255                     | position: the end of the list
        .long   12                      | flags (4 + 8): a page 2 of its own; knob H is FREQ, not LEV (core 2.2)
        .long   0xf8, P_MODEL, P_WAVE, P_CLVL, P_SWEEP, P_SDEC, P_MOD, P_FREQ, 0x0a
        .long   lxr_name, lxr_short
        .long   P_CLICK, P_MOD2, P_FRQ2, P_CPIT, 0, 0, 0, 0      | page 2

| struct core_param_dt2: code, name, short, template, slot, min, max, default, formatter, randomize
        .globl  lxr_p_model, lxr_p_wave, lxr_p_clvl, lxr_p_sweep, lxr_p_sdec, lxr_p_mod, lxr_p_freq
        .globl  lxr_p_click, lxr_p_mod2, lxr_p_frq2, lxr_p_cpit
lxr_p_model: .long P_MODEL, pn_model, ps_model, 0xfe, 1, 0, 0x300, 0, lxr_model_fmt, 1
lxr_p_wave:  .long P_WAVE, pn_wave, ps_wave, 0xfe, 2, 0, 0x600, 0, lxr_wave_fmt, 1
lxr_p_clvl:  .long P_CLVL, pn_clvl, ps_clvl, 0xfe, 3, 0, 0x7f00, 0x4000, 0, 1
lxr_p_sweep: .long P_SWEEP, pn_sweep, ps_sweep, 0xfe, 4, 0, 0x7f00, 0x4000, 0, 1
lxr_p_sdec:  .long P_SDEC, pn_sdec, ps_sdec, 0xfe, 5, 0, 0x7f00, 0x3000, 0, 1
lxr_p_mod:   .long P_MOD, pn_mod, ps_mod, 0xfe, 6, 0, 0x7f00, 0, 0, 1
lxr_p_freq:  .long P_FREQ, pn_freq, ps_freq, 0xfe, 7, 0, 0x7f00, 0x3000, lxr_freq_fmt, 1
lxr_p_click: .long P_CLICK, pn_click, ps_click, 0xfe, 8, 0, 0x400, 0x100, lxr_click_fmt, 1
lxr_p_mod2:  .long P_MOD2, pn_mod2, ps_mod2, 0xfe, 9, 0, 0x7f00, 0, 0, 1
lxr_p_frq2:  .long P_FRQ2, pn_frq2, ps_frq2, 0xfe, 10, 0, 0x7f00, 0x3000, lxr_frq2_fmt, 1
lxr_p_cpit:  .long P_CPIT, pn_cpit, ps_cpit, 0xfe, 11, 0, 0x7f00, 0x4000, 0, 1

lxr_name:  .asciz "Elixir"
lxr_short: .asciz "ELX"
pn_model:  .asciz "Model"
ps_model:  .asciz "MODEL"
pn_wave:   .asciz "Waveform"
ps_wave:   .asciz "WAVE"
pn_clvl:   .asciz "Transient level"
ps_clvl:   .asciz "CLVL"
pn_sweep:  .asciz "Pitch sweep"
ps_sweep:  .asciz "SWEEP"
pn_sdec:   .asciz "Sweep decay"
ps_sdec:   .asciz "SDEC"
pn_mod:    .asciz "Mod/noise mix"
ps_mod:    .asciz "MOD"
pn_freq:   .asciz "Mod freq/filter"
ps_freq:   .asciz "FREQ"
pn_click:  .asciz "Transient"
ps_click:  .asciz "CLICK"
pn_mod2:   .asciz "Mod 2/noise/reso"
ps_mod2:   .asciz "MOD2"
pn_frq2:   .asciz "Mod 2 freq/mix/rate"
ps_frq2:   .asciz "FRQ2"
pn_cpit:   .asciz "Transient pitch"
ps_cpit:   .asciz "CPIT"
