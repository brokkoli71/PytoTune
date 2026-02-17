# Troubleshooting Live Autotune Audio Issues

## Quick Diagnosis

Run these test scripts in order:

### 1. Basic Audio Test
```bash
python test_audio.py
```

This will test:
- ✓ Playback (can you hear a beep?)
- ✓ Recording (is your mic working?)
- ✓ Live monitoring (can you hear yourself?)

### 2. Debug Live Autotune
```bash
python debug_live.py
```

This will show detailed logs of what's happening.

### 3. Check Available Devices
```bash
python live_autotune.py --list-devices
```

## Common Issues & Solutions

### Issue 1: No Sound Output

**Symptoms:** Script runs but you hear nothing

**Diagnosis:**
```bash
python test_audio.py
```

**Solutions:**

1. **Check system audio settings**
   - Make sure speakers/headphones are not muted
   - Check volume level in system settings
   - Verify correct output device is selected

2. **Try specific device**
   ```bash
   # List devices first
   python live_autotune.py --list-devices
   
   # Use device ID (e.g., device 0 for input, device 0 for output)
   python live_autotune.py --scale "C major" --device 0
   ```

3. **JACK Audio users**
   ```bash
   # Try JACK device directly
   sd.default.device = [15, 18]  # Logitech headset in your case
   ```

4. **PulseAudio/PipeWire users**
   ```bash
   # Use pipewire device
   python live_autotune.py --scale "C major" --device 12
   ```

### Issue 2: No Input from Microphone

**Symptoms:** Script runs but doesn't respond to voice

**Diagnosis:**
```bash
python test_audio.py
```
Look for "RMS level" - should be > 0.01 when speaking

**Solutions:**

1. **Check microphone is not muted**
   - System settings → Audio → Input
   - Check physical mute button on headset

2. **Check microphone level**
   ```bash
   alsamixer  # or pavucontrol for PulseAudio
   ```

3. **Try different input device**
   ```bash
   # Your Logitech headset is device 5
   python live_autotune.py --scale "C major" --device 5
   ```

4. **Grant microphone permissions**
   - Check if your system requires permission to access mic

### Issue 3: Audio Glitches/Dropouts

**Symptoms:** Stuttering, clicking, or interrupted audio

**Solutions:**

1. **Increase block size** (reduces CPU load)
   ```bash
   python live_autotune.py --scale "C major" --block-size 8192
   ```

2. **Close other audio applications**
   - Stop music players, browsers, etc.

3. **Check CPU usage**
   ```bash
   top
   # PytoTune should use < 50% of one core
   ```

4. **Disable CPU frequency scaling** (if needed)
   ```bash
   sudo cpupower frequency-set -g performance
   ```

### Issue 4: High Latency

**Symptoms:** Long delay between speaking and hearing output

**Solutions:**

1. **Reduce block size**
   ```bash
   python live_autotune.py --scale "C major" --block-size 2048
   ```

2. **Use JACK for lowest latency**
   - Configure JACK with lower buffer size
   - Use JACK device in live_autotune

### Issue 5: Script Crashes or Errors

**Symptoms:** Python errors, crashes

**Diagnosis:**
```bash
python debug_live.py
```

**Solutions:**

1. **Check all dependencies are installed**
   ```bash
   pip install -r requirements_live.txt
   ```

2. **Rebuild C++ library**
   ```bash
   cmake --build cmake-build-release --clean-first
   ```

3. **Check error messages**
   - Look for "ALSA lib" errors → ALSA configuration issue
   - Look for "PortAudio" errors → Audio backend issue
   - Look for "No such device" → Wrong device ID

### Issue 6: Permission Denied

**Symptoms:** "Cannot open audio device" error

**Solutions:**

1. **Add user to audio group**
   ```bash
   sudo usermod -a -G audio $USER
   # Log out and back in
   ```

2. **Check device permissions**
   ```bash
   ls -l /dev/snd/
   ```

## Your Specific Setup

Based on your system, you have:

**Input devices:**
- Device 0: HDA Intel PCH (built-in mic) - 2 channels
- Device 5: PRO X USB Audio (Logitech headset) - 1 channel
- Device 15: Logitech G PRO X Mono (JACK) - 1 channel
- Device 16: Built-in Audio (JACK) - 4 channels
- Device 18: Logitech G PRO X Stereo (JACK) - 2 channels

**Output devices:**
- Device 0: HDA Intel PCH (built-in) - 2 channels
- Device 1: U28E590 (monitor speakers) - 2 channels
- Device 16: Built-in Audio (JACK) - 2 channels
- Device 18: Logitech G PRO X (JACK) - 2 channels

### Recommended configurations:

**For Logitech headset (USB):**
```bash
python live_autotune.py --scale "C major" --device 5
```

**For Logitech headset (via JACK):**
```bash
python live_autotune.py --scale "C major" --device 18
```

**For built-in audio:**
```bash
python live_autotune.py --scale "C major" --device 0
```

## Advanced Debugging

### Check sounddevice configuration:
```python
import sounddevice as sd
print(sd.query_devices())
print(f"Default: {sd.default.device}")
print(f"Hostapis: {sd.query_hostapis()}")
```

### Test specific device manually:
```python
import sounddevice as sd
import numpy as np

# Set specific device
sd.default.device = [5, 0]  # Input 5 (Logitech), Output 0 (built-in)

# Test
duration = 2
fs = 44100
recording = sd.rec(int(duration * fs), samplerate=fs, channels=1)
sd.wait()
print(f"Recorded level: {np.max(np.abs(recording))}")

sd.play(recording, fs)
sd.wait()
```

### Enable verbose ALSA output:
```bash
export ALSA_VERBOSE=1
python live_autotune.py --scale "C major"
```

## Still Not Working?

1. **Run the debug script and send output:**
   ```bash
   python debug_live.py 2>&1 | tee debug_output.txt
   ```

2. **Check system audio:**
   ```bash
   # ALSA
   aplay -l
   arecord -l
   
   # PulseAudio
   pactl list sinks
   pactl list sources
   
   # JACK
   jack_lsp
   ```

3. **Test with simple sounddevice example:**
   ```bash
   python -m sounddevice
   ```

## Contact

If none of these solutions work, please provide:
1. Output from `python test_audio.py`
2. Output from `python debug_live.py`
3. Your audio system (ALSA/PulseAudio/JACK/PipeWire)
4. Operating system and version
