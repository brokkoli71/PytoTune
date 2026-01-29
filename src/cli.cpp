#include <iostream>
#include <string>
#include <vector>

#include "pytotune/api.h"
#include "pytotune/data-structures/scale.h"

void print_usage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  pytotune-cli midi <wav_input> <midi_input> <wav_output>" << std::endl;
    std::cout << "  pytotune-cli scale <wav_input> <scale_name> <tuning> <wav_output>" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "midi") {
            if (argc != 5) {
                std::cerr << "Error: Invalid number of arguments for midi mode." << std::endl;
                print_usage();
                return 1;
            }
            std::string wav_input = argv[2];
            std::string midi_input = argv[3];
            std::string wav_output = argv[4];
            p2t::tune_to_midi(wav_input, midi_input, wav_output);
        } else if (mode == "scale") {
            if (argc != 6) {
                std::cerr << "Error: Invalid number of arguments for scale mode." << std::endl;
                print_usage();
                return 1;
            }
            std::string wav_input = argv[2];
            std::string scale_name = argv[3];
            float tuning = std::stof(argv[4]);
            p2t::Scale scale = p2t::Scale::fromName(scale_name, tuning);
            std::string wav_output = argv[5];
            p2t::tune_to_scale(wav_input, scale, wav_output);
        } else {
            std::cerr << "Error: Unknown mode '" << mode << "'." << std::endl;
            print_usage();
            return 1;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
