#include <string>

#include <bars/common.h>

namespace NSound {
std::map<std::string, std::array<uint8_t, 4>> signatures {
    {"AMTA", {'A', 'M', 'T', 'A'}},
    {"BARS", {'B', 'A', 'R', 'S'}},
    {"MARK", {'M', 'A', 'R', 'K'}},
    {"DATA", {'D', 'A', 'T', 'A'}},
    {"EXT_", {'E', 'X', 'T', '_'}},
    {"STRG", {'S', 'T', 'R', 'G'}},
    {"FWAV", {'F', 'W', 'A', 'V'}},
    {"FSTP", {'F', 'S', 'T', 'P'}},
    {"INFO", {'I', 'N', 'F', 'O'}},
    {"PDAT", {'P', 'D', 'A', 'T'}}
};

enum class ReferenceTypes
{
    StreamInfo=0x4000,
    Seek,
    StreamData,
    Region,
    PrefetchData,
    WaveInfo=0x7000,
    WaveData
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

    if (signature == signatures["FSTP"]) {
        block_refs.resize(block_count-1); // Account for missing REGN block
    }
    else
        block_refs.resize(block_count);
    
    for (auto &block_ref : block_refs)
        block_ref = *reader.Read<NSound::SizedReference>();
}

}
