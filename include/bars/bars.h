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
struct BlockHeader {
    std::array<uint8_t, 4>  signature;
    uint32_t    section_size;
};

struct Reference {
    uint16_t        type;
    uint8_t         padding[2];
    int32_t         offset;
};

struct SizedReference : Reference {
    uint32_t size;
};

struct ReferenceTable {
    uint32_t                     count;
    std::vector<Reference>       refs;
};

namespace Amta {
    struct Data {
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
            uint32_t    channel_count;
            float       volume;
        };
        std::array<StreamTrack, 8>  stream_tracks;

        float   amplitude_peak; // Only in Version 4.0!!!!!
    };

    struct Marker {
        BlockHeader header;
        uint32_t    entry_count;

        struct MarkerInfo {
            uint32_t    id;

            // can be 0, but still have an entry in the string table
            uint32_t    asset_name_offset;

            uint32_t    start_pos;
            uint32_t    length;
        };
        std::vector<MarkerInfo> marker_infos; // size = this.entry_count
    };

    struct Ext_ {
        BlockHeader header;
        uint32_t    entry_count;

        struct ExtEntry {
            uint32_t    unknown[2];
        };
        std::vector<ExtEntry>   ext_entries; // size = this.entry_count
    };

    struct StringTable {
        BlockHeader header;
        std::string asset_name;
    };

    struct Header {
        std::array<uint8_t, 4>      signature;

        uint16_t    bom;
        uint16_t    version; // 0x100, 0x300 or 0x400
        uint32_t    file_size;
        std::unique_ptr<Data>           data;
        std::unique_ptr<Marker>         marker;
        std::unique_ptr<Ext_>           ext_;
        std::unique_ptr<StringTable>    string_table;

        void load_data(oead::util::BinaryReader& reader)
        {
            signature   =   *reader.Read<std::array<uint8_t, 4>>();
            bom         =   *reader.Read<uint16_t>();
            version     =   *reader.Read<uint16_t>();
            file_size   =   *reader.Read<uint32_t>();
            
            Data data_section {*reader.Read<Data>()};
            data = std::make_unique<Data>(data_section);

            BlockHeader marker_header       {*reader.Read<BlockHeader>()};
            uint32_t    marker_entry_count  {*reader.Read<uint32_t>()};
            Marker      marker_section      {marker_header, marker_entry_count};
            marker = std::make_unique<Marker>(marker_section);

            BlockHeader ext__header         {*reader.Read<BlockHeader>()};
            uint32_t    ext__entry_count    {*reader.Read<uint32_t>()};
            Ext_        ext__section        {ext__header, ext__entry_count};
            ext_ = std::make_unique<Ext_>(ext__section);

            BlockHeader strg_header         {*reader.Read<BlockHeader>()};
            std::string asset_name          {reader.ReadString(reader.Tell(), strg_header.section_size)};
            StringTable strg_section        {strg_header, asset_name};
            string_table = std::make_unique<StringTable>(strg_section);
        }
    };
} // Namespace Amta

namespace Bars {
    struct InfoBlock {
        uint8_t         codec;
        uint8_t         loop_flag;
        uint8_t         channel_count;
        uint8_t         region_count;
        uint32_t        sample_rate;
        uint32_t        loop_start;
        uint32_t        loop_end;
        uint32_t        og_loop_start;
        ReferenceTable  ref_table;
    };

    struct TrackInfo {
        uint8_t     volume;
        uint8_t     pan;
        uint16_t    unknown;
    };

    struct DspContext {
        uint16_t    predictor_scale;
        uint16_t    pre_sample;
        uint16_t    pre_sample2;
    };

    struct ImaContext {
        uint16_t    data;
        uint16_t    table_index;
    };

    struct RegnInfo {
        uint16_t    region_size;
        uint8_t     padding1[2];
        uint16_t    region_flag;
        uint8_t     padding2[2];
        int32_t     region_offset;
        uint32_t    loop_start;
        uint32_t    loop_end;
        uint32_t    unknown;
    };

    struct AudioHeader {
        std::array<uint8_t, 4> signature;
        uint16_t    bom;
        uint16_t    head_size;
        uint32_t    version;
        uint32_t    file_size;
        uint16_t    block_count;
        uint16_t    reserved;
        SizedReference info_reference;
        SizedReference data_reference;
        uint8_t     padding[20];
    };

    struct Header {
        std::array<uint8_t, 4>  signature;

        uint32_t    file_size;
        uint16_t    bom;
        uint16_t    version;
        uint32_t    asset_count;

        std::vector<uint32_t>   crc32hashes; // size = this.asset_count; must be sorted
        
        struct FileEntry {
            std::unique_ptr<Amta::Header>   amta;
            std::unique_ptr<AudioHeader>    asset;
        };
        std::vector<FileEntry>  file_entries; // size = this.asset_count; same order as the CRC32 hashes
    };



    class Parser {
    public:
        Parser(const std::string& file_name)
        {
            buffer.resize(std::filesystem::file_size(file_name));
            {
                std::ifstream ifs {file_name, std::ios_base::binary};
                ifs.read((char*)buffer.data(), buffer.size());
            }

            reader = {buffer, oead::util::Endianness::Little};
        }

        Header read()
        {
            Header header;

            // Reading std::optional, which defines 
            // * and -> for accesing its value
            header.signature    =   *reader.Read<std::array<uint8_t, 4>>();
            header.file_size    =   *reader.Read<uint32_t>();
            header.bom          =   *reader.Read<uint16_t>();
            header.version      =   *reader.Read<uint16_t>();
            header.asset_count  =   *reader.Read<uint32_t>();
            header.crc32hashes.resize(header.asset_count);
            header.file_entries.resize(header.asset_count);

            for (int i {0}; i<header.asset_count; ++i)
                header.crc32hashes[i] = *reader.Read<uint32_t>();

            for (int i {0}; i<header.asset_count; ++i) {
                Header::FileEntry file_entry;

                uint32_t amta_offset {*reader.Read<uint32_t>()};
                uint32_t file_offset {*reader.Read<uint32_t>()};

                reader.Seek(amta_offset);
                std::unique_ptr<Amta::Header>   amta;
                amta->load_data(reader);

                reader.Seek(file_offset);
                std::unique_ptr<AudioHeader>     audio_file;

            }

            return header;
        }
    private:
        oead::util::BinaryReader reader;
        std::vector<uint8_t> buffer;
    };
} // namespace Bars
}; // namespace NSound

// --------- Helper Functions --------------------------------------------------

NSound::Bars::Header get_main_header(const std::string& file_name);
void extract_amta(const NSound::Bars::Header header, oead::util::BinaryReader reader);

// -----------------------------------------------------------------------------