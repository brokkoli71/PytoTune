#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>

#include "pytotune/io/wav_file.h"
#include "../src/api.cpp"

namespace py = pybind11;

PYBIND11_MODULE(pytotune, m) {
    m.doc() = "PytoTune: Cloud Rap Auto-Tune Library";

    m.def("tune_to_midi", &p2t::tune_to_midi, "Tune a WAV file using a MIDI file as reference",
          py::arg("wav_path"), py::arg("midi_path"), py::arg("out_path"));

    m.def("tune_to_scale", &p2t::tune_to_scale, "Tune a WAV file to a musical scale",
          py::arg("wav_path"), py::arg("scale_name"), py::arg("tuning"), py::arg("out_path"));
}
