#include <iostream>
#include <array>
#include <vector>

namespace BARS {
    struct Header {
        std::array<uint8_t, 4> magic;
        uint32_t file_size;
        uint16_t bom;
        uint16_t unknown;
        uint32_t file_count;

        void read(std::istream& is)
        {
            is.read((char*)& magic, sizeof(magic));
            is.read((char*)& file_size, sizeof(file_size));
            is.read((char*)& bom, sizeof(bom));
            is.read((char*)& unknown, sizeof(unknown));
            is.read((char*)& file_count, sizeof(file_count));
        }
    };

    struct AudioHeader {
        std::array<uint8_t, 4> magic;
        // padding 2
        uint16_t head_size;
        uint32_t version;
        uint32_t file_size;
        uint16_t block_count;
        uint16_t unknown;
    };

    struct TRKStruct {
        std::vector<uint32_t> offsets;
        void read(std::istream& is, int count)
        {
            for (int i {0}; i<count*2; ++i) {
                uint32_t val;
                is.read((char*)& val, sizeof(val));
                offsets.push_back(val);
            }
        }
    };

    struct Unknown {
        std::vector<uint32_t> unknown;
    };

    struct AmtaHeader {
        std::array<uint8_t, 4> magic;
        uint16_t bom;
        uint16_t unknown;
        uint32_t size;
        uint32_t data_offset;
        uint32_t mark_offset;
        uint32_t ext__offset;
        uint32_t strg_offset;

        void read(std::istream& is)
        {
            is.read((char*)& magic, sizeof(magic));
            is.read((char*)& bom, sizeof(bom));
            is.read((char*)& unknown, sizeof(unknown));
            is.read((char*)& size, sizeof(uint32_t));
            is.read((char*)& data_offset, sizeof(uint32_t));
            is.read((char*)& mark_offset, sizeof(uint32_t));
            is.read((char*)& ext__offset, sizeof(uint32_t));
            is.read((char*)& strg_offset, sizeof(uint32_t));
        }
    };

    struct BlkHeader {
        std::array<uint8_t, 4> magic;
        uint32_t section_size;

        void read(std::istream& is)
        {
            is.read((char*)& magic, sizeof(magic));
            is.read((char*)& section_size, sizeof(section_size));
        }
    };

    struct FwavHeader {
        std::array<uint8_t, 4> magic;
        // padding 8
        uint32_t file_size;
        // padding 8
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

    struct Ref {
        uint16_t type;
        // padding 2
        int32_t offset;
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
};