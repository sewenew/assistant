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

#include "sw/assistant/vad.h"
#include <cstring>

namespace sw::assistant {

VadModel::VadModel(const std::string &model_path, int intra_threads, int inter_threads) {
    _init_threads(_session_options, intra_threads, inter_threads);

    _session = std::make_shared<Ort::Session>(_env, model_path.data(), _session_options);
}

std::vector<SpeechChunk> VadModel::predict(std::vector<float> &audio_data, const VadOptions &opts) {
    auto sample_rate_per_ms = opts.sample_rate / 1000;
    auto min_silence = sample_rate_per_ms * opts.min_silence.count();
    auto speech_pad = sample_rate_per_ms * opts.speech_pad.count();
    auto window_size = sample_rate_per_ms * opts.window_size.count();

    const int64_t input_node_dims[2] = {1, window_size};

    const int64_t hc_node_dims[3] = {2, 1, 64};
    const int hc_size = 2 * 1 * 64;
    std::vector<float> h(hc_size);
    std::vector<float> c(hc_size);

    const int64_t sr_node_dims[1] = {1};
    std::vector<int64_t> sr = {opts.sample_rate};

    std::vector<VadChunk> chunks;
    auto time_idx = SteadyTimePoint{};
    for (auto idx = 0U; idx < audio_data.size(); idx += window_size) {
        auto input_ort = Ort::Value::CreateTensor<float>(_memory_info, audio_data.data() + idx, window_size, input_node_dims, 2);
        auto sr_ort = Ort::Value::CreateTensor<int64_t>(_memory_info, sr.data(), sr.size(), sr_node_dims, 1);
        auto h_ort = Ort::Value::CreateTensor<float>(_memory_info, h.data(), h.size(), hc_node_dims, 3);
        auto c_ort = Ort::Value::CreateTensor<float>(_memory_info, c.data(), c.size(), hc_node_dims, 3);
        std::vector<Ort::Value> ort_inputs;
        ort_inputs.push_back(std::move(input_ort));
        ort_inputs.push_back(std::move(sr_ort));
        ort_inputs.push_back(std::move(h_ort));
        ort_inputs.push_back(std::move(c_ort));

        float output = 0.0f;
        try {
            auto ort_outputs = _session->Run(Ort::RunOptions{nullptr},
                    _input_node_names.data(), ort_inputs.data(), ort_inputs.size(),
                    _output_node_names.data(), _output_node_names.size());
            output = ort_outputs[0].GetTensorMutableData<float>()[0];
            auto *hn = ort_outputs[1].GetTensorMutableData<float>();
            std::memcpy(h.data(), hn, hc_size * sizeof(float));
            auto *cn = ort_outputs[2].GetTensorMutableData<float>();
            std::memcpy(c.data(), cn, hc_size * sizeof(float));
        } catch (const Ort::Exception &e) {
            output = -1.0f;
        }

        chunks.emplace_back(SteadyTimePoint(time_idx),
                SteadyTimePoint(time_idx + opts.window_size),
                output);
        time_idx += opts.window_size;
    }

    return _merge_chunks(chunks, opts);
}

void VadModel::_init_threads(Ort::SessionOptions &opts,
        int intra_threads, int inter_threads) {
    opts.SetIntraOpNumThreads(intra_threads);
    opts.SetInterOpNumThreads(inter_threads);
    opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
}

std::vector<SpeechChunk> VadModel::_merge_chunks(const std::vector<VadChunk> &chunks, const VadOptions &opts) const {
    auto triggered = false;
    std::vector<SpeechChunk> speeches;
    SpeechChunk cur;
    SteadyTimePoint temp_end;
    for (const auto &chunk : chunks) {
        if (chunk.prob >= opts.threshold) {
            // Speaking
            temp_end = SteadyTimePoint{};
        }

        if (chunk.prob >= opts.threshold - 0.15 && !triggered) {
            triggered = true;
            cur.start = chunk.start;
            continue;
        }

        if (chunk.prob < opts.threshold && triggered) {
            // Silence
            if (temp_end == SteadyTimePoint{}) {
                temp_end = chunk.start;
            }
            if (chunk.end - temp_end < opts.min_silence) {
                continue;
            } else {
                if (temp_end - cur.start > opts.min_speech) {
                    cur.start -= opts.speech_pad;
                    if (cur.start < SteadyTimePoint{}) {
                        cur.start = SteadyTimePoint{};
                    }
                    cur.end = temp_end + opts.speech_pad;
                    if (cur.end > chunks.back().end) {
                        cur.end = chunks.back().end;
                    }
                    speeches.push_back(cur);
                }
                cur = SpeechChunk{};
                temp_end = SteadyTimePoint{};
                triggered = false;
            }
        }
    }

    if (cur.start > SteadyTimePoint{} && !chunks.empty() && chunks.back().end - cur.start > opts.min_speech) {
        cur.end = chunks.back().end; // DO NOT add padding here.
        speeches.push_back(cur);
    }

    return speeches;
}

}
