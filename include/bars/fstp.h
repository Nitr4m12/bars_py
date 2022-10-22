#include <array>
#include <cstdint>

#include <bars/common.h>

#ifndef NSOUND_FSTP_H
#define NSOUND_FSTP_H

namespace NSound {
namespace Fstp {
struct TrackInfo {
    uint8_t volume;
    uint8_t pan;
    uint16_t unknown;
};

struct RegionInfo {
    uint16_t region_size;
    uint8_t padding1[2];
    uint16_t region_flag;
    uint8_t padding2[2];
    int32_t region_offset;
    uint32_t og_loop_start;
    uint32_t og_loop_end;
};

struct StreamInfo {
    uint8_t codec;
    bool    is_loop;
    uint8_t channel_count;
    uint8_t region_count;
    uint32_t sample_rate;
    uint32_t loop_start;
    uint32_t sample_count;
    uint32_t block_count;
    uint32_t block_size;
    uint32_t block_sample_count;
    uint32_t last_block_size;
    uint32_t last_block_sample_count;
    uint32_t last_block_padding_size;
    uint32_t seek_size;
    uint32_t sisc;
    RegionInfo region_info;  // Used only if region_count > 0, else omitted
    uint32_t crc32hash;      // Used to check that the revision of a prefetch and stream files match
};

struct PrefetchData {
    uint32_t start_frame;
    uint32_t prefetch_size;
    uint32_t reserved;

    // offset is relative to the start of the PrefetchData
    Reference to_prefetch_sample;
};

struct PrefetchDataBlock {
    BlockHeader header;
    Table<PrefetchData> prefetch_data;
    std::vector<uint8_t> data;
};

struct PrefetchFile {
    AudioHeader header;
    StreamInfo  info;
    PrefetchDataBlock data;
};
} // namespace NSound::Fstp
} // namespace NSound

#endif