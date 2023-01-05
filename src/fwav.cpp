#include <array>
#include <cassert>
#include <iostream>

#include <bars/fwav.h>
#include <oead/util/binary_reader.h>

namespace NSound::Fwav {
WaveInfo::WaveInfo(AudioReader& reader)
{
    header = reader.read<BlockHeader>();
    codec = reader.read<Fstm::StreamInfo::Codec>();
    loop_flag = reader.read<uint8_t>();
    reader.seek(reader.tell() + 2);
    sample_rate = reader.read<uint32_t>();
    loop_start = reader.read<uint32_t>();
    sample_count = reader.read<uint32_t>();
    og_loop_start = reader.read<uint32_t>();

    size_t channel_info_table_start = reader.tell();
    channel_info_table = reader.read_ref_table();
    dsp_adpcm_info_array.resize(channel_info_table.count);

    if (channel_info_table.count > 0) {
        for (int i {0}; i<channel_info_table.count; ++i) {
            Reference channel_info_ref = channel_info_table.items[i];
            reader.seek(channel_info_table_start + channel_info_ref.offset);
            size_t current_offset = reader.tell();
            ChannelInfo channel_info = reader.read<ChannelInfo>();

            reader.seek(current_offset + channel_info.toAdpcmInfo.offset);
            dsp_adpcm_info_array[i] = reader.read<Fstm::DspAdpcmInfo>();
        }
    }
}

DataBlock::DataBlock(AudioReader& reader)
{
    header = reader.read<BlockHeader>();
    pcm16.resize(header.section_size - 8);
    for (auto &sample : pcm16)
        sample = reader.read<uint16_t>();
}

WaveFile::WaveFile(AudioReader& reader)
{
    size_t file_start = reader.tell();
    header = {reader};

    for (auto &ref : header.block_refs) {
        reader.seek(file_start + ref.offset);
        if (ref.type == 0x7000)
            info = {reader};
        else if (ref.type == 0x7001)
            block = {reader};
    }
}
} // NSound::Fwav
