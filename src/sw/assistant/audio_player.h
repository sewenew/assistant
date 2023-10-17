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

#ifndef SEWENEW_ASSISTANT_AUDIO_PLAYER_H
#define SEWENEW_ASSISTANT_AUDIO_PLAYER_H

#include <chrono>
#include <string>
#include <vector>
#include <SDL2/SDL.h>

namespace sw::assistant {

struct AudioPlayerOptions {
    std::string device_name;
    int freq = 44100;
    SDL_AudioFormat format = AUDIO_S16;
    uint8_t channels = 2;
    uint16_t samples = 4096;
    int allowed_changes = 0;
};

class AudioPlayer {
public:
    explicit AudioPlayer(const AudioPlayerOptions &options = {});

    void play(const std::vector<uint8_t> &data);

private:
    SDL_AudioSpec _to_spec(const AudioPlayerOptions &options) const;

    uint32_t _calc_duration(uint32_t size) const;

    SDL_AudioSpec _audio_spec;

    int _device_id = 0;
};

}

#endif // end SEWENEW_ASSISTANT_AUDIO_PLAYER_H
