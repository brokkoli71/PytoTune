# Thoughts

Enter your thought here

## Frames

- mulitple of chache line (to not false share)
- should be aligned, such that simd is possible

## Intervall search for midi

TODO

## Pitch detection

possible improvement of YIN:

- sliding window for cumulative mean normalized difference function (O(n) instead of O(n * tau_max))
- might remove refinement via quadratic interpolation

## Goal pipeline

### With midi

```c++
MidiFile midiFile = MidiFile::load();
WavFile wavFile = WavFile::load();
PitchCorrectionPipeline pcp{};

WavFile result = pcp.matchMidi(wavFile, midiFile);
result.store();
```

```c++
matchMidi(WavFile wavFile, MidiFile midiFle) {
    Windowing windowing; // default
    windowing.sampleRate = wavFile.sampleRate;
    
    SilenceDetector sd{};
    WindowedData silenceMask = sd.process(wavFile, windowing);
    
    YinPitchDetection ypd{};
    WindowedData pitches = ypd.process(wavFile, silenceMask, windowing);
    
    // Map pitches and targets (from midi file) to factos
    WindowedData targetPitches = midi.toWindowedPitches();
    WindowedData pitchCorrectionFactors = targetPitches / pitches;
    
    PitchShifter ps{};
    return WavFile(ps.process(wavFile, pitchCorrectionFactors));
}
```

### Only Pitch Correction

```c++
MidiFile midiFile = MidiFile::load();
Scale scale{};
PitchCorrectionPipeline pcp{};

WavFile result = pcp.roundToScale(wavFile, scale);
result.store();
```

```c++
roundToScale(WavFile wavFile, Scale scale) {
    Windowing windowing; // default
    windowing.sampleRate = wavFile.sampleRate;
    
    SilenceDetector sd{};
    WindowedData silenceMask = sd.process(wavFile, windowing);
    
    YinPitchDetection ypd{};
    WindowedData pitches = ypd.process(wavFile, silenceMask, windowing);
    
    // Map pitches and targets (from scale) to factos
    WindowedData targetPitches = scale.getClosestPitch(pitches);
    WindowedData pitchCorrectionFactors = targetPitches / pitches;
    
    PitchShifter ps{};
    return WavFile(ps.process(wavFile, pitchCorrectionFactors));
}
```

### python library
- pybind11 instead of cython and ctypes
- ctypes would require a c interface

- optimization flags:
- -mavx2 -mfma instead of -march=native for better compatibility with different cpus (used in numpy aswell (TODO add reference))


### benchmarking TODO
 - https://github.com/viktorleis/perfevent