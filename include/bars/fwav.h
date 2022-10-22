#include <array>
#include <cstdint>

#include <bars/common.h>

#ifndef NSOUND_FWAV_H
#define NSOUND_FWAV_H

namespace NSound {
namespace Fwav {
struct ChannelInfo {
    Reference sample_ref;
    Reference adpcm_info_ref;
    uint32_t reserved;
};

struct DspContext {
    std::array<uint16_t, 16> coefficients;
    uint16_t predictor_scale;
    uint16_t pre_sample;
    uint16_t pre_sample2;
};

struct DspLoopContext {
    uint16_t predictor_scale;
    uint16_t loop_pre_sample;
    uint16_t loop_pre_sample2;
};

struct DspAdpcmInfo {
    DspContext context;
    DspLoopContext loop_context;
};

struct WaveInfo {
  uint8_t codec;
  uint8_t loop_flag;
  uint32_t sample_rate;
  uint32_t loop_start;
  uint32_t loop_end;
  uint32_t og_loop_start;
  ReferenceTable  channel_info_ref_table;
};

struct DataBlock {
    BlockHeader header;
    union 
    {
        // array of samples
        std::vector<int8_t> pcm8;
        std::vector<int16_t> pcm16;
        std::vector<uint8_t> bytes;
    };
};

struct WaveFile {
    AudioHeader header;
    WaveInfo    info;
    DataBlock   block;
};
} // namespace NSound::Fwav
} // namespace NSound

#endif