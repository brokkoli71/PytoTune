import sys
import os

# Add build directory to path so we can import the module
sys.path.append(os.path.join(os.getcwd(), 'cmake-build-release'))

import pytotune


def test_scale():
    print("Testing scale tuning (default pitch_range omitted)...")
    wav_path = "tests/data/voice_f440_sr44100.wav"
    out_path = "tests/testoutput/out_scale.wav"

    try:
        scale = pytotune.Scale.fromName("C major")
        # call without pitch_range (should use default HUMAN)
        pytotune.roundToScale(wav_path, scale, out_path)
        print(f"Success! Output saved to {out_path}")
    except Exception as e:
        print(f"Error: {e}")

    # Example: use singerToPitchRange helper
    try:
        pr = pytotune.singerToPitchRange("man")
        out_path2 = "tests/testoutput/out_scale_man.wav"
        pytotune.roundToScale(wav_path, scale, out_path2, pr)
        print(f"Success with singer range! Output saved to {out_path2}")
    except Exception as e:
        print(f"Error using singer range: {e}")


def test_midi():
    print("\nTesting midi tuning (default pitch_range omitted)...")
    wav_path = "tests/data/voice_f440_sr44100.wav"
    midi_path = "tests/data/test.mid"
    out_path = "tests/testoutput/out_midi.wav"

    try:
        # call without pitch_range (should use default HUMAN)
        pytotune.matchMidi(wav_path, midi_path, out_path)
        print(f"Success! Output saved to {out_path}")
    except Exception as e:
        print(f"Error: {e}")

    # Example: pass an explicit numeric PitchRange
    try:
        pr = pytotune.PitchRange(200.0, 800.0)
        out_path2 = "tests/testoutput/out_midi_customrange.wav"
        pytotune.matchMidi(wav_path, midi_path, out_path2, pr)
        print(f"Success with explicit PitchRange! Output saved to {out_path2}")
    except Exception as e:
        print(f"Error with explicit PitchRange: {e}")


if __name__ == "__main__":
    test_midi()
    test_scale()
