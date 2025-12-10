#ifndef PYTOTUNE_PITCH_DETECTOR_H
#define PYTOTUNE_PITCH_DETECTOR_H
#include "../io/wav_file.h"
namespace p2t
{
    struct PitchDetection {
        int window_size;
        int window_overlap;
        const WavData* audio_buffer;
        std::vector<float> pitch_values;
    };

    class YINPitchDetector
    {
    public:

        /**
         * Detects the pitch of the given audio buffer using the YIN algorithm with preset parameters.
         * @param audio_buffer The audio buffer containing the WavData.
         * @param f_min Minimum frequency to consider (in Hz).
         * @param f_max Maximum frequency to consider (in Hz).
         * @param threshold Threshold for pitch detection confidence (between 0 and 1).
         * @return A PitchDetection struct containing the detected pitch information.
         */
        PitchDetection detect_pitch(const WavData* audio_buffer, int f_min, int f_max, float threshold) const;

        /**
        * Constructor of the PitchDetector class.
        * @param window_size Size of the analysis window in samples.
        */
        YINPitchDetector(int window_size, int window_overlap);

    private:
        int window_size;
        int window_overlap;
    };
}


#endif //PYTOTUNE_PITCH_DETECTOR_H