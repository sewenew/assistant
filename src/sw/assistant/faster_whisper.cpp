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

#include "sw/assistant/faster_whisper.h"
#include <cstdio>
#include "sw/assistant/errors.h"

namespace sw::assistant {

std::string FasterWhisper::recognize(const std::vector<uint8_t> &wav, const WavOptions &opts) {
    WavWriter writer;
    writer.write("test.wav", opts, wav);
    std::string cmd = "python " + _asr_py;
    auto *pipe = popen(cmd.data(), "r");
    if (pipe == nullptr) {
        throw Error("failed to open asr.py");
    }
    std::string result;
    std::array<char, 1024> buffer;
    while (true) {
        auto num = fread(buffer.data(), sizeof(char), buffer.size(), pipe);
        if (num > 0) {
            result += std::string(buffer.data(), num);
        } else {
            break;
        }
    }
    pclose(pipe);

    return result;
}

}
