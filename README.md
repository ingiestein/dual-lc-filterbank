# Dual-LC Filterbank

Windows-first **VST 3** graphic equalizer built only on the [Steinberg VST3 SDK](https://github.com/steinbergmedia/vst3sdk) and bundled **VSTGUI**. No Qt, no JUCE.

Topology is a **parallel constant-K dual-LC inductor bank** (vintage LC graphic cards), not a stack of digital peaking EQs. Preamp feeds every cell; each cell mixes bipolar gain into a summer; the dry path keeps **0 dB cell gains at unity**.

## Signal flow

```
Stereo In → Preamp → Filter Bank (dry + 14 dual-LC cells) → Output Amp → Stereo Out
```

| Cells | Frequency |
| --- | --- |
| Low-pass | 60 Hz |
| Band-pass | 85, 120, 170, 240, 340, 480, 680, 960, 1360, 1920, 2720, 3840 Hz |
| High-pass | 7500 Hz |

Fixed **Q = 0.8** on every cell. Gain range **±12 dB**. Analog prototypes are discretized with the **bilinear transform + cutoff prewarp**. Coefficients rebuild on sample-rate change (44.1 / 48 / 88.2 / 96 kHz).

Preamp (0 to +24 dB) and output (±12 dB) are analog-style gain stages: linear gain plus a gentle tanh saturator.

## Parameters (all automatable)

| ID | Name | Range | Default |
| --- | --- | --- | --- |
| `cell_gain[0..13]` | 14 cell knobs | ±12 dB | 0 dB |
| `preamp_gain` | Preamp | 0 to +24 dB | 0 dB |
| `output_gain` | Output / makeup | ±12 dB | 0 dB |

Operate around **−18 dBFS ≈ +4 dBu**. 0 dBFS is full scale. Stereo in/out, linked channels. No oversampling in v1.

## Requirements

- **CMake ≥ 3.25**
- **C++17**, 64-bit
- **Windows (primary):** Visual Studio 2022 x64 (`MSVC`)
- Linux / macOS: kept working for CI and DSP tests (Ninja + GCC/Clang)

## Clone

The Steinberg SDK is a git submodule (`pluginterfaces`, `base`, `public.sdk`, `vstgui4`, `cmake`):

```bat
git clone --recursive https://github.com/ingiestein/dual-lc-filterbank.git
cd dual-lc-filterbank
```

If you already cloned without submodules:

```bat
git submodule update --init --recursive
```

## Windows build (MSVC 2022 x64)

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The SMTG CMake helpers emit a `DualLcFilterbank.vst3` **bundle**. With `SMTG_CREATE_PLUGIN_LINK=ON` (the default) a link is created in the local VST 3 folder.

### Install / load in a DAW

Copy or link the bundle to:

```
%LOCALAPPDATA%\Programs\Common\VST3
```

(or the common `C:\Program Files\Common Files\VST3` if you prefer a machine-wide install). Rescan plug-ins in **REAPER**, Cubase, or the Steinberg **VST3PluginTestHost**.

To skip the auto-link:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DSMTG_CREATE_PLUGIN_LINK=OFF
```

## Linux (Ninja, CI / cloud)

```bash
sudo apt-get install -y cmake ninja-build g++ pkg-config \
  libx11-dev libx11-xcb-dev libxcb-util-dev libxcb-cursor-dev \
  libxcb-keysyms1-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev \
  libfontconfig1-dev libcairo2-dev libfreetype6-dev libpango1.0-dev \
  libgtkmm-3.0-dev libsqlite3-dev

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSMTG_CREATE_PLUGIN_LINK=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

DSP-only (no VSTGUI / X11):

```bash
cmake -S . -B build-dsp -G Ninja -DDUALLC_BUILD_PLUGIN=OFF -DDUALLC_BUILD_TESTS=ON
cmake --build build-dsp
ctest --test-dir build-dsp --output-on-failure
```

## Validation

The SDK **validator** is built as a post-build step when `SMTG_RUN_VST_VALIDATOR=ON` (default). Run it by hand:

```bat
REM Windows (Release)
build\bin\Release\validator.exe build\VST3\Release\DualLcFilterbank.vst3
```

```bash
# Linux — path may be build/bin/Release/validator or build/bin/validator
./build/bin/Release/validator build/VST3/Release/DualLcFilterbank.vst3
```

Optional: [pluginval](https://github.com/Tracktion/pluginval) on Windows

```bat
pluginval --strictness-level 5 --validate "DualLcFilterbank.vst3"
```

## Layout

```
CMakeLists.txt
cmake/                 VST3 SDK path helper
src/plugin/            factory, processor, controller, version
src/dsp/               dual-LC cell, filter bank, amplifiers
src/gui/               VSTGUI editor.uidesc
resources/             bitmaps, Win32 .rc, snapshots
tests/                 Catch2 offline DSP tests
third_party/vst3sdk    Steinberg SDK submodule
```

## License

Plug-in sources: [MIT](LICENSE). `third_party/vst3sdk` remains under the Steinberg VST 3 SDK license. VST is a trademark of Steinberg Media Technologies GmbH.
