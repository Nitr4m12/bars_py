#include <array>
#include <cstdint>

#include <bars/common.h>
#include <ostream>
#include "oead/util/binary_reader.h"

#ifndef NSOUND_FSTP_H
#define NSOUND_FSTP_H

namespace NSound::Fstp {
struct TrackInfo {
    uint8_t volume;
    uint8_t pan;
    uint8_t span;
    uint8_t flags;
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
    Reference to_sample_data; // relative to the start of the prefetch data
    uint16_t region_info_size;
    uint8_t padding[2];
    Reference region_ref;
};

struct ChannelInfo {
    Reference adpcm_info_ref;
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

struct InfoBlock {
    BlockHeader header;

    Reference stminfo_ref;
    Reference track_info_table_ref;
    Reference channel_info_table_ref;

    StreamInfo  stream_info;
    Table<TrackInfo> track_info_table;
    Table<Reference> channel_info_table;
    std::vector<DspAdpcmInfo> dsp_adpcm_info_array;

    InfoBlock() = default;
    InfoBlock(oead::util::BinaryReader& reader);
};

struct PrefetchData {
    uint32_t start_frame;
    uint32_t prefetch_size;
    uint32_t reserved;

    // offset is relative to the start of the PrefetchData
    Reference to_prefetch_samples;

};

struct PrefetchDataBlock {
    BlockHeader header;
    Table<PrefetchData> prefetch_data;
    std::vector<uint8_t> sample_data;

    PrefetchDataBlock() = default;
    PrefetchDataBlock(oead::util::BinaryReader& reader);
};

struct PrefetchFile {
    AudioHeader header;
    InfoBlock   info;
    PrefetchDataBlock pdat;

    PrefetchFile() = default;
    PrefetchFile(oead::util::BinaryReader& reader);
};

void write_track_info(std::ostream& os, TrackInfo& track_info);
void write_stream_info(std::ostream& os, StreamInfo& stm_info);
void write_channel_info(std::ostream& os, ChannelInfo& ch_info);
void write_dsp_context(std::ostream& os, DspContext& dsp_ctx);
void write_dsp_lcontext(std::ostream& os, DspLoopContext& dsp_loop_ctx);
void write_adpcm_info(std::ostream& os, DspAdpcmInfo& adpcm_info);
void write_info_block(std::ostream& os, InfoBlock& info_blk);
void write_prefetch_data(std::ostream& os, PrefetchData& pdat);
void write_pdat_block(std::ostream& os, PrefetchDataBlock& pdat_blk);
void write(std::ostream& os, PrefetchFile& fstp);
} // namespace NSound::Fstp

#endif