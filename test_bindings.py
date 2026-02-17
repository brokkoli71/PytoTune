import sys
import os

sys.path.append(os.path.join(os.getcwd(), 'cmake-build-release'))

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
