#include <array>
#include <cstdint>

#include "bars/common.h"
#include "bars/fstm.h"
#include "bars/fstp.h"

#ifndef NSOUND_FWAV_H
#define NSOUND_FWAV_H

namespace NSound::Fwav {
struct DataBlock {
    BlockHeader header{'D', 'A', 'T', 'A', 0x0};
    std::vector<int16_t> pcm16;

    DataBlock() = default;
    DataBlock(AudioReader& reader);
};

struct ChannelInfo {
    Reference into_sample_data;
    Reference to_adpcm_info;
    uint32_t reserved;

    BINARYIO_DEFINE_FIELDS(ChannelInfo, into_sample_data, to_adpcm_info, reserved);
};

struct WaveInfo {
    BlockHeader header{'I', 'N', 'F', 'O', 0xc0};
    Fstm::StreamInfo::Codec codec{Fstm::StreamInfo::Codec::PCM_16};
    uint8_t loop_flag{0};
    uint32_t sample_rate{48000};
    uint32_t loop_start{0};
    uint32_t sample_count{0};
    uint32_t og_loop_start{0};
    Table<Reference> channel_info_ref_table;
    std::vector<ChannelInfo> channel_info_array;
    std::vector<Fstm::DspAdpcmInfo> dsp_adpcm_info_array;

    WaveInfo() = default;
    WaveInfo(AudioReader& reader);
};

struct WaveFile {
    AudioHeader header;
    WaveInfo info;
    DataBlock block;

    binaryio::endian endianness;

    WaveFile() = default;
    WaveFile(std::vector<uint8_t>::iterator begin,
             std::vector<uint8_t>::iterator end);

    std::vector<uint8_t> serialize();
};
} // namespace NSound::Fwav

#endif
