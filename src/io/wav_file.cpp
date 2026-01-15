#include "pytotune/io/wav_file.h"
#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>
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
    WavFile::WavFile(WavData data)
        : wavData_(std::move(data)) {
    }

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

        // For now: if stereo or more channels, take only first channel
        if (numSamples > 1) {
            std::vector<float> mono_samples(numSamples/numChannels);
            for (size_t i = 0; i < mono_samples.size(); ++i) {
                mono_samples[i] = samples[i * numChannels];
            }
            samples = std::move(mono_samples);
            samples.shrink_to_fit();
            numChannels = 1;
        }

        wavData_.sampleRate = sampleRate;
        wavData_.numChannels = numChannels;
        wavData_.samples = std::move(samples);
    }


    void WavFile::store(const std::string &path, uint16_t audioFormat) const {
        if (audioFormat != 1 && audioFormat != 3) {
            throw std::runtime_error("Unsupported audio format for writing. Only PCM (1) and Float (3) are supported.");
        }


        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file for writing: " + path);
        }

        // Helper function to write data
        auto writeData = [&](const void *data, size_t size) {
            file.write(reinterpret_cast<const char *>(data), size);
            if (!file) {
                throw std::runtime_error("Error writing to file");
            }
        };

        // RIFF Header
        char riff[4] = {'R', 'I', 'F', 'F'};
        writeData(riff, 4);

        // Placeholder for file size (we will fix this later)
        uint32_t fileSize = 0;
        writeData(&fileSize, sizeof(fileSize));

        // WAVE Header
        char wave[4] = {'W', 'A', 'V', 'E'};
        writeData(wave, 4);

        // fmt Chunk
        char fmt[4] = {'f', 'm', 't', ' '};
        writeData(fmt, 4);

        // Size of fmt chunk (16 for PCM or 18 for float)
        uint32_t fmtChunkSize = 16;
        writeData(&fmtChunkSize, sizeof(fmtChunkSize));

        // Audio Format (1 for PCM, 3 for Float)
        writeData(&audioFormat, sizeof(audioFormat));

        // Number of Channels
        writeData(&wavData_.numChannels, sizeof(wavData_.numChannels));

        // Sample Rate
        writeData(&wavData_.sampleRate, sizeof(wavData_.sampleRate));

        // Byte Rate (Sample Rate * Num Channels * Bits Per Sample / 8)
        uint32_t byteRate = wavData_.sampleRate * wavData_.numChannels * (audioFormat == 1 ? 2 : 4);
        // 2 bytes for 16-bit, 4 bytes for float
        writeData(&byteRate, sizeof(byteRate));

        // Block Align (Num Channels * Bits Per Sample / 8)
        uint16_t blockAlign = wavData_.numChannels * (audioFormat == 1 ? 2 : 4);
        // 2 bytes for 16-bit, 4 bytes for float
        writeData(&blockAlign, sizeof(blockAlign));

        // Bits Per Sample (16 for PCM, 32 for float)
        uint16_t bitsPerSample = (audioFormat == 1) ? 16 : 32;
        writeData(&bitsPerSample, sizeof(bitsPerSample));

        // data Chunk
        char data[4] = {'d', 'a', 't', 'a'};
        writeData(data, 4);

        // Data Chunk Size (number of samples * bits per sample / 8 * number of channels)
        uint32_t dataChunkSize = wavData_.samples.size() * (audioFormat == 1 ? 2 : 4) * wavData_.numChannels;
        writeData(&dataChunkSize, sizeof(dataChunkSize));

        // Writing audio samples
        if (audioFormat == 1) {
            // PCM 16-bit
            std::vector<int16_t> pcmData(wavData_.samples.size());
            for (size_t i = 0; i < wavData_.samples.size(); ++i) {
                pcmData[i] = static_cast<int16_t>(wavData_.samples[i] * 32768.0f); // Scale to 16-bit PCM range
            }
            writeData(pcmData.data(), pcmData.size() * sizeof(int16_t));
        } else if (audioFormat == 3) {
            // Float 32-bit
            writeData(wavData_.samples.data(), wavData_.samples.size() * sizeof(float));
        }

        // Now we need to update the RIFF chunk size, which is the total file size - 8 bytes for the RIFF header
        uint32_t totalFileSize = static_cast<uint32_t>(file.tellp());
        uint32_t riffChunkSize = totalFileSize - 8; // exclude the "RIFF" and size fields
        file.seekp(4, std::ios::beg); // Go back to the file size position
        writeData(&riffChunkSize, sizeof(riffChunkSize));

        file.close();
    }
}
