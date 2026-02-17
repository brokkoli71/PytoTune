#!/usr/bin/env python3
"""
Simplified live autotune - easier to debug
"""

import sys
import os
import numpy as np
import sounddevice as sd
import time

sys.path.append(os.path.join(os.path.dirname(__file__), 'cmake-build-release'))
import pytotune

print("Simple Live Autotune")
print("=" * 60)

# Configuration
SAMPLE_RATE = 44100
BLOCK_SIZE = 44100

# Create scale
scale = pytotune.Scale.from_name("C major")
print(f"Scale: C major")
print(f"Sample rate: {SAMPLE_RATE} Hz")
print(f"Block size: {BLOCK_SIZE} samples")
print(f"Latency: ~{BLOCK_SIZE/SAMPLE_RATE*1000:.0f} ms")
print()

# Shared buffer for processed audio
processed_buffer = None
processing_enabled = True

def audio_callback(indata, outdata, frames, time_info, status):
    """Audio callback - called by sounddevice"""
    global processed_buffer
    
    if status:
        print(f"Status: {status}")
    
    try:
        # Get mono audio
        audio = indata[:, 0] if indata.ndim > 1 else indata.flatten()
        audio = audio.astype(np.float32)
        
        # Process with PytoTune
        if processing_enabled:
            corrected = pytotune.tune_array_to_scale(audio, SAMPLE_RATE, scale)
        else:
            corrected = audio  # Pass through
        
        # Output (convert to stereo if needed)
        if outdata.shape[1] > 1:
            outdata[:, 0] = corrected[:len(outdata)]
            outdata[:, 1] = corrected[:len(outdata)]
        else:
            outdata[:] = corrected[:len(outdata)].reshape(-1, 1)
            
    except Exception as e:
        print(f"Error in callback: {e}")
        outdata.fill(0)

print("Starting live autotune...")
print("Speak or sing into your microphone!")
print("Press Ctrl+C to stop")
print()

try:
    # Open audio stream
    with sd.Stream(
        samplerate=SAMPLE_RATE,
        blocksize=BLOCK_SIZE,
        channels=1,
        callback=audio_callback,
        dtype=np.float32
    ):
        print("✓ Audio stream active")
        print()
        
        # Keep running
        while True:
            time.sleep(0.1)
            
except KeyboardInterrupt:
    print("\n\nStopped by user")
except Exception as e:
    print(f"\n✗ Error: {e}")
    import traceback
    traceback.print_exc()

print("\nDone!")
