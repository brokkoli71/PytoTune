#!/usr/bin/env python3
"""
PytoTune Live Autotune - Real-time pitch correction for microphone input

This script captures audio from your microphone, applies pitch correction 
in real-time, and plays the corrected audio back.

Requirements:
    - sounddevice (pip install sounddevice)
    - numpy (pip install numpy)

Usage:
    python live_autotune.py --scale "C major"
    python live_autotune.py --note 440.0
"""

import sys
import os
import argparse
import numpy as np
import sounddevice as sd
import queue
import threading

# Add build directory to path
sys.path.append(os.path.join(os.path.dirname(__file__), 'cmake-build-release'))

import pytotune


class LiveAutotune:
    def __init__(self, sample_rate=44100, block_size=4096, mode='scale', scale_name='C major', target_note=440.0):
        """
        Initialize the live autotune processor.
        
        Args:
            sample_rate: Audio sample rate (Hz)
            block_size: Processing block size (samples) - affects latency
            mode: 'scale' or 'note'
            scale_name: Musical scale name (e.g., 'C major', 'A minor')
            target_note: Target note frequency in Hz (for 'note' mode)
        """
        self.sample_rate = sample_rate
        self.block_size = block_size
        self.mode = mode
        self.target_note = target_note
        
        # Create scale object
        if mode == 'scale':
            self.scale = pytotune.Scale.from_name(scale_name)
            print(f"Using scale: {scale_name}")
        else:
            print(f"Using target note: {target_note} Hz")
        
        # Audio queues for input/output
        self.input_queue = queue.Queue()
        self.output_queue = queue.Queue()
        
        # Processing thread
        self.processing = True
        self.process_thread = threading.Thread(target=self._process_audio)
        
        # Latency calculation
        latency_ms = (block_size / sample_rate) * 1000
        print(f"Block size: {block_size} samples")
        print(f"Estimated latency: {latency_ms:.1f} ms")
        
    def _audio_callback(self, indata, outdata, frames, time, status):
        """Audio callback for sounddevice stream."""
        if status:
            print(f"Audio status: {status}")
        
        # Put input data in queue
        self.input_queue.put(indata.copy())
        
        # Get processed data from queue (or silence if not ready)
        try:
            processed = self.output_queue.get_nowait()
            outdata[:] = processed
        except queue.Empty:
            outdata.fill(0)  # Output silence if no processed data available
    
    def _process_audio(self):
        """Processing thread that applies pitch correction."""
        while self.processing:
            try:
                # Get input audio (with timeout to allow clean shutdown)
                audio_block = self.input_queue.get(timeout=0.1)
                
                # Convert to mono if stereo
                if audio_block.ndim > 1:
                    audio_block = audio_block.mean(axis=1)
                
                # Convert to float32
                audio_block = audio_block.astype(np.float32).flatten()
                
                # Apply pitch correction
                if self.mode == 'scale':
                    corrected = pytotune.tune_array_to_scale(
                        audio_block, 
                        self.sample_rate, 
                        self.scale
                    )
                else:
                    corrected = pytotune.tune_array_to_note(
                        audio_block,
                        self.sample_rate,
                        self.target_note
                    )
                
                # Reshape for output (mono to stereo if needed)
                corrected = corrected.reshape(-1, 1)
                
                # Put in output queue
                self.output_queue.put(corrected)
                
            except queue.Empty:
                continue
            except Exception as e:
                print(f"Processing error: {e}")
                import traceback
                traceback.print_exc()
    
    def start(self):
        """Start the live autotune processing."""
        print("\nStarting live autotune...")
        print("Speak or sing into your microphone!")
        print("Press Ctrl+C to stop.\n")
        
        # Start processing thread
        self.process_thread.start()
        
        try:
            # Start audio stream
            with sd.Stream(
                samplerate=self.sample_rate,
                blocksize=self.block_size,
                channels=1,
                callback=self._audio_callback,
                dtype=np.float32
            ):
                # Keep running until interrupted
                while True:
                    sd.sleep(100)
                    
        except KeyboardInterrupt:
            print("\nStopping...")
        finally:
            self.stop()
    
    def stop(self):
        """Stop the live autotune processing."""
        self.processing = False
        self.process_thread.join()
        print("Stopped.")


def list_audio_devices():
    """List available audio input devices."""
    print("Available audio devices:")
    print(sd.query_devices())


def main():
    parser = argparse.ArgumentParser(
        description='PytoTune Live Autotune - Real-time pitch correction'
    )
    
    # Mode selection
    mode_group = parser.add_mutually_exclusive_group(required=True)
    mode_group.add_argument(
        '--scale',
        type=str,
        help='Musical scale name (e.g., "C major", "A minor", "E major")'
    )
    mode_group.add_argument(
        '--note',
        type=float,
        help='Target note frequency in Hz (e.g., 440.0 for A4)'
    )
    
    # Audio settings
    parser.add_argument(
        '--sample-rate',
        type=int,
        default=44100,
        help='Sample rate in Hz (default: 44100)'
    )
    parser.add_argument(
        '--block-size',
        type=int,
        default=4096,
        help='Processing block size in samples (default: 4096). Smaller = lower latency but less stable.'
    )
    parser.add_argument(
        '--list-devices',
        action='store_true',
        help='List available audio devices and exit'
    )
    parser.add_argument(
        '--device',
        type=int,
        help='Input device ID (use --list-devices to see options)'
    )
    
    args = parser.parse_args()
    
    # List devices if requested
    if args.list_devices:
        list_audio_devices()
        return
    
    # Set device if specified
    if args.device is not None:
        sd.default.device = args.device
    
    # Determine mode and parameters
    if args.scale:
        mode = 'scale'
        scale_name = args.scale
        target_note = None
    else:
        mode = 'note'
        scale_name = None
        target_note = args.note
    
    # Create and start autotune
    autotune = LiveAutotune(
        sample_rate=args.sample_rate,
        block_size=args.block_size,
        mode=mode,
        scale_name=scale_name if scale_name else 'C major',
        target_note=target_note if target_note else 440.0
    )
    
    autotune.start()


if __name__ == '__main__':
    main()
