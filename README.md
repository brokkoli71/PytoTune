# PytoTune

PytoTune is an open-source pitch-correction library with a C++ backend, Python bindings, and a command-line interface.
It was built as an Algorithm Engineering project and focuses on making the full Auto-Tune-style pipeline transparent,
reproducible, and fast.

The project supports two correction modes:

- **MIDI-guided correction**: match an input WAV file to the notes of a MIDI file
- **Scale-guided correction**: quantize detected pitches to a musical scale such as `C major`, `E minor`, or more exotic
  tunings

Under the hood, PytoTune combines:

- **YIN-based pitch detection**
- **Scale or MIDI target pitch generation**
- **Fourier-based pitch shifting**
- **OpenMP parallelization**
- **SIMD acceleration via Google Highway**

## Table of Contents

- [Features](#features)
- [Project Status](#project-status)
- [How It Works](#how-it-works)
- [Repository Layout](#repository-layout)
- [Requirements](#requirements)
- [Setup and Build](#setup-and-build)
- [Python Usage](#python-usage)
- [CLI Usage](#cli-usage)
- [Testing](#testing)
- [Benchmarking](#benchmarking)
- [Limitations](#limitations)
- [Further Reading](#further-reading)
- [License](#license)

## Features

- C++20 core implementation for the heavy DSP work
- Python extension module built with `pybind11`
- Standalone CLI executable for file-based workflows
- Two pitch-correction modes:
    - WAV + MIDI → corrected WAV
    - WAV + musical scale → corrected WAV
- Built-in scale parsing, including common western modes and non-standard scales
- Configurable pitch-detection ranges, including presets for common vocal ranges
- Optimized YIN detector with:
    - OpenMP parallelization
    - SIMD vectorization with Highway
    - signal decimation before detection
- Unit tests with GoogleTest
- Linux benchmarking target and helper scripts

## Project Status

PytoTune is currently a **research/project codebase** rather than a polished end-user package. The core library, CLI,
tests, and Python bindings are present in this repository. However, the repository does **not** currently ship PyPI
packaging metadata such as a `pyproject.toml`.

That means local usage is currently centered around **building with CMake**.

## How It Works

The processing pipeline is:

1. **Load input data** from a WAV file and, optionally, a MIDI file
2. **Split audio into overlapping windows** (default: window size `4096`, stride `1024`)
3. **Detect pitch per window** using the YIN algorithm
4. **Compute target pitches** either from:
    - a MIDI note sequence, or
    - the nearest note in a chosen musical scale
5. **Apply pitch shifting** with a Fourier-transform-based algorithm
6. **Overlap-add corrected windows** into the final output WAV file

This architecture keeps the full signal-processing chain explicit and easy to inspect.

## Repository Layout

```text
.
├── include/                # Public headers
├── src/                    # C++ implementation, CLI, Python bindings
├── tests/                  # GoogleTest test suite and sample test assets
├── benchmarks/             # Benchmark executable and helper scripts
├── paper/                  # Project paper and figures
├── highway/                # SIMD dependency vendored into the repo
├── CMakeLists.txt          # Main build configuration
└── README.md
```

## Requirements

The most reliable local setup is to follow the CI toolchain choices.

### Linux

- `build-essential`
- `cmake`
- `ninja-build`
- `python3`

Install command:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build
```

### macOS

- Homebrew
- `cmake`
- `ninja`
- `gcc` (using `gcc-15` / `g++-15`)
- `python3`

Install command:

```bash
brew update
brew install cmake ninja gcc
```

### Windows

- Visual Studio C++ toolchain
- `cmake`
- `python`

### Dependencies fetched automatically by CMake

The following dependencies are downloaded automatically during configuration:

- `pybind11`
- `googletest`

The project also builds against the bundled `highway/` directory.

### Platform notes

- **Linux/macOS**: builds use the Ninja generator
- **macOS**: uses Homebrew GCC (`gcc-15` / `g++-15`) via `CC` and `CXX`
- **Windows**: use the Visual Studio generator defaults with `-A x64`
- **Benchmarking**: only enabled on Linux

## Setup and Build

### 1. Clone the repository

```bash
git clone https://github.com/brokkoli71/PytoTune.git
cd PytoTune
```

### 2. Configure + build (Linux)

```bash
mkdir -p build
cd build
cmake -S .. -B . -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -v
```

### 3. Configure + build (macOS with GCC)

Use Homebrew GCC 15:

```bash
export CC=gcc-15
export CXX=g++-15
mkdir -p build
cd build
cmake -S .. -B . -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -v
```

### 4. Configure + build (Windows)

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -- /m
```

This builds:

- the core static library
- the Python extension module `pytotune`
- the CLI executable `pytotune_cli`
- the GoogleTest binary `pytotune_tests`

### Optional build flags

The following CMake options are available:

- `PYTOTUNE_USE_HWY=ON|OFF` — enable/disable Highway SIMD in pitch detection
- `PYTOTUNE_USE_PREDEFINED_TWIDDLES=ON|OFF` — enable/disable predefined twiddle optimization in the FFT
- `PYTOTUNE_REIMPLEMENTED_WINDOWING=ON|OFF` — enable/disable the alternative pitch-shifter windowing implementation

Example:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPYTOTUNE_USE_HWY=OFF
cmake --build build -j
```

### macOS note

If the compiler name does not exist (for example `gcc-15` is missing), check available versions:

```bash
ls -1 /opt/homebrew/bin/gcc-* /usr/local/bin/gcc-*
```

Then set `CC` and `CXX` to matching binaries before running CMake.

## Python Usage

After building, the Python extension module is available in the build directory.

### Make the module importable

```bash
export PYTHONPATH="$PWD/build:$PYTHONPATH"
```

### Example: tune to a scale

```python
import pytotune

scale = pytotune.Scale.fromName("C major")
pytotune.roundToScale("input.wav", scale, "output_scale.wav")
```

### Example: tune to MIDI

```python
import pytotune

pytotune.matchMidi("input.wav", "reference.mid", "output_midi.wav")
```

### Example: use an explicit pitch range

```python
import pytotune

pitch_range = pytotune.PitchRange(82.41, 1046.50)
scale = pytotune.Scale.fromName("E minor")
pytotune.roundToScale("input.wav", scale, "output.wav", pitch_range)
```

### Example: use a preset singer range

```python
import pytotune

pitch_range = pytotune.singerToPitchRange("tenor")
pytotune.matchMidi("input.wav", "reference.mid", "output.wav", pitch_range)
```

### Available pitch-range presets

- `hearable`
- `piano`
- `human`
- `man`
- `woman`
- `bass`
- `tenor`
- `bariton`
- `alto`
- `soprano`
- `cat`

### Scale creation

You can create scales from names such as:

- `C major`
- `A minor`
- `E blues`
- `D dorian`
- `chromatic`
- `whole-tone`
- `quarter-tone`
- `edo19`
- `bohlen-pierce`

By default, `Scale.fromName(...)` uses **A4 = 442 Hz** unless you pass a different tuning explicitly.

Example:

```python
import pytotune

scale = pytotune.Scale.fromName("C major", 440)
```

## CLI Usage

The repository builds an executable named `pytotune_cli`.

### Basic syntax

```bash
./build/pytotune_cli midi <wav_input> <midi_input> <wav_output> [<singer> | <fmin> <fmax>]
./build/pytotune_cli scale <wav_input> <scale_name> <wav_output> [<singer> | <fmin> <fmax>]
```

### Examples

Tune a vocal track to a MIDI file using the default human range:

```bash
./build/pytotune_cli midi input.wav reference.mid output.wav
```

Tune a file to `C major` using a preset vocal range:

```bash
./build/pytotune_cli scale input.wav "C major" output.wav soprano
```

Tune a file to `E minor` using a custom pitch-detection range:

```bash
./build/pytotune_cli scale input.wav "E minor" output.wav 80 900
```

## Testing

### C++ tests (Linux/macOS)

```bash
cd build
ctest --output-on-failure
```

### C++ tests (Windows)

```powershell
cd build
ctest -C Release --output-on-failure
```

### Python bindings test (Linux/macOS)

Run from the repository root:

```bash
export PYTHONPATH=$PYTHONPATH:$(pwd)/build
python test_bindings.py
```

### Python bindings test (Windows)

```powershell
$env:PYTHONPATH = "$env:PYTHONPATH;$(Get-Location)\build\Release"
python test_bindings.py
```

The test data used by the suite lives in `tests/data/`.

## Benchmarking

Benchmark support is enabled only on **Linux**.

### Build a benchmarking configuration

The helper scripts in `benchmarks/` expect a build directory named `cmake-build-relwithdebinfo`.

```bash
cmake -S . -B cmake-build-relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build cmake-build-relwithdebinfo --target pytotune_benchmarks -j
```

### Run helper scripts

Examples:

```bash
bash benchmarks/benchmark_pitch_detection_scale.sh
bash benchmarks/benchmark_pipeline_scale.sh
```

Generated CSV outputs are written into the `benchmarks/` directory.

## Limitations

- The project is primarily designed for **offline processing**, not real-time use
- The current workflow is centered around **WAV input/output** and **MIDI reference files**
- The pitch detector is best suited to **monophonic or voice-like material**
- The repository currently provides **CMake-based local builds**, not a ready-to-publish PyPI package
- Audio quality was not the sole optimization target; the main focus of the project was algorithm engineering and
  performance analysis

## Further Reading

- The full project write-up is available in `paper/paper.tex`. The PDF builds are available as workflow artifacts.
- Test assets and naming conventions are documented in `tests/data/README.md`

## License

This project is licensed under the terms of the `LICENSE` file in the repository.