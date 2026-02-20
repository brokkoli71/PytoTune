# PytoTune Live Mode

Real-time pitch correction for live audio input (microphone).

## Features

- **Real-time processing**: Pitch correction for live vocals (~1 second latency)
- **Two modes**:
  - **Scale mode**: Correct pitch to match a musical scale (e.g., C major, A minor)
  - **Note mode**: Correct pitch to a specific target frequency (e.g., 440 Hz)
- **Simple and full-featured versions**: Choose based on your needs

## Requirements

Install Python dependencies:

```bash
pip install numpy sounddevice
```

## Quick Start

### Simple Version (Recommended)

```bash
# Scale mode (default: C major)
python simple_live.py

# Different scale
python simple_live.py --scale "A minor"

# Single note mode
python simple_live.py --note 440.0
```

### Full-Featured Version

```bash
# Scale mode
python live_autotune.py --scale "C major"

# Single note mode
python live_autotune.py --note 440.0
```

## Usage

### simple_live.py (Recommended)

Direct processing in audio callback - simple and reliable.

```bash
python simple_live.py [options]

Options:
  --scale SCALE         Musical scale (default: C major)
  --note FREQ          Target note frequency in Hz
  --block-size SIZE    Block size in samples (default: 44100)
  --sample-rate RATE   Sample rate in Hz (default: 44100)
```

Examples:
```bash
# C major scale
python simple_live.py

# A minor scale
python simple_live.py --scale "A minor"

# Single note (A4)
python simple_live.py --note 440.0

# Smaller block size (lower latency, may not work as well)
python simple_live.py --scale "C major" --block-size 22050
```

## How It Works

1. **Audio Capture**: Captures audio from your microphone in chunks (blocks)
2. **Pitch Detection**: Uses YIN algorithm to detect the pitch of each block
3. **Pitch Correction**: Applies phase vocoder-based pitch shifting to correct to target
4. **Playback**: Outputs the corrected audio in real-time

### Latency

Total latency = block_size / sample_rate + processing_time

With default settings (44100 samples @ 44.1 kHz):
- Block latency: ~1000 ms (1 second)
- Processing latency: ~10-50 ms (depends on CPU)
- **Total: ~1-1.05 seconds**

Note: Larger block sizes are required for accurate pitch detection. You can try smaller sizes like 22050 (0.5 seconds) but pitch detection may be less reliable.

## API Functions

The following Python functions are now available for array-based processing:

### `tune_array_to_scale(samples, sample_rate, scale)`

Process raw audio samples to match a musical scale.

**Parameters:**
- `samples`: NumPy array of float32 audio samples in range [-1.0, 1.0]
- `sample_rate`: Sample rate in Hz (e.g., 44100)
- `scale`: Scale object created with `pytotune.Scale.from_name()`

**Returns:**
- NumPy array of corrected audio samples

**Example:**
```python
import numpy as np
import pytotune

# Generate test signal
samples = np.sin(2 * np.pi * 440 * np.linspace(0, 1, 44100)).astype(np.float32)

# Create scale
scale = pytotune.Scale.from_name("C major")

# Process
corrected = pytotune.tune_array_to_scale(samples, 44100, scale)
```

### `tune_array_to_note(samples, sample_rate, target_note)`

Process raw audio samples to match a specific target note frequency.

**Parameters:**
- `samples`: NumPy array of float32 audio samples in range [-1.0, 1.0]
- `sample_rate`: Sample rate in Hz (e.g., 44100)
- `target_note`: Target note frequency in Hz (e.g., 440.0 for A4)

**Returns:**
- NumPy array of corrected audio samples

**Example:**
```python
import numpy as np
import pytotune

# Generate test signal
samples = np.sin(2 * np.pi * 440 * np.linspace(0, 1, 44100)).astype(np.float32)

# Process to C4 (261.63 Hz)
corrected = pytotune.tune_array_to_note(samples, 44100, 261.63)
```

## Troubleshooting

### No audio input/output

Check your audio devices:
```bash
python live_autotune.py --list-devices
```

Then specify the device:
```bash
python live_autotune.py --scale "C major" --device <device_id>
```

### High latency

Reduce the block size (may affect stability):
```bash
python live_autotune.py --scale "C major" --block-size 2048
```

### Unstable pitch correction

Increase the block size:
```bash
python live_autotune.py --scale "C major" --block-size 8192
```

### Audio glitches or dropouts

- Close other applications using audio
- Increase block size for more stable processing
- Check CPU usage (processing should be < 50% of one core)

## Notes

- The first few blocks may output silence while the processing pipeline initializes
- Works best with clear, sustained notes (singing, instruments)
- Background noise may affect pitch detection accuracy
- Mono input only (stereo is automatically converted to mono)
