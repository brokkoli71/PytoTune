#include <iostream>
#include <string>
#include <vector>

#include "pytotune/api.h"
#include "pytotune/algorithms/yin_pitch_detector.h"
#include "pytotune/data-structures/scale.h"

void printUsage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  pytotune-cli midi <wav_input> <midi_input> <wav_output> (<singer> | <fmin> <fmax>)?" << std::endl;
    std::cout << "  pytotune-cli scale <wav_input> <scale_name> <wav_output> (<singer> | <fmin> <fmax>)?" << std::endl;
    std::cout << std::endl;
    std::cout << " Optionally one can specify a frequency range for the pitch detection, by the following 2 options:" <<
            std::endl;
    std::cout <<
            " - <singer>: Will select a default range. Options are 'hearable' (all hearable frequencies), 'piano' (musically sensible), 'human', 'man', 'woman', 'bass', 'tenor', 'bariton', 'alto', 'soprano', 'cat'. Default is 'human'"
            << std::endl;
    std::cout << " - <fmin> <fmax>: Minimum and maximum detectable frequency" << std::endl;
}

p2t::PitchRange singerToPitchRange(const std::string &singer) {
    if (singer == "human") {
        return p2t::VoiceRanges::HUMAN;
    } else if (singer == "piano") {
        return p2t::VoiceRanges::PIANO;
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

    std::cerr << "Invalid singer: " << singer << std::endl;
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "midi") {
            if (argc < 5 || argc > 7) {
                std::cerr << "Error: Invalid number of arguments for midi mode." << std::endl;
                printUsage();
                return 1;
            }
            std::string wavPath = argv[2];
            std::string midiPath = argv[3];
            std::string outPath = argv[4];

            if (argc == 5) {
                p2t::matchMidi(wavPath, midiPath, outPath, p2t::VoiceRanges::HUMAN);
            } else if (argc == 6) {
                p2t::matchMidi(wavPath, midiPath, outPath, singerToPitchRange(argv[5]));
            } else {
                const p2t::PitchRange pr = {
                    std::stof(argv[5]), std::stof(argv[6])
                };
                p2t::matchMidi(wavPath, midiPath, outPath, pr);
            }
        } else if (mode == "scale") {
            if (argc < 5 || argc > 7) {
                std::cerr << "Error: Invalid number of arguments for scale mode." << std::endl;
                printUsage();
                return 1;
            }
            std::string wavPath = argv[2];
            std::string scaleName = argv[3];
            std::string outPath = argv[4];
            p2t::Scale scale = p2t::Scale::fromName(scaleName);

            if (argc == 5) {
                p2t::roundToScale(wavPath, scale, outPath, p2t::VoiceRanges::HUMAN);
            } else if (argc == 6) {
                p2t::roundToScale(wavPath, scale, outPath, singerToPitchRange(argv[5]));
            } else {
                const p2t::PitchRange pr = {
                    std::stof(argv[5]), std::stof(argv[6])
                };
                p2t::roundToScale(wavPath, scale, outPath, pr);
            }
        } else {
            std::cerr << "Error: Unknown mode '" << mode << "'." << std::endl;
            printUsage();
            return 1;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
