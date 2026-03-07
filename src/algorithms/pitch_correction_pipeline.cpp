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

    std::vector<float> PitchCorrectionPipeline::valuesPitchedToMidi(const WavFile &src, const MidiFile &midiFile, const WindowedData<float> &pitches, Windowing windowing, const float tuning) {
        const auto sampleRate = static_cast<float>(src.data().sampleRate);

        WindowedData<float> targetPitches = midiFile.getWindowedHighestPitches(windowing, sampleRate, tuning);

        std::vector<float> pitchCorrectionFactors(pitches.data.size());
        for (int i = 0; i < pitches.data.size(); ++i) {
            pitchCorrectionFactors[i] = (pitches.data[i] == 0 || targetPitches.data[i] == 0)
                                            ? 1.f
                                            : targetPitches.data[i] / pitches.data[i];
        }

        PitchShifter ps(windowing, sampleRate);
        std::vector<float> outValues = ps.run(src.data().samples, {windowing, pitchCorrectionFactors});
        peakNormalise(outValues);
        return outValues;
    }

    WavFile PitchCorrectionPipeline::matchMidi(const WavFile &src,
                                               const MidiFile &midiFile,
                                               Windowing windowing,
                                               const float tuning,
                                               const PitchRange pitchRange) {
        const auto sampleRate = static_cast<float>(src.data().sampleRate);

        YINPitchDetector ypd(windowing);
        WindowedData<float> pitches = ypd.detectPitch(src.data(), pitchRange);

        auto outValues = valuesPitchedToMidi(src, midiFile, pitches, windowing, tuning);
        return WavFile(WavData(static_cast<unsigned int>(sampleRate), 1, outValues));
    }

    WavFile PitchCorrectionPipeline::roundToScale(const WavFile &src,
                                                  const Scale &scale,
                                                  Windowing windowing, PitchRange pitchRange) {
        const auto sampleRate = static_cast<float>(src.data().sampleRate);

        YINPitchDetector ypd(windowing);
        WindowedData<float> pitches = ypd.detectPitch(src.data(), pitchRange, 0.05f);

        std::vector<float> pitchCorrectionFactors(pitches.data.size());
        for (int i = 0; i < pitches.data.size(); ++i) {
            pitchCorrectionFactors[i] = scale.getPitchCorrectionFactor(pitches.data[i]);
        }

        PitchShifter ps(windowing, sampleRate);
        std::vector<float> outValues = ps.run(src.data().samples, {windowing, pitchCorrectionFactors});
        peakNormalise(outValues);
        return WavFile(WavData(static_cast<unsigned int>(sampleRate), 1, outValues));
    }

} // namespace p2t
