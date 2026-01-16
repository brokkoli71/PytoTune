//
// Created by Moritz Seppelt on 12.12.25.
//

#include "pytotune/algorithms/pitch_correction_pipeline.h"

#include <algorithm>
#include <iostream>

#include "pytotune/algorithms/pitch_shifter.h"
#include "pytotune/algorithms/yin_pitch_detector.h"

namespace p2t {
    WavFile PitchCorrectionPipeline::matchMidi(const WavFile &src,
                                               const MidiFile &midiFile,
                                               Windowing windowing,
                                               const float tuning) {
        windowing.sampleRate = static_cast<float>(src.data().sampleRate);

        // Silence detectotr TODO

        // TODO Fix the yin pitcher to use Windowing
        YINPitchDetector ypd(windowing.windowSize, windowing.windowSize - windowing.stride);
        PitchDetection pitches = ypd.detect_pitch(&src.data(), 20, 20000, 0.05f);

        WindowedData<float> targetPitches = midiFile.getWindowedHighestPitches(windowing, 0, tuning);

        std::vector<float> pitchCorrectionFactors(pitches.pitch_values.size());
        for (int i = 0; i < pitches.pitch_values.size(); ++i) {
            pitchCorrectionFactors[i] = targetPitches.data[i] / pitches.pitch_values[i];
        }

        PitchShifter ps(windowing);
        std::vector<float> outValues = ps.run(src.data().samples, {windowing, pitchCorrectionFactors});

        return WavFile(WavData(static_cast<unsigned int>(windowing.sampleRate), 1, outValues));
    }

    WavFile PitchCorrectionPipeline::roundToScale(const WavFile &src,
                                                  const Scale &scale,
                                                  Windowing windowing,
                                                  const float tuning) {
        windowing.sampleRate = static_cast<float>(src.data().sampleRate);

        // TODO Silence Detector

        // TODO Fix the yin pitcher to use Windowing
        std::cout << "Run Yin Pitch Detector" << std::endl;
        YINPitchDetector ypd(windowing.windowSize, windowing.windowSize - windowing.stride);
        PitchDetection pitches = ypd.detect_pitch(&src.data(), 20, 2000, 0.05f);

        std::cout << "Seek the target notes in the scale" << std::endl;
        std::vector<float> pitchCorrectionFactors(pitches.pitch_values.size());
        for (int i = 0; i < pitches.pitch_values.size(); ++i) {
            pitchCorrectionFactors[i] = scale.getClosestPitchInScale(pitches.pitch_values[i]) / pitches.pitch_values[i];
        }

        std::cout << "Run pitch correcting" << std::endl;
        PitchShifter ps(windowing);
        std::vector<float> outValues = ps.run(src.data().samples, {windowing, pitchCorrectionFactors});
        float maxP = *std::ranges::max_element(outValues);
        for (auto &p: outValues) {
            p /= maxP;
        }

        return WavFile(WavData(static_cast<unsigned int>(windowing.sampleRate), 1, outValues));
    }
}
