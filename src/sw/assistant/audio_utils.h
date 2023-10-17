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

#ifndef SEWENEW_ASSISTANT_AUDIO_UTILS_H
#define SEWENEW_ASSISTANT_AUDIO_UTILS_H

#include <string>
#include <vector>
#include "sw/assistant/errors.h"

namespace sw::assistant {
    
enum class AudioType {
    RECORDING = 0,
    PLAYBACK
};

class SDLError : public Error {
public:
    explicit SDLError(const std::string &msg);
};

class SDLInit {
public:
    SDLInit();

    ~SDLInit();
};

namespace audio_utils {

std::vector<std::string> list_devices(AudioType type);

}

}

#endif // end SEWENEW_ASSISTANT_AUDIO_UTILS_H
