#include <cstdint>
#include <cstdio>
#include <string>
#include <iostream>

#include "bars/common.h"
#include "bars/fstp.h"
#include "bars/fwav.h"
#include "oead/util/binary_reader.h"

namespace NSound {
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

    block_refs.resize(block_count);
    for (auto &block_ref : block_refs)
        block_ref = *reader.Read<NSound::SizedReference>();
}

void write_reference(oead::util::BinaryWriter& writer, Reference& ref)
{
    writer.Write<uint16_t>(ref.type);
    writer.Seek(writer.Tell() + 2);
    writer.Write<uint32_t>(ref.offset);
}

void write_sized_reference(oead::util::BinaryWriter& writer, SizedReference& sref)
{
    writer.Write<uint16_t>(sref.type);
    writer.Seek(writer.Tell() + 2);
    writer.Write<uint32_t>(sref.offset);
    writer.Write<uint32_t>(sref.size);
}

void write_audio_header(oead::util::BinaryWriter& writer, AudioHeader& header)
{
    writer.Write(header.signature);
    writer.Write<uint16_t>(VALID_BOM);
    writer.Write<uint16_t>(header.head_size);
    writer.Write<uint32_t>(header.version);
    writer.Write<uint32_t>(header.file_size);
    writer.Write<uint16_t>(header.block_count);
    writer.Write<uint16_t>(header.reserved);

    for (auto& block_ref : header.block_refs)
        write_sized_reference(writer, block_ref);
}

}
