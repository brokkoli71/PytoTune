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
# Scale mode (most common)
python live_autotune.py --scale "C major"

# Note mode (single target frequency)
python live_autotune.py --note 440.0

# List audio devices
python live_autotune.py --list-devices

# Use specific device
python live_autotune.py --scale "C major" --device 2
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

### Latency Tuning

```bash
# Lower latency (faster response, less stable)
python live_autotune.py --scale "C major" --block-size 2048

# Higher latency (slower response, more stable)
python live_autotune.py --scale "C major" --block-size 8192
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
python live_autotune.py --list-devices
python live_autotune.py --scale "C major" --device <id>
```

### High latency
```bash
python live_autotune.py --scale "C major" --block-size 2048
```

### Unstable/glitchy
```bash
python live_autotune.py --scale "C major" --block-size 8192
```

## Files

- `live_autotune.py` - Live streaming application
- `example_array_api.py` - API usage examples
- `test_bindings.py` - Unit tests
- `LIVE_MODE.md` - Full documentation
- `IMPLEMENTATION_SUMMARY.md` - Technical details

## Default Parameters

- Sample rate: 44100 Hz
- Block size: 4096 samples
- Window size: 4096 samples (internal)
- Stride: 1024 samples (internal)
- Estimated latency: ~100-115ms
