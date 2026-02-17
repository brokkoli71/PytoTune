# PytoTune Live Mode

Real-time pitch correction for live audio input (microphone).

## Features

- **Real-time processing**: Low-latency pitch correction for live vocals
- **Two modes**:
  - **Scale mode**: Correct pitch to match a musical scale (e.g., C major, A minor)
  - **Note mode**: Correct pitch to a specific target frequency (e.g., 440 Hz)
- **Configurable latency**: Adjust block size to balance latency vs. stability

## Requirements

Install Python dependencies:

```bash
pip install numpy sounddevice
```

## Usage

### Quick Start

Correct to C major scale:
```bash
python live_autotune.py --scale "C major"
```

Correct to a specific note (A4 = 440 Hz):
```bash
python live_autotune.py --note 440.0
```

### Options

```bash
python live_autotune.py --help
```

**Mode selection** (required, choose one):
- `--scale SCALE`: Musical scale name (e.g., "C major", "A minor", "E major")
- `--note FREQ`: Target note frequency in Hz (e.g., 440.0 for A4)

**Audio settings** (optional):
- `--sample-rate RATE`: Sample rate in Hz (default: 44100)
- `--block-size SIZE`: Processing block size in samples (default: 4096)
  - Smaller = lower latency but potentially less stable
  - Larger = higher latency but more stable pitch detection
- `--device ID`: Input device ID (use `--list-devices` to see options)
- `--list-devices`: List available audio devices and exit

### Examples

Use a different scale:
```bash
python live_autotune.py --scale "A minor"
```

Lower latency (smaller block size):
```bash
python live_autotune.py --scale "C major" --block-size 2048
```

Select specific audio device:
```bash
# First, list devices
python live_autotune.py --list-devices

# Then use the device ID
python live_autotune.py --scale "C major" --device 2
```

## How It Works

1. **Audio Capture**: Captures audio from your microphone in chunks (blocks)
2. **Pitch Detection**: Uses YIN algorithm to detect the pitch of each block
3. **Pitch Correction**: Applies phase vocoder-based pitch shifting to correct to target
4. **Playback**: Outputs the corrected audio in real-time

### Latency

Total latency = block_size / sample_rate + processing_time

With default settings (4096 samples @ 44.1 kHz):
- Block latency: ~93 ms
- Processing latency: ~5-20 ms (depends on CPU)
- **Total: ~100-115 ms**

For lower latency, reduce block size:
- 2048 samples: ~46 ms block latency
- 1024 samples: ~23 ms block latency (may be less stable)

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
