#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <iostream>

#include "pytotune/api.h"
#include "pytotune/data-structures/scale.h"

namespace py = pybind11;

PYBIND11_MODULE(pytotune, m) {
    m.doc() = "PytoTune: Cloud Rap Auto-Tune Library";

    m.def("tune_to_midi", &p2t::tune_to_midi, "Tune a WAV file using a MIDI file as reference",
          py::arg("wav_path"), py::arg("midi_path"), py::arg("out_path"));

    m.def("tune_to_scale", &p2t::tune_to_scale, "Tune a WAV file to a musical scale",
          py::arg("wav_path"), py::arg("scale"), py::arg("out_path"));

    m.def("tune_array_to_scale", 
          [](py::array_t<float> samples, unsigned int sample_rate, const p2t::Scale &scale) -> py::array_t<float> {
              py::buffer_info buf = samples.request();
              float* ptr = static_cast<float*>(buf.ptr);
              std::vector<float> samples_vec(ptr, ptr + buf.size);
              
              std::vector<float> result = p2t::tune_array_to_scale(samples_vec, sample_rate, scale);
              
              // Allocate new array and move data
              size_t size = result.size();
              float* data = new float[size];
              std::memcpy(data, result.data(), size * sizeof(float));
              
              // Create capsule for memory management
              py::capsule free_when_done(data, [](void *f) {
                  float *foo = reinterpret_cast<float *>(f);
                  delete[] foo;
              });
              
              return py::array_t<float>(
                  {size},  // shape
                  {sizeof(float)},  // strides
                  data,  // data pointer
                  free_when_done  // numpy will call this when done
              );
          },
          "Tune raw audio samples to a musical scale (live mode)",
          py::arg("samples"), py::arg("sample_rate"), py::arg("scale"));

    m.def("tune_array_to_note",
          [](py::array_t<float> samples, unsigned int sample_rate, float target_note) -> py::array_t<float> {
              py::buffer_info buf = samples.request();
              float* ptr = static_cast<float*>(buf.ptr);
              std::vector<float> samples_vec(ptr, ptr + buf.size);
              
              std::vector<float> result = p2t::tune_array_to_note(samples_vec, sample_rate, target_note);
              
              // Allocate new array and move data
              size_t size = result.size();
              float* data = new float[size];
              std::memcpy(data, result.data(), size * sizeof(float));
              
              // Create capsule for memory management
              py::capsule free_when_done(data, [](void *f) {
                  float *foo = reinterpret_cast<float *>(f);
                  delete[] foo;
              });
              
              return py::array_t<float>(
                  {size},  // shape
                  {sizeof(float)},  // strides
                  data,  // data pointer
                  free_when_done  // numpy will call this when done
              );
          },
          "Tune raw audio samples to a target note frequency (live mode)",
          py::arg("samples"), py::arg("sample_rate"), py::arg("target_note"));

    py::class_<p2t::Scale>(m, "Scale")
            .def(py::init<float, float, std::vector<float> >(),
                 py::arg("base_note"),
                 py::arg("repeat_factor"),
                 py::arg("notes"))

            .def_static("from_name",
                        &p2t::Scale::fromName,
                        py::arg("name"),
                        py::arg("tuning") = DEFAULT_A4)

            .def_static("from_mode_name",
                        &p2t::Scale::fromModeName,
                        py::arg("mode_name"),
                        py::arg("base_note"))

            .def_static("from_mode",
                        &p2t::Scale::fromMode,
                        py::arg("mode"),
                        py::arg("base_note"))

            .def("closest_pitch",
                 &p2t::Scale::getClosestPitchInScale,
                 py::arg("pitch"))

            .def_property(
                "base_note",
                &p2t::Scale::getBaseNote,
                &p2t::Scale::setBaseNote)

            .def_property(
                "repeat_factor",
                &p2t::Scale::getRepeatFactor,
                &p2t::Scale::setRepeatFactor)

            .def_property(
                "notes",
                [](const p2t::Scale &s) {
                    return s.getNotes();
                },
                &p2t::Scale::setNotes
            );
}
