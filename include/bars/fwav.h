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
  ReferenceTable  ref_table;
};

struct DataBlock {
  BlockHeader header;
  std::vector<uint16_t> data;
};
} // namespace NSound::Fwav
} // namespace NSound

#endif