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

#include <SDL2/SDL.h>
#include "sw/assistant/audio_utils.h"
#include "sw/assistant/errors.h"

namespace {

sw::va::SDLInit sdl_init;

}

namespace sw::assistant {

SDLError::SDLError(const std::string &msg) : Error(msg + ": " + SDL_GetError()) {}

SDLInit::SDLInit() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        throw SDLError("failed to init SDL");
    }
}

SDLInit::~SDLInit() {
    SDL_Quit();
}
    
namespace audio_utils {

std::vector<std::string> list_devices(AudioType type) {
    int is_capture = SDL_TRUE;
    switch (type) {
    case AudioType::RECORDING:
        is_capture = SDL_TRUE;
        break;

    case AudioType::PLAYBACK:
        is_capture = SDL_FALSE;
        break;

    default:
        throw Error("invalid audio device type");
    }

    auto device_cnt = SDL_GetNumAudioDevices(is_capture);
    if (device_cnt < 1) {
        throw SDLError("no audio device found");
    }

    std::vector<std::string> devs;
    devs.reserve(device_cnt);
    for (auto idx = 0; idx < device_cnt; ++idx) {
        auto *name = SDL_GetAudioDeviceName(idx, is_capture);
        devs.push_back(name);
    }

    return devs;
}

}

}
