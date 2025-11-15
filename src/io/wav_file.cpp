#include "../../include/pytotune/io/wav_file.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>


namespace {
    template<typename T>
    T read(std::ifstream &file) {
        T value;
        file.read(reinterpret_cast<char *>(&value), sizeof(T));
        if (file.gcount() != sizeof(T) || file.fail()) {
            throw std::runtime_error("Failed to read required number of bytes from file");
        }
        return value;
    }
}

namespace p2t {
    WavFile WavFile::load(const std::string &path) {
        WavFile result;
        result.readFile(path);
        return result;
    }

    void WavFile::readFile(const std::string &path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open file: " + path);

        char riff[4];
        file.read(riff, 4);
        if (std::strncmp(riff, "RIFF", 4) != 0)
            throw std::runtime_error("Invalid WAV file (missing RIFF)");

        int fileSize;
        file.read(reinterpret_cast<char *>(&fileSize), 4);

        char wave[4];
        file.read(wave, 4);
        if (std::strncmp(wave, "WAVE", 4) != 0)
            throw std::runtime_error("Invalid WAV file (missing WAVE)");

        // --- Read chunks ---
        uint16_t audioFormat = 0, numChannels = 0, bitsPerSample = 0;
        uint32_t sampleRate = 0;

        std::vector<char> dataChunk;

        while (file && !file.eof()) {
            char chunkId[4];
            if (!file.read(chunkId, 4)) break;
            auto chunkSize = read<uint32_t>(file);

            if (std::strncmp(chunkId, "fmt ", 4) == 0) {
                audioFormat = read<uint16_t>(file);
                if (audioFormat != 1 && audioFormat != 3) // PCM or float
                    throw std::runtime_error("Unsupported WAV format. Only PCM and float are supported.");
                numChannels = read<uint16_t>(file);
                sampleRate = read<uint32_t>(file);
                // Ignore byte rate and block align as can be computed from other values.
                file.ignore(6);

                bitsPerSample = read<uint16_t>(file);
                file.ignore(chunkSize - 16);
            } else if (std::strncmp(chunkId, "data", 4) == 0) {
                dataChunk.resize(chunkSize);
                file.read(dataChunk.data(), chunkSize);
            } else {
                file.ignore(chunkSize); // skip unknown chunk
            }
        }

        if (dataChunk.empty())
            throw std::runtime_error("No data chunk found in WAV file");

        size_t numSamples = dataChunk.size() / (bitsPerSample / 8);
        std::vector<float> samples(numSamples);

        if (bitsPerSample == 16 && audioFormat == 1) {
            // PCM
            const int16_t *ptr = reinterpret_cast<const int16_t *>(dataChunk.data());
            for (size_t i = 0; i < numSamples; ++i)
                samples[i] = ptr[i] / 32768.0f;
        } else if (bitsPerSample == 32 && audioFormat == 3) {
            // float
            const float *ptr = reinterpret_cast<const float *>(dataChunk.data());
            samples.assign(ptr, ptr + numSamples);
        } else {
            throw std::runtime_error("Unsupported WAV format. Only 16-bit PCM and 32-bit float are supported.");
        }

        wavData_.sampleRate = sampleRate;
        wavData_.numChannels = numChannels;
        wavData_.samples = std::move(samples);
    }
}
