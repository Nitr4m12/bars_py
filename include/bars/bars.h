#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <fstream>

#include <oead/util/binary_reader.h>

/*
    sources: 
    https://wiki.oatmealdome.me/Aal/BARS_(File_Format)
    https://github.com/Kinnay/Nintendo-File-Formats/wiki/AMTA-File-Format
    https://github.com/aboood40091/BCFSTM-BCFWAV-Converter
    https://github.com/NanobotZ/bfstp-fixer
*/

namespace NSound {
struct Header {
    std::array<uint8_t, 4> signature;
    uint32_t file_size;
    uint16_t bom;
    uint16_t unknown;
    uint32_t asset_count;
    std::vector<uint32_t> crc32hashes; // size = this.asset_count; must be sorted
    struct FileEntry {
        uint32_t amta_offset;
        uint32_t asset_offset;
    };
    std::vector<FileEntry> file_entries; // size = this.asset_count; same order as the CRC32 hashes
};

struct Reference {
    uint16_t type;
    uint8_t padding[2];
    int32_t offset;
};

struct ReferenceWithSize : Reference {
    uint32_t size;
};

struct BlockHeader {
    std::array<uint8_t, 4> signature;
    uint32_t section_size;
};

struct Unknown {
    std::vector<uint32_t> unknown;
};

struct AmtaHeader {
    std::array<uint8_t, 4> signature;
    uint16_t    bom;
    uint16_t    unknown;
    uint32_t    size;
    uint32_t    data_offset;
    uint32_t    mark_offset;
    uint32_t    ext__offset;
    uint32_t    strg_offset;
};

struct AmtaData {
    enum Type : uint8_t {
        Wave=0, 
        Stream
    };

    BlockHeader header;

    // can be 0, but still have an entry in the string table
    uint32_t    asset_name_offset;

    uint32_t    sample_count;
    Type        type; // 0=Wave, 1=Stream
    uint8_t     wave_channels;
    uint8_t     used_stream_tracks; // Up to 8
    uint8_t     flags;
    float       volume;
    uint32_t    sample_rate;
    uint32_t    loop_start_sample;
    uint32_t    loop_end_sample;
    float       loudness;

    struct StreamTrack {
        uint32_t channel_count;
        float volume;
    };
    std::array<StreamTrack, 8> stream_tracks;

    float       amplitude_peak; // Only in Version 4.0!!!!!
};

struct AmtaMarker {
    BlockHeader     header;
    uint32_t        entry_count;

    struct MarkerInfo {
        uint32_t id;
        // can be 0, but still have an entry in the string table
        uint32_t asset_name_offset;
        uint32_t start_pos;
        uint32_t length;
    };
    std::vector<MarkerInfo> marker_infos; // size = this.entry_count
};

struct AmtaExt {
    BlockHeader header;
    uint32_t entry_count;

    struct ExtEntry {
        uint32_t unknown[2];
    };
    std::vector<ExtEntry> ext_entries; // size = this.entry_count
};

struct AmtaString {
    BlockHeader header;
    std::string asset_name;
};

struct AudioHeader {
    std::array<uint8_t, 4> signature;
    uint16_t    bom;
    uint16_t    head_size;
    uint32_t    version;
    uint32_t    file_size;
    uint16_t    block_count;
    uint16_t    reserved;
};

struct FwavHeader {
    AudioHeader header;
    uint32_t info_offset;
    uint32_t data_offset;
    // padding 32
};

struct StreamInfo {
    uint8_t codec;
    uint8_t loop_flag;
    uint8_t channel_count;
    uint8_t region_count;
    uint32_t sample;
    uint32_t loop_start;
    uint32_t sample_count;
    uint32_t sample_blk_count;
    uint32_t sample_blk_sample_count;
    uint32_t last_sample_size;
    uint32_t last_sample_sample_count;
    uint32_t last_sample_padding_size;
    uint32_t seek_size;
    uint32_t sisc;
};

struct WaveInfo {
    uint8_t codec;
    uint8_t loop_flag;
    // padding 2
    uint32_t sample;
    uint32_t loop_start;
    uint32_t loop_end;
    uint32_t unknown;
};

struct TrackInfo {
    uint8_t volume;
    uint8_t pan;
    uint16_t unknown;
};

struct DspContext {
    uint16_t predictor_scale;
    uint16_t pre_sample;
    uint16_t pre_sample2;
};

struct ImaContext {
    uint16_t data;
    uint16_t table_index;
};

struct RegnInfo {
    uint16_t region_size;
    // padding 2
    uint16_t region_flag;
    // padding 2
    int32_t region_offset;
    uint32_t loop_start;
    uint32_t loop_end;
    uint32_t unknown;
};

}; // namespace NSound

// --------- Helper Functions --------------------------------------------------

NSound::Header get_main_header(const std::string& file_name);
void extract_amta(const NSound::Header header, oead::util::BinaryReader reader);

// -----------------------------------------------------------------------------