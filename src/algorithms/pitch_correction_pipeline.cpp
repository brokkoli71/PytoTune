#include "pytotune/algorithms/pitch_correction_pipeline.h"

#include <algorithm>
#include <iostream>

#include "pytotune/algorithms/pitch_shifter.h"
#include "pytotune/algorithms/yin_pitch_detector.h"
#include <omp.h>

namespace p2t {
    WavFile PitchCorrectionPipeline::matchMidi(const WavFile &src,
                                               const MidiFile &midiFile,
                                               Windowing windowing,
                                               const float tuning,
                                               const PitchRange pitch_range) {
        const float sampleRate = static_cast<float>(src.data().sampleRate);


        YINPitchDetector ypd(windowing);
        double start = omp_get_wtime();
        WindowedData<float> pitches = ypd.detect_pitch(src.data(), pitch_range, 0.05f);

        double middle = omp_get_wtime();

        WindowedData<float> targetPitches = midiFile.getWindowedHighestPitches(windowing, sampleRate, tuning);

        std::vector<float> pitchCorrectionFactors(pitches.data.size());
        for (int i = 0; i < pitches.data.size(); ++i) {
            pitchCorrectionFactors[i] = (pitches.data[i] == 0 || targetPitches.data[i] == 0)
                                            ? 1
                                            : targetPitches.data[i] / pitches.data[i];
        }


        PitchShifter ps(windowing, sampleRate);
        std::vector<float> outValues = ps.run(src.data().samples, {windowing, pitchCorrectionFactors});
        double end = omp_get_wtime();

        std::cout << "Time for Pitch Detection: " << middle - start << ". " << (middle - start) / (end - start) * 100 <<
                "%" << std::endl;
        std::cout << "Time for Pitch Shifting: " << end - middle << ". " << (end - middle) / (end - start) * 100 << "%"
                << std::endl;

        return WavFile(WavData(static_cast<unsigned int>(sampleRate), 1, outValues));
    }

    WavFile PitchCorrectionPipeline::roundToScale(const WavFile &src,
                                                  const Scale &scale,
                                                  Windowing windowing, PitchRange pitch_range) {
        const float sampleRate = static_cast<float>(src.data().sampleRate);


        std::cout << "Run Yin Pitch Detector" << std::endl;
        YINPitchDetector ypd(windowing);
        WindowedData<float> pitches = ypd.detect_pitch(src.data(), pitch_range, 0.05f);

        std::cout << "Seek the target notes in the scale" << std::endl;
        std::vector<float> pitchCorrectionFactors(pitches.data.size());
        for (int i = 0; i < pitches.data.size(); ++i) {
            pitchCorrectionFactors[i] = scale.getClosestPitchInScale(pitches.data[i]) / pitches.data[i];
        }

        std::cout << "Run pitch correcting" << std::endl;
        PitchShifter ps(windowing, sampleRate);
        std::vector<float> outValues = ps.run(src.data().samples, {windowing, pitchCorrectionFactors});
        float maxP = *std::ranges::max_element(outValues);
        for (auto &p: outValues) {
            p /= maxP;
        }

        return WavFile(WavData(static_cast<unsigned int>(sampleRate), 1, outValues));
    }
}
