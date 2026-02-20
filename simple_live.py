#!/usr/bin/env python3
"""
Simplified live autotune - Real-time pitch correction

Usage:
    python simple_live.py                    # Scale mode (C major)
    python simple_live.py --scale "A minor"  # Different scale
    python simple_live.py --note 440.0       # Single note mode (A4)
"""

import sys
import os
import argparse
import numpy as np
import sounddevice as sd
import time

sys.path.append(os.path.join(os.path.dirname(__file__), 'cmake-build-release'))
import pytotune

def main():
    parser = argparse.ArgumentParser(description='Simple Live Autotune')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--scale', type=str, default='C major', help='Musical scale (default: C major)')
    group.add_argument('--note', type=float, help='Target note frequency in Hz (e.g., 440.0)')
    parser.add_argument('--block-size', type=int, default=44100, help='Block size in samples (default: 44100)')
    parser.add_argument('--sample-rate', type=int, default=44100, help='Sample rate (default: 44100)')
    
    args = parser.parse_args()

    # Configuration
    SAMPLE_RATE = args.sample_rate
    BLOCK_SIZE = args.block_size
    
    # Determine mode
    if args.note:
        mode = 'note'
        target_note = args.note
        print(f"Mode: Single Note ({target_note} Hz)")
    else:
        mode = 'scale'
        scale = pytotune.Scale.from_name(args.scale)
        print(f"Mode: Scale ({args.scale})")
    
    print(f"Sample rate: {SAMPLE_RATE} Hz")
    print(f"Block size: {BLOCK_SIZE} samples ({BLOCK_SIZE/SAMPLE_RATE*1000:.0f} ms)")
    print()

    def audio_callback(indata, outdata, frames, time_info, status):
        """Audio callback - processes in real-time"""
        if status:
            print(f"Status: {status}")
        
        try:
            # Get mono audio
            audio = indata[:, 0] if indata.ndim > 1 else indata.flatten()
            audio = audio.astype(np.float32)
            
            # Process with PytoTune
            if mode == 'note':
                corrected = pytotune.tune_array_to_note(audio, SAMPLE_RATE, target_note)
            else:
                corrected = pytotune.tune_array_to_scale(audio, SAMPLE_RATE, scale)
            
            # Output
            if outdata.shape[1] > 1:
                outdata[:, 0] = corrected[:len(outdata)]
                outdata[:, 1] = corrected[:len(outdata)]
            else:
                outdata[:] = corrected[:len(outdata)].reshape(-1, 1)
                
        except Exception as e:
            print(f"Error: {e}")
            outdata.fill(0)

    print("Starting live autotune...")
    print("Speak or sing into your microphone!")
    print("Press Ctrl+C to stop")
    print()

    try:
        with sd.Stream(
            samplerate=SAMPLE_RATE,
            blocksize=BLOCK_SIZE,
            channels=1,
            callback=audio_callback,
            dtype=np.float32
        ):
            print("✓ Audio stream active\n")
            while True:
                time.sleep(0.1)
                
    except KeyboardInterrupt:
        print("\n\nStopped by user")
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()

    print("\nDone!")

if __name__ == '__main__':
    main()
