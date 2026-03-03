#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include <string>

#include "pytotune/api.h"
#include "pytotune/data-structures/scale.h"

namespace py = pybind11;

static p2t::PitchRange singerToPitchRange(const std::string &singer) {
    if (singer == "human") {
        return p2t::VoiceRanges::HUMAN;
    } else if (singer == "man") {
        return p2t::VoiceRanges::MAN;
    } else if (singer == "woman") {
        return p2t::VoiceRanges::WOMAN;
    } else if (singer == "bass") {
        return p2t::VoiceRanges::BASS;
    } else if (singer == "tenor") {
        return p2t::VoiceRanges::TENOR;
    } else if (singer == "bariton") {
        return p2t::VoiceRanges::BARITON;
    } else if (singer == "alto") {
        return p2t::VoiceRanges::ALTO;
    } else if (singer == "soprano") {
        return p2t::VoiceRanges::SOPRANO;
    } else if (singer == "hearable") {
        return p2t::VoiceRanges::HEARABLE;
    } else if (singer == "cat") {
        return p2t::VoiceRanges::CAT_PURR;
    }

    throw std::invalid_argument(std::string("Invalid singer: ") + singer);
}

PYBIND11_MODULE(pytotune, m) {
    m.doc() = "PytoTune: Cloud Rap Auto-Tune Library";

    // Bind PitchRange so Python can construct ranges and pass them to tune functions.
    py::class_<p2t::PitchRange>(m, "PitchRange")
        .def(py::init<float, float>(), py::arg("min"), py::arg("max"))
        .def_readwrite("min", &p2t::PitchRange::min)
        .def_readwrite("max", &p2t::PitchRange::max)
        .def("__repr__", [](const p2t::PitchRange &p) {
            return "<PitchRange min=" + std::to_string(p.min) + " max=" + std::to_string(p.max) + ">";
        });

    // Expose helper to map singer names to PitchRange
    m.def("singer_to_pitch_range", &singerToPitchRange, "Convert singer name to a PitchRange", py::arg("singer"));

    // Expose common voice ranges as module attributes for convenience
    m.attr("VoiceRange_HUMAN") = py::cast(p2t::VoiceRanges::HUMAN);
    m.attr("VoiceRange_MAN") = py::cast(p2t::VoiceRanges::MAN);
    m.attr("VoiceRange_WOMAN") = py::cast(p2t::VoiceRanges::WOMAN);
    m.attr("VoiceRange_BASS") = py::cast(p2t::VoiceRanges::BASS);
    m.attr("VoiceRange_TENOR") = py::cast(p2t::VoiceRanges::TENOR);
    m.attr("VoiceRange_BARITON") = py::cast(p2t::VoiceRanges::BARITON);
    m.attr("VoiceRange_ALTO") = py::cast(p2t::VoiceRanges::ALTO);
    m.attr("VoiceRange_SOPRANO") = py::cast(p2t::VoiceRanges::SOPRANO);
    m.attr("VoiceRange_HEARABLE") = py::cast(p2t::VoiceRanges::HEARABLE);
    m.attr("VoiceRange_CAT_PURR") = py::cast(p2t::VoiceRanges::CAT_PURR);

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

    // Make pitch_range optional by defaulting to HUMAN (PitchRange is registered above)
    m.def("tune_to_midi", &p2t::tune_to_midi, "Tune a WAV file using a MIDI file as reference",
          py::arg("wav_path"), py::arg("midi_path"), py::arg("out_path"),
          py::arg("pitch_range") = p2t::VoiceRanges::HUMAN);

    m.def("tune_to_scale", &p2t::tune_to_scale, "Tune a WAV file to a musical scale",
          py::arg("wav_path"), py::arg("scale"), py::arg("out_path"),
          py::arg("pitch_range") = p2t::VoiceRanges::HUMAN);
}
