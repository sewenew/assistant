/**************************************************************************
   Copyright (c) 2023 sewenew

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *************************************************************************/

#ifndef SEWENEW_ASSISTANT_WAV_H
#define SEWENEW_ASSISTANT_WAV_H

#include <fstream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include "sw/assistant/errors.h"

namespace sw::assistant {

struct WavOptions {
    uint16_t channels = 2;
    uint32_t sample_per_second = 44100;
    SDL_AudioFormat format = AUDIO_S16;
};

struct WavHeader {
    uint8_t chunk_id[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunk_size = 0;
    uint8_t format[4] = {'W', 'A', 'V', 'E'};
    uint8_t fmt_chunk_id[4] = {'f', 'm', 't', ' '};
    uint32_t fmt_chunk_size = 16;
    uint16_t audio_format = 1;
    uint16_t num_channels = 0;
    uint32_t sample_rate = 0;
    uint32_t bytes_rate = 0;
    uint16_t block_align = 0;
    uint16_t bits_per_sample = 0;
    uint8_t data_chunk_id[4] = {'d', 'a', 't', 'a'};
    uint32_t data_chunk_size = 0;
};

class WavWriter {
public:
    WavWriter() = default;
    
    void write(const std::string &path, const WavOptions &options, const std::vector<uint8_t> &data) {
        _header.num_channels = options.channels;
        _header.sample_rate = options.sample_per_second;
        _header.bytes_rate = options.sample_per_second * options.channels * ((SDL_AUDIO_MASK_BITSIZE & options.format) / 8);
        if (options.channels == 1) {
            _header.block_align = 2;
        } else if (options.channels == 2) {
            _header.block_align = 4;
        } else {
            throw Error("unsupported channel number");
        }
        _header.bits_per_sample = SDL_AUDIO_MASK_BITSIZE & options.format;

        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char *>(&_header), sizeof(_header));
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
    }

private:
    WavHeader _header;
};

}

#endif // end SEWENEW_ASSISTANT_WAV_H
