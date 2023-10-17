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

#include "sw/assistant/audio_player.h"
#include "sw/assistant/audio_utils.h"
#include "sw/assistant/errors.h"
#include <cassert>
#include <SDL2/SDL.h>

namespace sw::assistant {

AudioPlayer::AudioPlayer(const AudioPlayerOptions &options) {
    auto desired_spec = _to_spec(options);

    const char *device_name = nullptr;
    if (!options.device_name.empty()) {
        device_name = options.device_name.data();
    }

    _device_id = SDL_OpenAudioDevice(device_name, SDL_FALSE,
            &desired_spec, &_audio_spec, options.allowed_changes);
    if (_device_id == 0) {
        throw SDLError("failed to open recording device");
    }
}

void AudioPlayer::play(const std::vector<uint8_t> &wav) {
    auto duration = _calc_duration(wav.size());

    SDL_PauseAudioDevice(_device_id, SDL_FALSE);
    if (SDL_QueueAudio(_device_id, wav.data(), wav.size()) != 0) {
        throw SDLError("failed to play");
    }
    SDL_Delay(duration);

    SDL_PauseAudioDevice(_device_id, SDL_TRUE);
}

SDL_AudioSpec AudioPlayer::_to_spec(const AudioPlayerOptions &options) const {
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

uint32_t AudioPlayer::_calc_duration(uint32_t size) const {
    auto bytes_per_sample = SDL_AUDIO_BITSIZE(_audio_spec.format) / 8;
    auto bytes_per_second = bytes_per_sample * _audio_spec.channels * _audio_spec.freq;
    return static_cast<uint32_t>(size / bytes_per_second) * 1000;
}

}
