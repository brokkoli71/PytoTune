#!/usr/bin/env python3
"""
Quick audio test - checks if you can hear anything
"""

import sounddevice as sd
import numpy as np
import time

print("Audio Device Test")
print("=" * 60)

# Show devices
print("\nAvailable devices:")
print(sd.query_devices())

# Test 1: Play a beep
print("\n" + "=" * 60)
print("TEST 1: Playing a 440Hz beep for 1 second")
print("You should hear a tone through your speakers/headphones")
print("=" * 60)
time.sleep(1)

try:
    duration = 1.0
    sample_rate = 44100
    frequency = 440.0
    
    t = np.linspace(0, duration, int(sample_rate * duration))
    beep = 0.3 * np.sin(2 * np.pi * frequency * t).astype(np.float32)
    
    sd.play(beep, sample_rate)
    sd.wait()
    
    print("✓ Playback test completed")
    
except Exception as e:
    print(f"✗ Playback failed: {e}")
    print("\nTry specifying an output device:")
    print("  sd.default.device = [input_device, output_device]")

# Test 2: Record and playback
print("\n" + "=" * 60)
print("TEST 2: Recording for 2 seconds, then playing back")
print("Say something into your microphone!")
print("=" * 60)
print("Recording in 2 seconds...")
time.sleep(2)
print("Recording NOW...")

try:
    duration = 2.0
    sample_rate = 44100
    
    recording = sd.rec(int(duration * sample_rate), 
                       samplerate=sample_rate, 
                       channels=1, 
                       dtype=np.float32)
    sd.wait()
    
    print(f"✓ Recorded {len(recording)} samples")
    print(f"  RMS level: {np.sqrt(np.mean(recording**2)):.4f}")
    print(f"  Max level: {np.max(np.abs(recording)):.4f}")
    
    print("\nPlaying back in 1 second...")
    time.sleep(1)
    
    sd.play(recording, sample_rate)
    sd.wait()
    
    print("✓ Playback completed")
    
    if np.max(np.abs(recording)) < 0.001:
        print("\n⚠ WARNING: Input level is very low!")
        print("  Check if your microphone is:")
        print("  - Connected properly")
        print("  - Not muted")
        print("  - Selected as input device in your system settings")
    
except Exception as e:
    print(f"✗ Record/playback failed: {e}")
    import traceback
    traceback.print_exc()

# Test 3: Live monitoring (loopback)
print("\n" + "=" * 60)
print("TEST 3: Live monitoring (5 seconds)")
print("You should hear yourself with a slight delay")
print("=" * 60)
print("Starting in 2 seconds...")
time.sleep(2)

try:
    def callback(indata, outdata, frames, time, status):
        if status:
            print(status)
        outdata[:] = indata
    
    print("Monitoring... speak into your microphone!")
    with sd.Stream(channels=1, callback=callback):
        sd.sleep(5000)
    
    print("✓ Live monitoring completed")
    
except Exception as e:
    print(f"✗ Live monitoring failed: {e}")
    import traceback
    traceback.print_exc()

print("\n" + "=" * 60)
print("Test complete!")
print("=" * 60)
print("\nIf you didn't hear anything:")
print("1. Check your system audio settings")
print("2. Make sure your speakers/headphones are connected")
print("3. Try a different device with:")
print("   sd.default.device = [input_id, output_id]")
print("4. Check ALSA/PulseAudio/JACK configuration")
