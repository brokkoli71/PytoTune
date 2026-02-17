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
        const float sampleRate = static_cast<float>(src.data().sampleRate);

        // Silence detectotr TODO

        // TODO Fix the yin pitcher to use Windowing
        YINPitchDetector ypd(windowing);
        WindowedData<float> pitches = ypd.detect_pitch(src.data(), 20, 2000, 0.05f);

        WindowedData<float> targetPitches = midiFile.getWindowedHighestPitches(windowing, sampleRate, tuning);

        std::vector<float> pitchCorrectionFactors(pitches.data.size());
        for (int i = 0; i < pitches.data.size(); ++i) {
            pitchCorrectionFactors[i] = targetPitches.data[i] / pitches.data[i];
        }

        PitchShifter ps(windowing, sampleRate);
        std::vector<float> outValues = ps.run(src.data().samples, {windowing, pitchCorrectionFactors});

        return WavFile(WavData(static_cast<unsigned int>(sampleRate), 1, outValues));
    }

    WavFile PitchCorrectionPipeline::roundToScale(const WavFile &src,
                                                  const Scale &scale,
                                                  Windowing windowing) {
        const float sampleRate = static_cast<float>(src.data().sampleRate);

        // TODO Silence Detector

        // TODO Fix the yin pitcher to use Windowing
        std::cout << "Run Yin Pitch Detector" << std::endl;
        YINPitchDetector ypd(windowing);
        WindowedData<float> pitches = ypd.detect_pitch(src.data(), 20, 2000, 0.05f);

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

    std::vector<float> PitchCorrectionPipeline::processArrayToScale(const std::vector<float> &samples,
                                                                      unsigned int sample_rate,
                                                                      const Scale &scale,
                                                                      Windowing windowing) {
        const float sampleRate = static_cast<float>(sample_rate);
        
        // Create WavData - make a copy since we need to keep the original samples
        WavData wavData;
        wavData.sampleRate = sample_rate;
        wavData.numChannels = 1;
        wavData.samples = samples;  // Copy the samples
        
        YINPitchDetector ypd(windowing);
        WindowedData<float> pitches = ypd.detect_pitch(wavData, 20, 2000, 0.05f);

        std::vector<float> pitchCorrectionFactors(pitches.data.size());
        for (int i = 0; i < pitches.data.size(); ++i) {
            pitchCorrectionFactors[i] = scale.getClosestPitchInScale(pitches.data[i]) / pitches.data[i];
        }

        PitchShifter ps(windowing, sampleRate);
        std::vector<float> outValues = ps.run(samples, {windowing, pitchCorrectionFactors});
        
        return outValues;
    }

    std::vector<float> PitchCorrectionPipeline::processArrayToNote(const std::vector<float> &samples,
                                                                     unsigned int sample_rate,
                                                                     float target_note,
                                                                     Windowing windowing) {
        const float sampleRate = static_cast<float>(sample_rate);
        
        // Create WavData - make a copy since we need to keep the original samples
        WavData wavData;
        wavData.sampleRate = sample_rate;
        wavData.numChannels = 1;
        wavData.samples = samples;  // Copy the samples
        
        YINPitchDetector ypd(windowing);
        WindowedData<float> pitches = ypd.detect_pitch(wavData, 20, 2000, 0.05f);

        std::vector<float> pitchCorrectionFactors(pitches.data.size());
        for (int i = 0; i < pitches.data.size(); ++i) {
            pitchCorrectionFactors[i] = target_note / pitches.data[i];
        }

        PitchShifter ps(windowing, sampleRate);
        std::vector<float> outValues = ps.run(samples, {windowing, pitchCorrectionFactors});
        
        // Don't normalize for live mode - keep original amplitude
        return outValues;
    }
}
