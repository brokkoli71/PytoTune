# PytoTune

**Cloud Rap Auto-Tune Library with Real-Time Processing**

## Features

- 🎵 **File-based processing**: Pitch-correct audio files using MIDI or musical scales
- 🎤 **Live mode**: Real-time pitch correction from microphone input (~100ms latency)
- 🐍 **Python API**: Easy-to-use NumPy-based interface
- ⚡ **Low latency**: Optimized C++ core with Python bindings
- 🎹 **Flexible tuning**: Scale-based or single-note correction

## Quick Start

### Installation
```bash
# Build C++ library and install Python dependencies
./setup_live.sh
```

### Live Mode (Real-time)
```bash
# Correct microphone input to C major scale
python live_autotune.py --scale "C major"

# Correct to specific note (A4 = 440 Hz)
python live_autotune.py --note 440.0
```

### File Processing
```python
import pytotune

# Correct a WAV file to a scale
scale = pytotune.Scale.from_name("C major")
pytotune.tune_to_scale("input.wav", scale, "output.wav")

# Correct using MIDI reference
pytotune.tune_to_midi("vocals.wav", "melody.mid", "output.wav")
```

### Array Processing (Custom Applications)
```python
import numpy as np
import pytotune

# Process raw audio samples
samples = np.sin(2 * np.pi * 440 * t).astype(np.float32)
scale = pytotune.Scale.from_name("C major")
corrected = pytotune.tune_array_to_scale(samples, 44100, scale)
```

## Documentation

- **[LIVE_MODE.md](LIVE_MODE.md)** - Complete guide for real-time processing
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - Quick reference card
- **[IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)** - Technical details

## Examples

```bash
# Run array API examples
python example_array_api.py

# Test Python bindings
python test_bindings.py
```

## Goal

### Input
There are 3 possible inputs:
- **Files**: Paths to a sound file and a MIDI file with options (e.g., what happens if MIDI has a pause or is polyphonic)
- **Files + Scale**: Path to a sound file and a scale (array of relative pitches to the tuning pitch, e.g., "C major") and a tuning (the pitch of A4, e.g., 440 Hz)
- **Live Audio**: Raw audio samples from microphone or other source (NEW!)

### Output
The pitch-corrected sound (file or array of samples).

### Motivation
We want to learn how pitch correction works in detail and how it can be efficiently optimized.
But most importantly, we cannot sing and want to create outstanding cloud rap.

### Keywords
cloud rap, auto-tune, music, python library, real-time, live mode, deluxe, parallelized, open source, audio