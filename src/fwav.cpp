#include <array>
#include <cassert>
#include <iostream>

#include <bars/fwav.h>
#include <oead/util/binary_reader.h>

namespace NSound::Fwav {
WaveInfo read_info_block(oead::util::BinaryReader& reader)
{
    WaveInfo wav_info;
    wav_info.header = *reader.Read<BlockHeader>();
    wav_info.codec = *reader.Read<uint8_t>();
    wav_info.loop_flag = *reader.Read<uint8_t>();
    reader.Seek(reader.Tell() + 2);
    wav_info.sample_rate = *reader.Read<uint32_t>();
    wav_info.loop_start = *reader.Read<uint32_t>();
    wav_info.sample_count = *reader.Read<uint32_t>();
    wav_info.og_loop_start = *reader.Read<uint32_t>();

    size_t channel_info_table_start = reader.Tell();
    wav_info.channel_info_table = read_table<Reference>(reader);
    wav_info.dsp_adpcm_info_array.resize(wav_info.channel_info_table.count);

    if (wav_info.channel_info_table.count > 0) {
        for (int i {0}; i<wav_info.channel_info_table.count; ++i) {
            Reference channel_info_ref = wav_info.channel_info_table.items[i];
            reader.Seek(channel_info_table_start + channel_info_ref.offset);
            size_t current_offset = reader.Tell();
            ChannelInfo channel_info = *reader.Read<ChannelInfo>();

            reader.Seek(current_offset + channel_info.toAdpcmInfo.offset);
            wav_info.dsp_adpcm_info_array[i] = *reader.Read<Fstp::DspAdpcmInfo>();
        }
    }

    return wav_info;
}

DataBlock read_data_block(oead::util::BinaryReader& reader)
{
    DataBlock data;
    data.header = *reader.Read<BlockHeader>();
    data.pcm16.resize(data.header.section_size - 8);
    for (auto &sample : data.pcm16)
        sample = *reader.Read<uint16_t>();

    return data;
}

WaveFile read(oead::util::BinaryReader& reader)
{
    size_t file_start = reader.Tell();
    WaveFile fwav;
    fwav.header = AudioHeader{reader};

    std::array<uint8_t, 4> sign{'F','W','A','V'};
    assert(fwav.header.signature == sign);

    for (auto &ref : fwav.header.block_refs) {
        reader.Seek(file_start + ref.offset);
        if (ref.type == 0x7000)
            fwav.info = read_info_block(reader);
        else if (ref.type == 0x7001)
            fwav.block = read_data_block(reader);
    }

    return fwav;
}
} // NSound::Fwav
