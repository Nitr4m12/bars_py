#include <array>
#include <cstdint>

#include <bars/common.h>
#include <bars/fstp.h>

#ifndef NSOUND_FWAV_H
#define NSOUND_FWAV_H

namespace NSound::Fwav {

struct ChannelInfo {
    Reference intoSampleData;
    Reference toAdpcmInfo;
    uint32_t reserved;
};

struct WaveInfo {
    BlockHeader header;
    uint8_t codec;
    uint8_t loop_flag;
    uint8_t padding[2];
    uint32_t sample_rate;
    uint32_t loop_start;
    uint32_t sample_count;
    uint32_t og_loop_start;
    Table<Reference>  channel_info_table;
    std::vector<Fstp::DspAdpcmInfo> dsp_adpcm_info_array;
};

struct DataBlock {
    BlockHeader header;
    std::vector<int16_t> pcm16;
};

struct WaveFile {
    AudioHeader header;
    WaveInfo    info;
    DataBlock   block;
};

DataBlock read_data_block(oead::util::BinaryReader& reader);
WaveInfo read_info_block(oead::util::BinaryReader& reader);
WaveFile read(oead::util::BinaryReader& reader);

} // namespace NSound::Fwav

#endif
