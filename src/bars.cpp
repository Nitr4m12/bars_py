#include <fstream>
#include <filesystem>
#include <stdexcept>

#include <bars/bars.h>
#include <bars/amta.h>
#include <bars/fstp.h>
#include <bars/fwav.h>

std::map<std::string, int> NSound::reference_types
{
    {"StreamInfo",  0x4000},
    {"Seek",        0x4001},
    {"StreamData",  0x4002},
    {"Region",      0x4003},
    {"WaveInfo",    0x7000},
    {"WaveData",    0x7001}
};
namespace NSound {
AudioHeader load_audio_header(oead::util::BinaryReader& reader)
{
    AudioHeader header;
    header.signature = *reader.Read<std::array<uint8_t, 4>>();
    header.bom = *reader.Read<uint16_t>();
    header.head_size = *reader.Read<uint16_t>();
    header.version = *reader.Read<uint32_t>();
    header.file_size = *reader.Read<uint32_t>();
    header.block_count = *reader.Read<uint16_t>();
    header.reserved = *reader.Read<uint16_t>();

    if (header.signature == std::array<uint8_t, 4>{'F', 'S', 'T', 'P'}) {
        header.block_refs.resize(header.block_count-1);
    }
    else
        header.block_refs.resize(header.block_count);
    
    for (auto &block_ref : header.block_refs)
        block_ref = *reader.Read<NSound::SizedReference>();

    return header;
}

namespace Bars {

Header load_header(oead::util::BinaryReader& reader)
{
    Header header;
    header.signature = *reader.Read<std::array<uint8_t, 4>>();
    header.file_size = *reader.Read<uint32_t>();
    header.bom = *reader.Read<uint16_t>();
    header.version = *reader.Read<uint16_t>();
    header.asset_count = *reader.Read<uint32_t>();

    header.crc32hashes.resize(header.asset_count);
    header.file_entries.resize(header.asset_count);

    for (auto &hash : header.crc32hashes)
        hash = *reader.Read<uint32_t>();
    
    for (auto &entry : header.file_entries) {
        entry.amta_offset = {*reader.Read<uint32_t>()};
        entry.asset_offset = {*reader.Read<uint32_t>()};
    }

    return header;
}

} // namespace Bars

Parser::Parser(std::string file_name)
{
    buffer.resize(std::filesystem::file_size(file_name));
    std::ifstream ifs {file_name};
    ifs.read((char*)buffer.data(), buffer.size());

    reader = {buffer, oead::util::Endianness::Little};
}

void Parser::load()
{
    Bars::Header header {Bars::load_header(reader)};
    std::vector<Amta::Header> amta_array;
    for (const auto &entry : header.file_entries) {
        Amta::Header amta {*reader.Read<Amta::Header>(entry.amta_offset)};
        reader.Seek(entry.asset_offset);
        AudioHeader audio_file {load_audio_header(reader)};
    }
}
} // namespace NSound