import sys
import os

# Add build directory to path so we can import the module
sys.path.append(os.path.join(os.getcwd(), 'cmake-build-release'))

import pytotune
import numpy as np


def test_scale():
    print("Testing scale tuning...")
    wav_path = "tests/data/voice_f440_sr44100.wav"
    out_path = "tests/testoutput/out_scale.wav"

    # Tuning A4 to 440Hz, Scale C Major
    try:
        scale = pytotune.Scale.from_name("C major")
        pytotune.tune_to_scale(wav_path, scale, out_path)
        print(f"Success! Output saved to {out_path}")
    except Exception as e:
        print(f"Error: {e}")


def test_midi():
    print("\nTesting midi tuning...")
    wav_path = "tests/data/voice_f440_sr44100.wav"
    midi_path = "tests/data/test.mid"
    out_path = "tests/testoutput/out_midi.wav"

    try:
        pytotune.tune_to_midi(wav_path, midi_path, out_path)
        print(f"Success! Output saved to {out_path}")
    except Exception as e:
        print(f"Error: {e}")


def test_array_to_scale():
    print("\nTesting array-based scale tuning...")
    try:
        # Generate a simple test signal: 440Hz sine wave
        sample_rate = 44100
        duration = 0.5  # seconds
        frequency = 440.0
        
        t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
        samples = np.sin(2 * np.pi * frequency * t).astype(np.float32)
        
        # Create scale
        scale = pytotune.Scale.from_name("C major")
        
        # Process array
        result = pytotune.tune_array_to_scale(samples, sample_rate, scale)
        
        print(f"Success! Processed {len(samples)} samples -> {len(result)} samples")
        print(f"Input shape: {samples.shape}, Output shape: {result.shape}")
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()


def test_array_to_note():
    print("\nTesting array-based note tuning...")
    try:
        # Generate a simple test signal: 440Hz sine wave
        sample_rate = 44100
        duration = 0.5  # seconds
        frequency = 440.0
        
        t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
        samples = np.sin(2 * np.pi * frequency * t).astype(np.float32)
        
        # Tune to C4 (261.63 Hz)
        target_note = 261.63
        
        # Process array
        result = pytotune.tune_array_to_note(samples, sample_rate, target_note)
        
        print(f"Success! Processed {len(samples)} samples -> {len(result)} samples")
        print(f"Input shape: {samples.shape}, Output shape: {result.shape}")
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    test_midi()
    test_scale()
    test_array_to_scale()
    test_array_to_note()
