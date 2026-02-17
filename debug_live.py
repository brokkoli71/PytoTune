#!/usr/bin/env python3
"""
Debug version of live autotune with verbose logging
"""

import sys
import os
import numpy as np
import sounddevice as sd

sys.path.append(os.path.join(os.path.dirname(__file__), 'cmake-build-release'))
import pytotune

print("=" * 60)
print("LIVE AUTOTUNE DEBUG")
print("=" * 60)

# Test 1: Check audio devices
print("\n1. Audio Devices:")
print("-" * 60)
print(sd.query_devices())
print(f"\nDefault input device: {sd.default.device[0]}")
print(f"Default output device: {sd.default.device[1]}")

# Test 2: Simple audio loopback (no processing)
print("\n2. Testing Simple Audio Loopback (5 seconds)...")
print("-" * 60)
print("Speak into your microphone - you should hear yourself with a slight delay")
print("Starting in 3 seconds...")

import time
time.sleep(3)

try:
    duration = 5  # seconds
    print(f"Recording and playing back for {duration} seconds...")
    
    # Simple callback that just copies input to output
    def loopback_callback(indata, outdata, frames, time, status):
        if status:
            print(f"Status: {status}")
        outdata[:] = indata
    
    with sd.Stream(channels=1, callback=loopback_callback):
        sd.sleep(int(duration * 1000))
    
    print("✓ Loopback test completed")
    
except Exception as e:
    print(f"✗ Loopback test failed: {e}")
    import traceback
    traceback.print_exc()
    print("\nTry specifying a device manually:")
    print("  python live_autotune.py --scale 'C major' --device 5")
    sys.exit(1)

# Test 3: Test PytoTune processing
print("\n3. Testing PytoTune Processing...")
print("-" * 60)

try:
    # Generate test signal
    sample_rate = 44100
    samples = np.sin(2 * np.pi * 440 * np.linspace(0, 0.5, 22050)).astype(np.float32)
    
    scale = pytotune.Scale.from_name("C major")
    corrected = pytotune.tune_array_to_scale(samples, sample_rate, scale)
    
    print(f"✓ PytoTune processing works")
    print(f"  Input: {len(samples)} samples")
    print(f"  Output: {len(corrected)} samples")
    
except Exception as e:
    print(f"✗ PytoTune processing failed: {e}")
    import traceback
    traceback.print_exc()
    sys.exit(1)

# Test 4: Full live autotune with verbose logging
print("\n4. Testing Live Autotune with Verbose Logging...")
print("-" * 60)
print("Starting live autotune for 10 seconds...")
print("Speak or sing into your microphone!")
print()

import queue
import threading

class VerboseLiveAutotune:
    def __init__(self, sample_rate=44100, block_size=4096):
        self.sample_rate = sample_rate
        self.block_size = block_size
        self.scale = pytotune.Scale.from_name("C major")
        
        self.input_queue = queue.Queue()
        self.output_queue = queue.Queue()
        
        self.processing = True
        self.process_thread = threading.Thread(target=self._process_audio)
        
        self.callback_count = 0
        self.process_count = 0
        self.error_count = 0
        
    def _audio_callback(self, indata, outdata, frames, time, status):
        if status:
            print(f"⚠ Audio callback status: {status}")
        
        self.callback_count += 1
        if self.callback_count % 10 == 0:
            print(f"  Callback #{self.callback_count}: {frames} frames, queue size: {self.input_queue.qsize()}/{self.output_queue.qsize()}")
        
        self.input_queue.put(indata.copy())
        
        try:
            processed = self.output_queue.get_nowait()
            outdata[:] = processed
        except queue.Empty:
            outdata.fill(0)
            if self.callback_count > 5:  # Allow time for startup
                print(f"  ⚠ Output queue empty at callback #{self.callback_count}")
    
    def _process_audio(self):
        while self.processing:
            try:
                audio_block = self.input_queue.get(timeout=0.1)
                
                if audio_block.ndim > 1:
                    audio_block = audio_block.mean(axis=1)
                
                audio_block = audio_block.astype(np.float32).flatten()
                
                # Check input level
                rms = np.sqrt(np.mean(audio_block**2))
                if self.process_count % 10 == 0:
                    print(f"  Process #{self.process_count}: RMS level = {rms:.4f}")
                
                corrected = pytotune.tune_array_to_scale(
                    audio_block, 
                    self.sample_rate, 
                    self.scale
                )
                
                corrected = corrected.reshape(-1, 1)
                self.output_queue.put(corrected)
                
                self.process_count += 1
                
            except queue.Empty:
                continue
            except Exception as e:
                self.error_count += 1
                print(f"✗ Processing error #{self.error_count}: {e}")
                if self.error_count <= 3:
                    import traceback
                    traceback.print_exc()
    
    def start(self, duration=10):
        print(f"Starting audio stream (sample_rate={self.sample_rate}, block_size={self.block_size})...")
        
        self.process_thread.start()
        
        try:
            with sd.Stream(
                samplerate=self.sample_rate,
                blocksize=self.block_size,
                channels=1,
                callback=self._audio_callback,
                dtype=np.float32
            ):
                sd.sleep(int(duration * 1000))
                
        except Exception as e:
            print(f"✗ Stream error: {e}")
            import traceback
            traceback.print_exc()
        finally:
            self.stop()
    
    def stop(self):
        self.processing = False
        self.process_thread.join()
        print(f"\nStatistics:")
        print(f"  Callbacks: {self.callback_count}")
        print(f"  Processed: {self.process_count}")
        print(f"  Errors: {self.error_count}")

try:
    autotune = VerboseLiveAutotune()
    autotune.start(duration=10)
    print("\n✓ Live autotune test completed")
    
except KeyboardInterrupt:
    print("\n\n⚠ Interrupted by user")
except Exception as e:
    print(f"\n✗ Test failed: {e}")
    import traceback
    traceback.print_exc()

print("\n" + "=" * 60)
print("Debug complete!")
print("=" * 60)
