#include <cassert>

#include "bars/fwav.h"

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
    channel_info_ref_table = reader.read_ref_table();
    dsp_adpcm_info_array.resize(channel_info_ref_table.count);
    channel_info_array.resize(channel_info_ref_table.count);

    if (channel_info_ref_table.count > 0) {
        for (int i{0}; i < channel_info_ref_table.count; ++i) {
            Reference channel_info_ref = channel_info_ref_table.items[i];
            reader.seek(channel_info_table_start + channel_info_ref.offset);

            size_t current_offset = reader.tell();
            ChannelInfo channel_info = reader.read<ChannelInfo>();

            channel_info_array[i] = channel_info;

            reader.seek(current_offset + channel_info.to_adpcm_info.offset);
            dsp_adpcm_info_array[i] = reader.read<Fstm::DspAdpcmInfo>();
        }
    }
}

DataBlock::DataBlock(AudioReader& reader) 
{
    header = reader.read<BlockHeader>();
    pcm16.resize((header.section_size / 2) - 4);
    for (auto& sample : pcm16)
        sample = reader.read<uint16_t>();
}

WaveFile::WaveFile(AudioReader& reader) 
{
    std::size_t file_start {reader.tell()};

    header = {reader};
    if (header.bom == 0xFFFE) {
        reader.swap_endianness();
        reader.seek(file_start);
        header = {reader};
    }

    endianness = reader.endianness();

    for (auto& ref : header.block_refs) {
        reader.seek(file_start + ref.offset);
        if (ref.type == 0x7000)
            info = {reader};
        else if (ref.type == 0x7001)
            block = {reader};
    }
}

void WaveFile::serialize(AudioWriter& writer) 
{
    std::size_t file_start {writer.tell()};
    
    writer.write_audio_header(header);
    for (auto& ref : header.block_refs) {
        writer.seek(file_start + ref.offset);
        switch (ref.type) {
        case 0x7000: {
            writer.write<BlockHeader>(info.header);
            writer.write<Fstm::StreamInfo::Codec>(info.codec);
            writer.write<uint8_t>(info.loop_flag);
            writer.seek(writer.tell() + 2);
            writer.write<uint32_t>(info.sample_rate);
            writer.write<uint32_t>(info.loop_start);
            writer.write<uint32_t>(info.sample_count);
            writer.write<uint32_t>(info.og_loop_start);

            size_t ch_info_table_start{writer.tell()};
            writer.write_table<Reference>(info.channel_info_ref_table);

            for (int i{0}; i < info.channel_info_ref_table.count; ++i) {
                size_t to_ch_info{ch_info_table_start +
                                  info.channel_info_ref_table.items[i].offset};
                writer.seek(to_ch_info);
                writer.write<ChannelInfo>(info.channel_info_array[i]);
                writer.seek(to_ch_info +
                            info.channel_info_array[i].to_adpcm_info.offset);
                writer.write<Fstm::DspAdpcmInfo>(info.dsp_adpcm_info_array[i]);
            }
            break;
        }
        case 0x7001: {
            writer.write<BlockHeader>(block.header);
            for (auto& sample : block.pcm16)
                writer.write<int16_t>(sample);
            break;
        }
        default:
            break;
        }
    }
}
} // namespace NSound::Fwav
