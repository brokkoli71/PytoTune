#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>

#include "pytotune/io/wav_file.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/data-structures/scale.h"
#include "pytotune/data-structures/windowing.h"
#include "pytotune/algorithms/pitch_correction_pipeline.h"

namespace py = pybind11;

void tune_to_midi(const std::string& wav_path, const std::string& midi_path, const std::string& out_path) {
    auto wav = p2t::WavFile::load(wav_path);
    auto midi = p2t::MidiFile::load(midi_path);

    int windowSize = 4096;
    int stride = 1024;
    p2t::Windowing windowing(windowSize, stride);

    p2t::PitchCorrectionPipeline pipeline;
    p2t::WavFile outWav = pipeline.matchMidi(wav, midi, windowing, 0.0f);
    outWav.store(out_path);
}

void tune_to_scale(const std::string& wav_path, const std::string& scale_name, float tuning, const std::string& out_path) {
    auto wav = p2t::WavFile::load(wav_path);
    auto scale = p2t::Scale::fromName(scale_name, tuning);

    int windowSize = 4096;
    int stride = 1024;
    p2t::Windowing windowing(windowSize, stride);

    p2t::PitchCorrectionPipeline pipeline;
    p2t::WavFile outWav = pipeline.roundToScale(wav, scale, windowing);
    outWav.store(out_path);
}

PYBIND11_MODULE(pytotune, m) {
    m.doc() = "PytoTune: Cloud Rap Auto-Tune Library";

    m.def("tune_to_midi", &tune_to_midi, "Tune a WAV file using a MIDI file as reference",
          py::arg("wav_path"), py::arg("midi_path"), py::arg("out_path"));

    m.def("tune_to_scale", &tune_to_scale, "Tune a WAV file to a musical scale",
          py::arg("wav_path"), py::arg("scale_name"), py::arg("tuning"), py::arg("out_path"));
}
