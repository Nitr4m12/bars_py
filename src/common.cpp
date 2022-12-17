#include <cstdint>
#include <cstdio>
#include <string>
#include <iostream>

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

void write_reference(std::ostream& os, Reference& ref)
{
    os.write(reinterpret_cast<char*>(&ref.type), sizeof(uint16_t));
    os.seekp(2, std::ios_base::cur);
    os.write(reinterpret_cast<char*>(&ref.offset), sizeof(int32_t));
}

void write_sized_reference(std::ostream& os, SizedReference& sref)
{
    os.write(reinterpret_cast<char*>(&sref.type), sizeof(uint16_t));
    os.seekp(2, std::ios_base::cur);
    os.write(reinterpret_cast<char*>(&sref.offset), sizeof(int32_t));
    os.write(reinterpret_cast<char*>(&sref.size), sizeof(uint32_t));
}

void write_audio_header(std::ostream& os, AudioHeader& header)
{
    os.write(reinterpret_cast<char*>(&header.signature), 4);
    os.write(reinterpret_cast<char*>(&header.bom), sizeof(uint16_t));
    os.write(reinterpret_cast<char*>(&header.head_size), sizeof(uint16_t));
    os.write(reinterpret_cast<char*>(&header.version), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&header.file_size), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&header.block_count), sizeof(uint16_t));
    os.write(reinterpret_cast<char*>(&header.reserved), sizeof(uint16_t));

    for (auto& block_ref : header.block_refs)
        write_sized_reference(os, block_ref);
}

}
