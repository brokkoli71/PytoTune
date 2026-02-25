#ifndef PYTOTUNE_PITCH_DETECTOR_H
#define PYTOTUNE_PITCH_DETECTOR_H
#include "../io/wav_file.h"
#include "pytotune/data-structures/windowing.h"

namespace p2t {
    class YINPitchDetector {
    public:
        /**
         * Detects the pitch of the given audio buffer using the YIN algorithm with preset parameters.
         * @param audio_buffer The audio buffer containing the WavData.
         * @param f_min Minimum frequency to consider (in Hz).
         * @param f_max Maximum frequency to consider (in Hz).
         * @param threshold Threshold for pitch detection confidence (between 0 and 1).
         * @return A PitchDetection struct containing the detected pitch information.
         */
        [[nodiscard]] WindowedData<float> detect_pitch(const WavData &audio_buffer, int f_min, int f_max,
                                                       float threshold) const;

        /**
        * Constructor of the PitchDetector class.
        * @param windowing The windowing of all future pitch detection runs
        */
        explicit YINPitchDetector(Windowing windowing);

    private:
        Windowing windowing;

        // Helper function only used internally
        static inline float sinc(float x);

        static std::vector<float> design_lowpass_fir(int taps, float cutoff);

        static std::vector<float> decimate_zero_phase(const std::vector<float> &input, int factor);

        static std::vector<float> convolve(const std::vector<float> &signal, const std::vector<float> &kernel);
    };
}


#endif //PYTOTUNE_PITCH_DETECTOR_H
