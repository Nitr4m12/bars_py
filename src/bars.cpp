#include <fstream>
#include <filesystem>
#include <stdexcept>

#include <bars/bars.h>
#include <bars/amta.h>

std::map<std::string, int> NSound::reference_types
{
    {"StreamInfo",  0x4000},
    {"Seek",        0x4001},
    {"StreamData",  0x4002},
    {"Region",      0x4003},
    {"PrefetchData",0x4004},
    {"WaveInfo",    0x7000},
    {"WaveData",    0x7001}
};
namespace NSound {
std::map<std::string, std::array<uint8_t, 4>> signatures
{
    {"Prefetch",   {'F','S','T','P'}},
    {"Wave",       {'F','W','A','V'}}
};
AudioHeader::AudioHeader(oead::util::BinaryReader& reader)
{
    signature = *reader.Read<typeof(signature)>();
    bom = *reader.Read<uint16_t>();
    head_size = *reader.Read<uint16_t>();
    version = *reader.Read<uint32_t>();
    file_size = *reader.Read<uint32_t>();
    block_count = *reader.Read<uint16_t>();
    reserved = *reader.Read<uint16_t>();

    if (signature == std::array<uint8_t, 4>{'F', 'S', 'T', 'P'}) {
        block_refs.resize(block_count-1);
    }
    else
        block_refs.resize(block_count);
    
    for (auto &block_ref : block_refs)
        block_ref = *reader.Read<NSound::SizedReference>();
}

namespace Bars {

Header::Header(oead::util::BinaryReader& reader) 
{
    signature = *reader.Read<typeof(signature)>();
    file_size = *reader.Read<uint32_t>();
    bom = *reader.Read<uint16_t>();
    version = *reader.Read<uint16_t>();
    asset_count = *reader.Read<uint32_t>();

    crc32hashes.resize(asset_count);
    file_entries.resize(asset_count);

    for (auto &hash : crc32hashes)
        hash = *reader.Read<uint32_t>();

    for (auto &entry : file_entries) {
        entry.amta_offset = *reader.Read<uint32_t>();
        entry.asset_offset = *reader.Read<uint32_t>();
    }
}

} // namespace Bars

} // namespace NSound