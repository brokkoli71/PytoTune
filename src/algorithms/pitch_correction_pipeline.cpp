#include "pytotune/algorithms/pitch_correction_pipeline.h"

#include <algorithm>

#include "pytotune/algorithms/pitch_shifter.h"
#include "pytotune/algorithms/yin_pitch_detector.h"

namespace p2t {

    static void peakNormalise(std::vector<float> &samples) {
        float maxAbs = 0.f;
        for (float s : samples)
            maxAbs = std::max(maxAbs, std::abs(s));

        if (maxAbs > 1.f) {
            const float normFactor = 1.f / maxAbs;
            for (float &s : samples)
                s *= normFactor;
        }
    }

    WindowedData<float> PitchCorrectionPipeline::detectPitch(const WavFile &src,
                                                              Windowing windowing,
                                                              PitchRange pitchRange,
                                                              float threshold,
                                                              int decimationFactor) const {
        YINPitchDetector ypd(windowing);
        return ypd.detectPitch(src.data(), pitchRange, threshold, decimationFactor);
    }

    WavFile PitchCorrectionPipeline::shiftPitch(const WavFile &src,
                                                Windowing windowing,
                                                const std::vector<float> &correctionFactors) const {
        const auto sampleRate = static_cast<float>(src.data().sampleRate);
        PitchShifter ps(windowing, sampleRate);
        auto outValues = ps.run(src.data().samples, {windowing, correctionFactors});
        peakNormalise(outValues);
        return WavFile(WavData(static_cast<unsigned int>(sampleRate), 1, outValues));
    }

    WavFile PitchCorrectionPipeline::matchMidi(const WavFile &src,
                                               const MidiFile &midiFile,
                                               Windowing windowing,
                                               const float tuning,
                                               const PitchRange pitchRange) const {
        const auto sampleRate = static_cast<float>(src.data().sampleRate);
        const auto pitches = detectPitch(src, windowing, pitchRange);
        const auto factors = midiFile.getPitchCorrectionFactors(pitches, windowing, sampleRate, tuning);
        return shiftPitch(src, windowing, factors);
    }

    WavFile PitchCorrectionPipeline::roundToScale(const WavFile &src,
                                                  const Scale &scale,
                                                  Windowing windowing,
                                                  PitchRange pitchRange) const {
        const auto pitches = detectPitch(src, windowing, pitchRange, 0.05f);
        const auto factors = scale.getPitchCorrectionFactors(pitches.data);
        return shiftPitch(src, windowing, factors);
    }

} // namespace p2t
