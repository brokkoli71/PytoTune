//
// Created by Moritz Seppelt on 12.11.25.
//

#ifndef PYTOTUNE_MODETEMPLATES_H
#define PYTOTUNE_MODETEMPLATES_H
#include <vector>
#include <string>
#include <unordered_map>

namespace p2t {

    // Represents a musical mode template (abstract scale)
    struct Mode {
        float repeatFactor;
        std::vector<float> notes;
    };

    // Global map of mode names to templates
    extern const std::unordered_map<std::string, Mode> modes;

    struct EqualTemperamentUtils {
        static float semitoneRatio(int semitone, float division = 12.0f, float repeat = 2.0f);

        static std::vector<float> semitoneRatios(const std::vector<int>& semitones, float division = 12.0f, float repeat = 2.0f);
    };

} //p2t

#endif //PYTOTUNE_MODETEMPLATES_H