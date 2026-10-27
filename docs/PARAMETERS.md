# ELIXIR parameters

ELIXIR has two SRC pages. Press SRC again to reach page 2; the header shows 1/2 or 2/2. Every parameter can be
locked on a trig, used as an LFO destination, and is saved with the sound.

| Page / knob | Name | DRUM | SNARE | CYMBAL / HIHAT |
|---|---|---|---|---|
| 1 A | TUNE | pitch | pitch | pitch |
| 1 B | MODEL | DRUM, SNARE, CYMBAL, HIHAT | | |
| 1 C | WAVE | carrier: SINE, TRI, SAW, SQUARE, PW25, PW10, NOISE | carrier | carrier |
| 1 D | CLVL | transient level | transient level | transient level |
| 1 E | SWEEP | pitch sweep depth | pitch sweep depth | pitch sweep depth |
| 1 F | SDEC | pitch sweep decay | pitch sweep decay | pitch sweep decay |
| 1 G | MOD | FM depth (follows the sweep) | carrier to noise mix | modulator 1 amount |
| 1 H | FREQ | modulator interval (semitones) | band-pass cutoff | modulator 1 interval |
| 2 A | CLICK | transient: OFF, SNAP, CLICK1, CLICK2, NOISE | | |
| 2 B | MOD2 | noise mix | filter resonance | modulator 2 amount |
| 2 C | FRQ2 | the modulator as audio (right of the centre only) | noise rate / colour | modulator 2 interval |
| 2 D | CPIT | transient pitch (CLICK1 / CLICK2) | | |

Notes:

- CLVL does nothing when CLICK is OFF. With the default SNAP it is a very short pitch spike, so it is subtle; it is
  clearest with CLICK1, CLICK2 or NOISE.
- CPIT only changes CLICK1 and CLICK2.
- HIHAT is CYMBAL at half the FM depth, through a fixed high-pass.
- The Digitakt's own FILTER, AMP, LFO and overdrive pages work as on any machine. The AMP envelope shapes the voice.
