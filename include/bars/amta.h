#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

#include <oead/util/binary_reader.h>

#include "bars/common.h"

#ifndef NSOUND_AMTA_H
#define NSOUND_AMTA_H

namespace NSound::Amta {
struct Data {
    enum Type : uint8_t { Wave = 0, Stream };

    BlockHeader header{'D', 'A', 'T', 'A', 0x64};

    // can be 0, but still have an entry in the string table
    uint32_t asset_name_offset{0};

    uint32_t sample_count{0};
    Type type{Wave};
    uint8_t wave_channels{0};
    uint8_t used_stream_tracks{0}; // Up to 8
    uint8_t flags{0};
    uint32_t duration{0};
    uint32_t sample_rate{48000};
    uint32_t loop_start_sample{0};
    uint32_t loop_end_sample{0};
    float loudness{static_cast<float>(0x912bb9c1)};

    struct StreamTrack {
        uint32_t channel_count{0};
        float volume{0x3f800000};
        BINARYIO_DEFINE_FIELDS(StreamTrack, channel_count, volume);
    };
    std::array<StreamTrack, 8> stream_tracks;

    float amplitude_peak{
        static_cast<float>(0x4b52414d)}; // Only in Version 4.0!!!!!

    BINARYIO_DEFINE_FIELDS(Data, header, asset_name_offset, sample_count, type,
                           wave_channels, used_stream_tracks, flags, duration,
                           sample_rate, loop_start_sample, loop_end_sample,
                           loudness, stream_tracks, amplitude_peak);
} __attribute__((packed));

struct Marker {
    BlockHeader header{'M', 'A', 'R', 'K', 0x4};
    uint32_t entry_count;

    struct MarkerInfo {
        uint32_t id;

        // can be 0, but still have an entry in the string table
        uint32_t asset_name_offset;

        uint32_t start_pos;
        uint32_t length;
    };
    std::vector<MarkerInfo> marker_infos; // size = this.entry_count

    Marker() = default;
    Marker(AudioReader& reader);
};

struct Ext_ {
    BlockHeader header{{'E', 'X', 'T', '_'}, 0x4};
    uint32_t entry_count{0};

    struct ExtEntry {
        uint32_t unknown[2];
    };
    std::vector<ExtEntry> ext_entries; // size = this.entry_count

    Ext_() = default;
    Ext_(AudioReader& reader);
};

struct Strg {
    BlockHeader header;
    std::string asset_name;

    Strg() = default;
    Strg(AudioReader& reader);
};

struct Header {
    std::array<uint8_t, 4> signature{'A', 'M', 'T', 'A'};

    uint16_t bom{0xFEFF};
    uint16_t version{0x400}; // 0x100, 0x300 or 0x400
    uint32_t file_size{0};
    uint32_t data_offset{0};
    uint32_t marker_offset{0};
    uint32_t ext_offset{0};
    uint32_t strg_offset{0};

    BINARYIO_DEFINE_FIELDS(Header, signature, bom, version, file_size,
                           data_offset, marker_offset, ext_offset, strg_offset);
};

struct AmtaFile {
    Header header;
    Data data;
    Marker marker;
    Ext_ ext;
    Strg strg;

    binaryio::endian endianness;

    AmtaFile() = default;
    AmtaFile(std::vector<uint8_t>::iterator begin,
             std::vector<uint8_t>::iterator end);

    std::vector<uint8_t> serialize();
};
} // Namespace NSound::Amta

#endif
