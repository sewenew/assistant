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

#ifndef SEWENEW_ASSISTANT_VAD_H
#define SEWENEW_ASSISTANT_VAD_H

#include <chrono>
#include <string>
#include "onnxruntime_cxx_api.h"

namespace sw::assistant {

struct VadOptions {
    int sample_rate = 16000;

    std::chrono::milliseconds min_silence = std::chrono::milliseconds(2000);
    std::chrono::milliseconds min_speech = std::chrono::milliseconds(250);
    std::chrono::milliseconds speech_pad = std::chrono::milliseconds(400);
    // Window size should be 32ms, 64ms, or 96ms. Otherwise, you might get performance issue.
    std::chrono::milliseconds window_size = std::chrono::milliseconds(64);

    float threshold = 0.5f;
};

using SteadyTimePoint = std::chrono::time_point<std::chrono::steady_clock>;

struct VadChunk {
    VadChunk(const SteadyTimePoint &s, const SteadyTimePoint &e, float p) :
        start(s), end(e), prob(p) {}

    SteadyTimePoint start;
    SteadyTimePoint end;
    float prob = 0.0f;
};

struct SpeechChunk {
    SteadyTimePoint start = SteadyTimePoint{std::chrono::milliseconds(-1)};
    SteadyTimePoint end;
};

class VadModel {
public:
    explicit VadModel(const std::string &model_path, int intra_threads = 1, int inter_threads = 1);

    std::vector<SpeechChunk> predict(std::vector<float> &data, const VadOptions &opts = {});

private:
    void _init_threads(Ort::SessionOptions &opts, int intra_threads, int inter_threads);

    std::vector<SpeechChunk> _merge_chunks(const std::vector<VadChunk> &chunks, const VadOptions &opts) const;

    std::vector<const char *> _input_node_names = {"input", "sr", "h", "c"};

    std::vector<const char *> _output_node_names = {"output", "hn", "cn"};

    Ort::Env _env;
    Ort::SessionOptions _session_options;
    std::shared_ptr<Ort::Session> _session;
    Ort::MemoryInfo _memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);
};

}

#endif // end SEWENEW_ASSISTANT_VAD_H
