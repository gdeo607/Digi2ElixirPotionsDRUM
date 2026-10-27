# Digi2ElixirPotionsDRUM: ELIXIR for the Digitakt II

**ELIXIR** is a drum-synthesis machine for the **Elektron Digitakt II** (OS 1.17). It is an elekloader mod. One
machine holds four voice models, and its **MODEL** knob picks between them:

- **DRUM**: a carrier phase-modulated by a sine, the FM depth following the pitch sweep.
- **SNARE**: a carrier plus band-passed sample-and-hold noise.
- **CYMBAL**: a carrier phase-modulated by two square modulators.
- **HIHAT**: the same, thinner and high-passed.

Every model has a pitch sweep and a transient (snap, two click samples, or a noise burst). The voices are written
after the voices of the open-source **LXR drum machine firmware** by Sonic Potions / Erica Synths (GPL-3.0).

> Unofficial and unsupported. Flashing modified firmware is at your own risk; the stock OS always recovers the
> unit (send the stock .syx from the boot menu). This repository contains no Elektron code or firmware.

Parameters, per model: [docs/PARAMETERS.md](docs/PARAMETERS.md).

## What you need

| | |
|---|---|
| Unit | Digitakt II, OS 1.17 |
| Mods | **core-dt2 2.4** (see Digi2Core) and this mod, `elemods/elixir-0.2.0.elemod` |
| elekloader | a build with `sharc_raw` support for the Digitakt II (DSP objects in mods) |
| Not with | DrumVoice or older LXR test builds: ELIXIR takes machine type 12 |

Build the firmware from your own stock OS file:

```
python -m elekloader.patch --stock Digitakt_II_OS1.17.syx \
  --mod core-2.4.elemod --mod elemods/elixir-0.2.0.elemod --out Digitakt_II_elixir.syx
```

Never share the resulting .syx: it contains Elektron's OS.

## Layout

| path | what |
|---|---|
| `mod/` | the ColdFire side: the machine descriptor and its eleven parameters (`machine.s`), the value text and the MODEL knob drawing (`fmt.c`), `mod.json`, and `sharc_raw.json`, which adds the DSP object to core's voice table |
| `dsp/machines/lxr/` | the DSP voice as a digikit-format module: the C voice (`lxr.c`, `lxr.h`), the tables (`lxm_tables.c`), the entry shim (`lxm_entry.s`) and the built object `build/lxr-dsp.dko.json` |
| `tests/` | host tests of the C voice: `render.py` renders example sounds, `fuzz.py` checks that no setting produces NaN or runaway output |
| `elemods/` | the built mod |

## Building

The DSP object is built with digikit's patchkit (the Selache C compiler for the SHARC+). Copy
`dsp/machines/lxr` into a digikit checkout's `modules/machines/` and run:

```
mise run patch:build -- modules/machines/lxr
```

Copy `build/lxr-dsp.dko.json` into `mod/`. Then build the mod with elekloader's SDK and an m68k cross GCC:

```
python -m elekloader.sdk.build mod --stock Digitakt_II_OS1.17.syx
```

The host tests build the voice with gcc:

```
gcc -O2 -shared -fPIC -o liblxm.so dsp/machines/lxr/src/dsp/lxr.c dsp/machines/lxr/src/dsp/lxm_tables.c
python3 tests/fuzz.py ./liblxm.so
python3 tests/render.py ./liblxm.so out/
```

## Changes

- **0.2.0**: renamed ELIXIR. Knob D (CLVL) now holds its value: core 2.4 stops the stock OS cutting value 28 to its
  sample-slot bits on every frame.
- **0.1.3**: the snare's filter is a zero-delay state-variable filter, and a voice that goes wrong restarts silent.
  Before this, FREQ above about 100 could silence the whole unit.
- **0.1.2**: knob H is the machine's own (FREQ), no longer the level; page 2 reaches the voice.
- **0.1**: the first version.

## Credits and licences

- The voice models follow the LXR drum machine firmware (DrumVoice.c, Snare.c, CymbalVoice.c, HiHat.c,
  Oscillator.c, transientGenerator.c; Copyright (C) 2012-2026 Sonic Potions / Erica Synths, GPL-3.0). The two click
  samples in `lxm_tables.c` are the LXR firmware's transient samples.
- `dsp/machines/lxr/src/dsp/lxm_entry.s` is derived from digikit's Plaits entry shim: **digikit by Em D -
  https://github.com/ElekLabs/digikit** (AGPL-3.0-or-later, with the additional terms in `dsp/NOTICE-digikit`). It is
  modified: it reads ELIXIR's sixteen controls.
- core-dt2 2.x builds on digikit's machine, parameter and DSP-inline frameworks and on irpina's elekloader core.
- Built with irpina's [elekloader](https://github.com/irpina/elekloader).

Licence: GPL-3.0-only (`LICENSE`), except `lxm_entry.s`, which is AGPL-3.0-or-later (`dsp/LICENSE-AGPL-3.0`). The
DSP object combines them under section 13 of both licences.

## Trademarks

Elektron and Digitakt are trademarks of Elektron Music Machines. LXR is a product of Sonic Potions / Erica Synths.
They are named only to say what this works with or was derived from. This project is not affiliated with or
endorsed by any of them.
