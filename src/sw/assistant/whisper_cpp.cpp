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

#include "sw/assistant/whisper_cpp.h"
#include "sw/assistant/errors.h"

namespace sw::assistant {

WhisperCpp::WhisperCpp(const whisper_params &params) {
    _whisper_ctx = WhisperCtxUPtr(whisper_init_from_file(params.model.data()));
    if (!_whisper_ctx) {
        throw Error("failed to load whisper.cpp model");
    }

    _wparams = _params(params);

    _processors = params.n_processors;
}

std::string WhisperCpp::recognize(const std::vector<uint8_t> &wav, const WavOptions &opts) {
    // TODO: what's if wav.size() % 2 != 0?
    std::vector<float> pcmf32;
    auto *wav_f32 = reinterpret_cast<const uint16_t*>(wav.data());
    for (auto idx = 0U; idx < wav.size() / 2; ++idx) {
        pcmf32.push_back(static_cast<float>(wav_f32[idx]) / 32768.0f);
    }

    if (whisper_full_parallel(_whisper_ctx.get(), _wparams, pcmf32.data(), pcmf32.size(), _processors) != 0) {
        throw Error("failed to recognize");
    }

    auto num = whisper_full_n_segments(_whisper_ctx.get());
    std::string result;
    for (auto idx = 0; idx < num; ++idx) {
        const auto *text = whisper_full_get_segment_text(_whisper_ctx.get(), idx);
        if (result.empty()) {
            result += "\n";
        }
        result += text;
    }

    return result;
}

whisper_full_params WhisperCpp::_params(const whisper_params &params) const {
    auto wparams = whisper_full_default_params(WHISPER_SAMPLING_BEAM_SEARCH);
    wparams.print_realtime = false;
    wparams.print_progress   = params.print_progress;
    wparams.print_timestamps = !params.no_timestamps;
    wparams.print_special    = params.print_special;
    wparams.translate        = params.translate;
    wparams.language         = params.language.c_str();
    wparams.detect_language  = params.detect_language;
    wparams.n_threads        = params.n_threads;
    wparams.n_max_text_ctx   = params.max_context >= 0 ? params.max_context : wparams.n_max_text_ctx;
    wparams.offset_ms        = params.offset_t_ms;
    wparams.duration_ms      = params.duration_ms;

    wparams.token_timestamps = params.output_wts || params.max_len > 0;
    wparams.thold_pt         = params.word_thold;
    wparams.max_len          = params.output_wts && params.max_len == 0 ? 60 : params.max_len;
    wparams.split_on_word    = params.split_on_word;

    wparams.speed_up         = params.speed_up;
    wparams.debug_mode       = params.debug_mode;

    wparams.tdrz_enable      = params.tinydiarize; // [TDRZ]

    wparams.initial_prompt   = params.prompt.c_str();

    wparams.greedy.best_of        = params.best_of;
    wparams.beam_search.beam_size = params.beam_size;

    wparams.temperature_inc  = params.no_fallback ? 0.0f : wparams.temperature_inc;
    wparams.entropy_thold    = params.entropy_thold;
    wparams.logprob_thold    = params.logprob_thold;

    return wparams;
}

}
