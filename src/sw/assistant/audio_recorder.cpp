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

#include "sw/assistant/audio_recorder.h"
#include "sw/assistant/audio_utils.h"
#include "sw/assistant/errors.h"
#include <cassert>
#include <SDL2/SDL.h>
#include <iostream>
#include <thread>

namespace sw::assistant {

AudioRecorder::AudioRecorder(const AudioRecorderOptions &options) {
    auto desired_spec = _to_spec(options);

    const char *device_name = nullptr;
    if (!options.device_name.empty()) {
        device_name = options.device_name.data();
    }

    _device_id = SDL_OpenAudioDevice(device_name, SDL_TRUE,
            &desired_spec, &_audio_spec, options.allowed_changes);
    if (_device_id == 0) {
        throw SDLError("failed to open recording device");
    }
}

std::vector<uint8_t> AudioRecorder::record(const std::chrono::seconds &duration) {
    auto precision = std::chrono::milliseconds(10);
    if (duration < precision) {
        // TODO: throw error.
        return {};
    }

    auto buffer_size = _calc_buffer_size(duration);
    std::vector<uint8_t> buffer;
    buffer.resize(buffer_size);

    auto end = std::chrono::steady_clock::now() + duration;

    SDL_PauseAudioDevice(_device_id, SDL_FALSE);

    auto idx = 0U;
    while (std::chrono::steady_clock::now() < end && buffer.size() > idx) {
        auto len = SDL_DequeueAudio(_device_id, buffer.data() + idx, buffer.size() - idx);
        /*
        if (len < buffer.size() - idx) {
            std::cerr << SDL_GetError() << std::endl;
        }
        */
        idx += len;
        std::this_thread::sleep_for(precision);
    }

    SDL_PauseAudioDevice(_device_id, SDL_TRUE);

    assert(idx <= buffer.size());

    buffer.resize(idx);

    return buffer;
}

/*
void AudioRecorder::_callback(void *user_data, uint8_t *stream, int len) {
}
*/

SDL_AudioSpec AudioRecorder::_to_spec(const AudioRecorderOptions &options) const {
    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);

    desired_spec.freq = options.freq;
    desired_spec.format = options.format;
    desired_spec.channels = options.channels;
    desired_spec.samples = options.samples;
    desired_spec.callback = nullptr;//_callback;
    //desired_spec.userdata = this;

    return desired_spec;
}

uint32_t AudioRecorder::_calc_buffer_size(const std::chrono::seconds &duration) const {
    auto bytes_per_sample = (SDL_AUDIO_MASK_BITSIZE & _audio_spec.format) / 8;

    auto bytes_per_second = bytes_per_sample * _audio_spec.channels * _audio_spec.freq;

    // Add 1 second buffer.
    return bytes_per_second * (duration.count() + 1);
}

}
