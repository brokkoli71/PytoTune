import sys
import os

# Try to find the build directory
possible_build_dirs = ['cmake-build-debug', 'cmake-build-release', 'cmake-build-relwithdebinfo', 'build']
found = False

print(f"Current working directory: {os.getcwd()}")
print("Searching for pytotune module...")

possible_subdirs = ['', 'Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel']

for d in possible_build_dirs:
    base_path = os.path.join(os.getcwd(), d)
    print(f"Checking {base_path}...")
    if os.path.exists(base_path):
        print(f"  Directory exists: {base_path}")

        # Check subdirectories (including root)
        for subdir in possible_subdirs:
            path = os.path.join(base_path, subdir) if subdir else base_path

            if not os.path.exists(path):
                 continue

            print(f"    Scanning {path}...")
            # Check if the module is in there (look for .so or .pyd)
            try:
                files = os.listdir(path)
                # print(f"    Files: {files}") # Reduce verbosity if needed, or keep for debugging
                for f in files:
                    if f.startswith('pytotune') and (f.endswith('.so') or f.endswith('.pyd')):
                        sys.path.append(path)
                        print(f"Found pytotune in {path}")
                        found = True
                        break
            except OSError as e:
                print(f"    Error reading {path}: {e}")

            if found:
                break
    else:
        print(f"  Directory does not exist: {base_path}")

    if found:
        break

import pytotune


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


if __name__ == "__main__":
    test_midi()
    test_scale()
