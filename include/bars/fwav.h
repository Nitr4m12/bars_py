#include <array>
#include <cstdint>

#include <bars/common.h>
#include <bars/fstp.h>

#ifndef NSOUND_FWAV_H
#define NSOUND_FWAV_H

namespace NSound::Fwav {
struct WaveInfo {
    uint8_t codec;
    uint8_t loop_flag;
    uint32_t sample_rate;
    uint32_t loop_start;
    uint32_t loop_end;
    uint32_t og_loop_start;
    Table<Reference>  channel_info_ref_table;

    WaveInfo() = default;
    WaveInfo(oead::util::BinaryReader& reader);
};

struct DataBlock {
    BlockHeader header;
    std::vector<int16_t> pcm16;

    DataBlock() = default;
    DataBlock(oead::util::BinaryReader& reader);
};

struct WaveFile {
    AudioHeader header;
    WaveInfo    info;
    DataBlock   block;

    WaveFile() = default;
    WaveFile(oead::util::BinaryReader& reader);
};
} // namespace NSound::Fwav

#endif
