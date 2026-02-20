# PytoTune Live Mode - Quick Reference

## Installation

```bash
./setup_live.sh
```

Or manually:
```bash
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release -j$(nproc)
pip install -r requirements_live.txt
```

## Live Autotune Usage

### Basic Commands

```bash
# Simple version (recommended)
python simple_live.py                    # C major (default)
python simple_live.py --scale "A minor"  # Different scale
python simple_live.py --note 440.0       # Single note mode

# Full-featured version
python live_autotune.py --scale "C major"
python live_autotune.py --note 440.0

# List audio devices
python live_autotune.py --list-devices
```

### Common Scales

```bash
--scale "C major"
--scale "C minor"
--scale "A minor"
--scale "E major"
--scale "G major"
--scale "D minor"
```

### Block Size Adjustment

```bash
# Default (1 second latency, most reliable)
python simple_live.py --block-size 44100

# Lower latency (0.5 seconds, may be less reliable)
python simple_live.py --block-size 22050
```

## Python API

### Import
```python
import numpy as np
import pytotune
```

### Scale Mode
```python
# Create audio samples (float32, [-1.0, 1.0])
samples = np.sin(2 * np.pi * 440 * t).astype(np.float32)

# Create scale
scale = pytotune.Scale.from_name("C major")

# Process
corrected = pytotune.tune_array_to_scale(samples, 44100, scale)
```

### Note Mode
```python
# Process to specific frequency
corrected = pytotune.tune_array_to_note(samples, 44100, 440.0)
```

## Testing

```bash
# Test Python bindings
python test_bindings.py

# Run examples
python example_array_api.py
```

## Common Note Frequencies

| Note | Frequency (Hz) |
|------|----------------|
| C4   | 261.63        |
| D4   | 293.66        |
| E4   | 329.63        |
| F4   | 349.23        |
| G4   | 392.00        |
| A4   | 440.00        |
| B4   | 493.88        |
| C5   | 523.25        |

## Troubleshooting

### No audio
```bash
# Use simple version
python simple_live.py
```

### High latency (default is ~1 second)
```bash
# Try 0.5 seconds (may be less reliable)
python simple_live.py --block-size 22050
```

## Files

- `simple_live.py` - Simple live autotune (recommended)
- `live_autotune.py` - Full-featured version with threading
- `example_array_api.py` - API usage examples
- `test_bindings.py` - Unit tests
- `LIVE_MODE.md` - Full documentation
- `IMPLEMENTATION_SUMMARY.md` - Technical details

## Default Parameters

- Sample rate: 44100 Hz
- Block size: 44100 samples (1 second)
- Window size: 8192 samples (internal)
- Stride: 2048 samples (internal)
- Expected latency: ~1 second
