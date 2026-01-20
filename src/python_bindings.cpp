#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include <algorithm>
#include <vector>

#include "pytotune/io/wav_file.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/algorithms/yin_pitch_detector.h"
#include "pytotune/algorithms/pitch_shifter.h"
#include "pytotune/data-structures/scale.h"
#include "pytotune/data-structures/windowing.h"

namespace py = pybind11;

void tune_to_midi(const std::string& wav_path, const std::string& midi_path, const std::string& out_path) {
    auto wav = p2t::WavFile::load(wav_path);
    auto midi = p2t::MidiFile::load(midi_path);

    int windowSize = 4096;
    int stride = 1024;
    p2t::Windowing windowing(windowSize, stride, static_cast<float>(wav.data().sampleRate));

    p2t::YINPitchDetector detector(windowSize, windowSize - stride);
    auto pitchResult = detector.detect_pitch(&wav.data(), 50, 3000, 0.15f);

    auto targetPitches = midi.getWindowedHighestPitches(windowing, 0.0f);

    // 3. Calculate Factors
    std::vector<float> factors;
    size_t len = std::min(pitchResult.pitch_values.size(), targetPitches.data.size());
    factors.reserve(len);

    for (size_t i = 0; i < len; ++i) {
        float current = pitchResult.pitch_values[i];
        float target = targetPitches.data[i];

        // Shift only if we have a valid current pitch and a valid target pitch
        if (current > 1.0f && target > 1.0f) {
            factors.push_back(target / current);
        } else {
            factors.push_back(1.0f);
        }
    }

    // Handle length mismatch if any (pad 1.0)
    if (len < pitchResult.pitch_values.size()) {
       factors.insert(factors.end(), pitchResult.pitch_values.size() - len, 1.0f);
    }

    // 4. Shift
    p2t::PitchShifter shifter(windowing);
    p2t::WindowedData<float> factorData(windowing, factors);
    auto outSamples = shifter.run(wav.data().samples, factorData);

    // 5. Save
    p2t::WavFile outWav({wav.data().sampleRate, wav.data().numChannels, outSamples});
    outWav.store(out_path);
}

void tune_to_scale(const std::string& wav_path, const std::string& scale_name, float tuning, const std::string& out_path) {
    auto wav = p2t::WavFile::load(wav_path);
    auto scale = p2t::Scale::fromName(scale_name, tuning);

    int windowSize = 4096;
    int stride = 1024;
    p2t::Windowing windowing(windowSize, stride, static_cast<float>(wav.data().sampleRate));

    p2t::YINPitchDetector detector(windowSize, windowSize - stride);
    auto pitchResult = detector.detect_pitch(&wav.data(), 50, 3000, 0.15f);

    std::vector<float> factors;
    factors.reserve(pitchResult.pitch_values.size());

    for (float current : pitchResult.pitch_values) {
        if (current > 1.0f) {
            float target = scale.getClosestPitchInScale(current);
            factors.push_back(target / current);
        } else {
            factors.push_back(1.0f);
        }
    }

    p2t::PitchShifter shifter(windowing);
    p2t::WindowedData<float> factorData(windowing, factors);
    auto outSamples = shifter.run(wav.data().samples, factorData);

    p2t::WavFile outWav({wav.data().sampleRate, wav.data().numChannels, outSamples});
    outWav.store(out_path);
}

PYBIND11_MODULE(pytotune, m) {
    m.doc() = "PytoTune: Cloud Rap Auto-Tune Library";

    m.def("tune_to_midi", &tune_to_midi, "Tune a WAV file using a MIDI file as reference",
          py::arg("wav_path"), py::arg("midi_path"), py::arg("out_path"));

    m.def("tune_to_scale", &tune_to_scale, "Tune a WAV file to a musical scale",
          py::arg("wav_path"), py::arg("scale_name"), py::arg("tuning"), py::arg("out_path"));
}
