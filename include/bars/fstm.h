#include <cstdint>
#include <vector>

#include "bars/common.h"

#ifndef NSOUND_FSTM_H
#define NSOUND_FSTM_H

namespace NSound::Fstm {
struct TrackInfo {
    uint8_t volume{0};
    uint8_t pan{0};
    uint8_t span{0};
    uint8_t flags{0};

    BINARYIO_DEFINE_FIELDS(TrackInfo, volume, pan, span, flags);
};

struct StreamInfo {
    enum class Codec : uint8_t { PCM_8 = 0, PCM_16, ADPCM };

    Codec codec{Codec::ADPCM};
    bool is_loop{0};
    uint8_t channel_count{0};
    uint8_t region_count{0};
    uint32_t sample_rate{48000};
    uint32_t loop_start{0};
    uint32_t sample_count{0};
    uint32_t block_count{0};
    uint32_t block_size{0};
    uint32_t block_sample_count{0};
    uint32_t last_block_size{0};
    uint32_t last_block_sample_count{0};
    uint32_t last_block_padding_size{0};
    uint32_t seek_size{0};
    uint32_t sisc{0};
    Reference to_sample_data{0x1f00,
                             0}; // relative to the start of the prefetch data
    uint16_t region_info_size{0};
    uint8_t padding[2]{0, 0};
    Reference region_ref{0x4003, 0};

    BINARYIO_DEFINE_FIELDS(StreamInfo, codec, is_loop, channel_count,
                           region_count, sample_rate, loop_start, sample_count,
                           block_count, block_size, block_sample_count,
                           last_block_size, last_block_sample_count,
                           last_block_padding_size, seek_size, sisc,
                           to_sample_data);
};

struct DspContext {
    uint16_t predictor_scale{0};
    int16_t pre_sample{0};
    int16_t pre_sample2{0};

    BINARYIO_DEFINE_FIELDS(DspContext, predictor_scale, pre_sample,
                           pre_sample2);
};

struct DspAdpcmInfo {
    std::array<uint16_t, 16> coefficients;
    DspContext context;
    DspContext loop_context;

    BINARYIO_DEFINE_FIELDS(DspAdpcmInfo, coefficients, context, loop_context);
};

struct HistoryInfo {
    int16_t yn1;
    int16_t yn2;
};

struct RegionInfo {
    uint32_t start;
    uint32_t end;

    // DspContext [number of channels]
    std::vector<DspContext> loop_info;
};

struct RegionBlock {
    BlockHeader header{{'R','E','G','N'},0x100};
    RegionInfo region_info;
};

struct SeekBlock {
    BlockHeader header{{'S','E','E','K'},0x100};

    // History [number of blocks][number of channels]
    std::vector<std::vector<HistoryInfo>> history;
};

struct InfoBlock {
    BlockHeader header{'I', 'N', 'F', 'O', 0x100};

    Reference stminfo_ref{0x4100, 0};
    Reference track_info_table_ref{0x100, 0};
    Reference channel_info_table_ref{0x101, 0};

    StreamInfo stream_info;
    Table<TrackInfo> track_info_table;
    ReferenceTable channel_info_table;
    std::vector<Reference> dsp_adpcm_ref_array;
    std::vector<DspAdpcmInfo> dsp_adpcm_info_array;

    InfoBlock() = default;
    InfoBlock(AudioReader& reader);
};

struct StreamData {
    BlockHeader header{{'D', 'A', 'T', 'A'}, 0x0};
    std::vector<uint8_t> sample_data;
};

struct StreamFile {
    AudioHeader header;
    InfoBlock info;
    SeekBlock seek;
    RegionBlock region;
};

} // namespace NSound::Fstm
#endif