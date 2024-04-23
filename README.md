# Cetone033

Cetone033 is a analogue-style bassline synthesizer by Neotec Software. Though it has been designed for basslines, it's also a chiptune style synthesizer. **Originally written by [René Jeschke](https://github.com/rjeschke).**

Sadly, Cetone Synth series had been discontinued for more than 12 years (since 2012), and it only supported VST 2.4. Original project is [here](https://github.com/rjeschke/cetonesynths).

**But now, I (AnClark) brings it to life again, by re-implementing those plugins to [DISTRHO Plugin Framework](https://distrho.github.io/DPF/).** It now runs well on most modern platforms.

![Screenshot of Cetone033](Screenshot.png)

## Features

- **2 oscillators with 3 chiptune-style waveforms**
  - Waveforms: Saw, Square (pulse), Triangle
- **Analog-modelled filter**
  - **Supported models**: Biquad, **Moog**
  - Switchable filter mode
  - Resonance support
- **Simple modulation on filter**
- Glide (portamento) support
- Simple "Attack-Decay" envelope
- **Cross-platform**
  - Supports: Windows, macOS, Linux
- **Multi-format**
  - Provides: VST 2.4, VST3, LV2, CLAP, Standalone (JACK only)

## How To Build

### 0. Prerequisites

You need to install GCC, CMake, GNU Make and Ninja on your platform. Ninja is highly recommended for its high performance.

### On Linux

```bash
# Ubuntu
sudo apt update
sudo apt install gcc cmake make ninja

# Arch Linux
sudo pacman -Syu
sudo pacman -S gcc cmake make ninja
```

### On Windows

**You need to install Msys2 to build Cetone Synth series.** Download and install it from <https://msys2.org>.

After installation, run **MSYS2 UCRT64 Shell** from desktop (or Start Menu), then execute these commands:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
pacman -S make
```

You can also install Git in Msys2, if you need:

```bash
pacman -S git
```

**Every command below should be executed in MSYS2 UCRT64 Shell.**

### 1. Clone source tree

```bash
# Source tree has 1 submodule: DPF. So you need to add --recursive
git clone https://github.com/AnClark/Minaton-XT.git minaton --recursive

# If you forget --recursive, run this
cd minaton
git submodule update --init --recursive
```

### 2. Build

Cetone series now use CMake as build system. **All platforms share the same commands**.

You can explicitly specify built type here. For best performance, `Release` build is recommended. Optionally you can also set build type to `Debug`.

```bash
cd minaton
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -GNinja    # If you want to use Ninja
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release            # If you are not using Ninja. On Msys2, CMake uses Ninja by default
cmake --build build
```

Built plug-ins reside in `build/bin`. Copy plugins to your DAW's search paths.

## License

- [René's original repo](https://github.com/rjeschke/cetonesynths) is provided 'as is' and license free for public use.
- This repository is licensed under GNU GPLv3.

## Credits

- [René Jeschke](https://github.com/rjeschke) - Original author
- [AnClark Liu](https://github.com/AnClark) - Maintainer of this repository. Re-implemented Cetone033 to DPF.