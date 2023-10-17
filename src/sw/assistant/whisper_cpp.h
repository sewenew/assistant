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

#ifndef SEWENEW_ASSISTANT_WHISPER_CPP_H
#define SEWENEW_ASSISTANT_WHISPER_CPP_H

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <whisper.h>
#include "sw/assistant/wav.h"

namespace sw::assistant {

struct whisper_params {
    int32_t n_threads    = std::min(4, (int32_t) std::thread::hardware_concurrency());
    int32_t n_processors =  1;
    int32_t offset_t_ms  =  0;
    int32_t offset_n     =  0;
    int32_t duration_ms  =  0;
    int32_t progress_step =  5;
    int32_t max_context  = -1;
    int32_t max_len      =  0;
    int32_t best_of      =  2;
    int32_t beam_size    = -1;

    float word_thold    =  0.01f;
    float entropy_thold =  2.40f;
    float logprob_thold = -1.00f;

    bool speed_up        = false;
    bool debug_mode      = false;
    bool translate       = false;
    bool detect_language = false;
    bool diarize         = false;
    bool tinydiarize     = false;
    bool split_on_word   = false;
    bool no_fallback     = false;
    bool output_txt      = false;
    bool output_vtt      = false;
    bool output_srt      = false;
    bool output_wts      = false;
    bool output_csv      = false;
    bool output_jsn      = false;
    bool output_lrc      = false;
    bool print_special   = false;
    bool print_colors    = false;
    bool print_progress  = false;
    bool no_timestamps   = false;
    bool log_score       = false;

    std::string language  = "en";
    std::string prompt;
    std::string font_path = "/System/Library/Fonts/Supplemental/Courier New Bold.ttf";
    std::string model     = "models/ggml-base.en.bin";

    // [TDRZ] speaker turn string
    std::string tdrz_speaker_turn = " [SPEAKER_TURN]"; // TODO: set from command line

    std::string openvino_encode_device = "CPU";

    std::vector<std::string> fname_inp = {};
    std::vector<std::string> fname_out = {};
};

class WhisperCpp {
public:
    explicit WhisperCpp(const whisper_params &params);

    std::string recognize(const std::vector<uint8_t> &wav, const WavOptions &opts);

private:
    struct WhisperCtxDeleter {
        void operator()(whisper_context *ctx) const {
            if (ctx != nullptr) {
                whisper_free(ctx);
            }
        }
    };

    using WhisperCtxUPtr = std::unique_ptr<whisper_context, WhisperCtxDeleter>;

    whisper_full_params _params(const whisper_params &params) const;

    WhisperCtxUPtr _whisper_ctx;

    whisper_full_params _wparams;

    int _processors = 1;
};

}

#endif // end SEWENEW_ASSISTANT_WHISPER_CPP_H
