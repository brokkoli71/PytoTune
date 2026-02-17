#!/usr/bin/env python3
"""
Simple example demonstrating array-based pitch correction without live streaming.

This example shows how to use the new array-based API functions to process
audio data without dealing with WAV files.
"""

import sys
import os
import numpy as np

# Add build directory to path
sys.path.append(os.path.join(os.path.dirname(__file__), 'cmake-build-release'))

import pytotune


def example_scale_correction():
    """Example: Correct a generated tone to a musical scale."""
    print("Example 1: Scale-based correction")
    print("-" * 50)
    
    # Generate a 440Hz sine wave (A4) for 0.5 seconds
    sample_rate = 44100
    duration = 0.5
    frequency = 440.0  # A4
    
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    samples = np.sin(2 * np.pi * frequency * t).astype(np.float32)
    
    print(f"Input: {frequency} Hz sine wave")
    print(f"Duration: {duration} seconds")
    print(f"Samples: {len(samples)}")
    
    # Create a C major scale (A is not in C major, so it will be corrected)
    scale = pytotune.Scale.from_name("C major")
    print(f"Target scale: C major")
    
    # Process the audio
    corrected = pytotune.tune_array_to_scale(samples, sample_rate, scale)
    
    print(f"Output samples: {len(corrected)}")
    print(f"Closest note in C major to A440: {scale.closest_pitch(440.0):.2f} Hz")
    print()


def example_note_correction():
    """Example: Correct a generated tone to a specific target note."""
    print("Example 2: Single note correction")
    print("-" * 50)
    
    # Generate a 450Hz sine wave (slightly sharp) for 0.5 seconds
    sample_rate = 44100
    duration = 0.5
    frequency = 450.0  # Slightly sharp
    
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    samples = np.sin(2 * np.pi * frequency * t).astype(np.float32)
    
    print(f"Input: {frequency} Hz sine wave (slightly sharp)")
    print(f"Duration: {duration} seconds")
    print(f"Samples: {len(samples)}")
    
    # Target: A4 = 440Hz
    target_note = 440.0
    print(f"Target note: {target_note} Hz (A4)")
    
    # Process the audio
    corrected = pytotune.tune_array_to_note(samples, sample_rate, target_note)
    
    print(f"Output samples: {len(corrected)}")
    print(f"Correction: {frequency} Hz -> {target_note} Hz")
    print()


def example_chunk_processing():
    """Example: Process audio in chunks (simulating streaming)."""
    print("Example 3: Chunk-based processing (streaming simulation)")
    print("-" * 50)
    
    # Generate a longer tone
    sample_rate = 44100
    duration = 2.0
    frequency = 440.0
    
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    full_signal = np.sin(2 * np.pi * frequency * t).astype(np.float32)
    
    # Process in chunks
    chunk_size = 4096  # Same as default window size
    scale = pytotune.Scale.from_name("C major")
    
    print(f"Total samples: {len(full_signal)}")
    print(f"Chunk size: {chunk_size}")
    print(f"Number of chunks: {len(full_signal) // chunk_size}")
    
    corrected_chunks = []
    
    for i in range(0, len(full_signal) - chunk_size, chunk_size):
        chunk = full_signal[i:i + chunk_size]
        corrected_chunk = pytotune.tune_array_to_scale(chunk, sample_rate, scale)
        corrected_chunks.append(corrected_chunk)
        
        if i == 0:
            print(f"Processing chunk {len(corrected_chunks)}: {len(chunk)} samples -> {len(corrected_chunk)} samples")
    
    print(f"Processed {len(corrected_chunks)} chunks")
    print(f"Total output samples: {sum(len(c) for c in corrected_chunks)}")
    print()


def example_different_scales():
    """Example: Compare different scales."""
    print("Example 4: Different musical scales")
    print("-" * 50)
    
    # Generate a 330Hz tone (E4)
    sample_rate = 44100
    duration = 0.5
    frequency = 330.0
    
    t = np.linspace(0, duration, int(sample_rate * duration), dtype=np.float32)
    samples = np.sin(2 * np.pi * frequency * t).astype(np.float32)
    
    print(f"Input: {frequency} Hz (E4)")
    
    scales = ["C major", "C minor", "A minor", "E major"]
    
    for scale_name in scales:
        scale = pytotune.Scale.from_name(scale_name)
        corrected = pytotune.tune_array_to_scale(samples, sample_rate, scale)
        closest = scale.closest_pitch(frequency)
        print(f"  {scale_name:12s}: closest pitch = {closest:.2f} Hz")
    
    print()


if __name__ == '__main__':
    print("=" * 50)
    print("PytoTune Array-Based API Examples")
    print("=" * 50)
    print()
    
    example_scale_correction()
    example_note_correction()
    example_chunk_processing()
    example_different_scales()
    
    print("=" * 50)
    print("All examples completed!")
    print()
    print("For live streaming, see: live_autotune.py")
    print("  python live_autotune.py --scale \"C major\"")
