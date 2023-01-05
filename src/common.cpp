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

AudioHeader::AudioHeader(AudioReader& reader)
{
    signature = reader.read<typeof(signature)>();
    bom = reader.read<uint16_t>();
    head_size = reader.read<uint16_t>();
    version = reader.read<uint32_t>();
    file_size = reader.read<uint32_t>();
    block_count = reader.read<uint16_t>();
    reserved = reader.read<uint16_t>();

    block_refs.resize(block_count);
    for (auto &block_ref : block_refs)
        block_ref = reader.read<NSound::SizedReference>();
}

template<>
void AudioWriter::write<AudioHeader>(AudioHeader data)
{
    writer.Write(data.signature);
    writer.Write<uint16_t>(VALID_BOM);
    writer.Write<uint16_t>(data.head_size);
    writer.Write<uint32_t>(data.version);
    writer.Write<uint32_t>(data.file_size);
    writer.Write<uint16_t>(data.block_count);
    writer.Write<uint16_t>(data.reserved);

    for (auto& block_ref : data.block_refs)
        writer.Write<SizedReference>(block_ref);
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
}
