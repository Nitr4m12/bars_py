#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

#include "bars/common.h"
#include "bars/fstp.h"
#include "bars/fwav.h"


namespace NSound {
enum class ReferenceTypes {
    StreamInfo = 0x4000,
    Seek,
    StreamData,
    Region,
    PrefetchData,
    WaveInfo = 0x7000,
    WaveData
};

AudioHeader::AudioHeader(AudioReader& reader) {
    signature = reader.read<typeof(signature)>();
    bom = reader.read<uint16_t>();
    head_size = reader.read<uint16_t>();
    version = reader.read<uint32_t>();
    file_size = reader.read<uint32_t>();
    block_count = reader.read<uint16_t>();
    reserved = reader.read<uint16_t>();

    block_refs.resize(block_count);
    for (auto& block_ref : block_refs)
        block_ref = reader.read<NSound::SizedReference>();
}

void AudioWriter::write_audio_header(AudioHeader header) {
    write(header.signature);
    write<uint16_t>(VALID_BOM);
    write<uint16_t>(header.head_size);
    write<uint32_t>(header.version);
    write<uint32_t>(header.file_size);
    write<uint16_t>(header.block_count);
    write<uint16_t>(header.reserved);

    for (auto& block_ref : header.block_refs)
        write<SizedReference>(block_ref);
}
} // namespace NSound
