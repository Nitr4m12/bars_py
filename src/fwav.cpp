#include <array>
#include <cassert>
#include <iostream>

#include <bars/fwav.h>
#include <oead/util/binary_reader.h>

namespace NSound::Fwav {
WaveInfo::WaveInfo(oead::util::BinaryReader& reader)
{
    codec = *reader.Read<uint8_t>();
    loop_flag = *reader.Read<uint8_t>();
    sample_rate = *reader.Read<uint32_t>();
    loop_start = *reader.Read<uint32_t>();
    loop_end = *reader.Read<uint32_t>();
    og_loop_start = *reader.Read<uint32_t>();

    channel_info_ref_table.count = *reader.Read<uint32_t>();
    channel_info_ref_table.items.resize(channel_info_ref_table.count);
    for (auto &item : channel_info_ref_table.items)
        item = *reader.Read<Reference>();
}

DataBlock::DataBlock(oead::util::BinaryReader& reader)
    :header{*reader.Read<BlockHeader>()}
{
    pcm16.resize(header.section_size - 8);
    for (auto &sample : pcm16)
        sample = *reader.Read<uint16_t>();
}

WaveFile::WaveFile(oead::util::BinaryReader& reader)
{
    size_t file_start = reader.Tell();
    header = {reader};

    std::array<uint8_t, 4> sign{'F','W','A','V'};
    assert(header.signature == sign);

    for (auto &ref : header.block_refs) {
        reader.Seek(file_start + ref.offset);
        if (ref.type == 0x7000)
            info = {reader};
        else if (ref.type == 0x7001)
            block = {reader};
    }
}
} // NSound::Fwav
