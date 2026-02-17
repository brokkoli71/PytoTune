#!/usr/bin/env python3
"""
Debug version - shows what PytoTune is returning
"""

import sys
import os
import numpy as np
import sounddevice as sd
import time

sys.path.append(os.path.join(os.path.dirname(__file__), 'cmake-build-release'))
import pytotune

print("Debug Live Autotune - Analyzing Output")
print("=" * 60)

# Configuration
SAMPLE_RATE = 44100
BLOCK_SIZE = 4096

# Create scale
scale = pytotune.Scale.from_name("C major")

callback_count = 0

def audio_callback(indata, outdata, frames, time_info, status):
    """Audio callback with debugging"""
    global callback_count
    callback_count += 1
    
    if status:
        print(f"Status: {status}")
    
    try:
        # Get mono audio
        audio = indata[:, 0] if indata.ndim > 1 else indata.flatten()
        audio = audio.astype(np.float32)
        
        # Check input
        in_rms = np.sqrt(np.mean(audio**2))
        in_max = np.max(np.abs(audio))
        
        # Process with PytoTune
        corrected = pytotune.tune_array_to_scale(audio, SAMPLE_RATE, scale)
        
        # Check output
        out_rms = np.sqrt(np.mean(corrected**2))
        out_max = np.max(np.abs(corrected))
        out_len = len(corrected)
        has_nan = np.any(np.isnan(corrected))
        has_inf = np.any(np.isinf(corrected))
        
        # Log every 10 callbacks
        if callback_count % 10 == 0:
            print(f"Callback {callback_count}:")
            print(f"  Input:  len={len(audio)}, RMS={in_rms:.6f}, max={in_max:.6f}")
            print(f"  Output: len={out_len}, RMS={out_rms:.6f}, max={out_max:.6f}")
            print(f"  Has NaN: {has_nan}, Has Inf: {has_inf}")
            
            if out_max < 0.001:
                print(f"  ⚠ Output is very quiet or silent!")
            if out_len != len(audio):
                print(f"  ⚠ Output length mismatch!")
        
        # Output (convert to stereo if needed)
        if outdata.shape[1] > 1:
            outdata[:, 0] = corrected[:len(outdata)]
            outdata[:, 1] = corrected[:len(outdata)]
        else:
            outdata[:] = corrected[:len(outdata)].reshape(-1, 1)
            
    except Exception as e:
        print(f"✗ Error in callback: {e}")
        import traceback
        traceback.print_exc()
        outdata.fill(0)

print("Starting debug stream...")
print("Speak or sing into your microphone!")
print("Watch the output levels...")
print()

try:
    with sd.Stream(
        samplerate=SAMPLE_RATE,
        blocksize=BLOCK_SIZE,
        channels=1,
        callback=audio_callback,
        dtype=np.float32
    ):
        print("✓ Stream active\n")
        
        # Run for 10 seconds
        for i in range(10):
            time.sleep(1)
            print(f"Running... {i+1}/10")
            
except KeyboardInterrupt:
    print("\n\nStopped by user")
except Exception as e:
    print(f"\n✗ Error: {e}")
    import traceback
    traceback.print_exc()

print("\nDone!")
