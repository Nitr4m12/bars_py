#include <cassert>

#include <bars/fstp.h>

namespace NSound::Fstp {
InfoBlock::InfoBlock(oead::util::BinaryReader& reader)
{
    header = *reader.Read<BlockHeader>();

    std::array<uint8_t, 4> sign {'I', 'N', 'F', 'O'};
    assert(header.signature == sign);

    size_t ref_start = reader.Tell();

    stminfo_ref = *reader.Read<Reference>();
    track_info_table_ref = *reader.Read<Reference>();
    channel_info_table_ref = *reader.Read<Reference>();

    if (stminfo_ref.offset != -1 || stminfo_ref.offset != 0){
        reader.Seek(ref_start + stminfo_ref.offset);
        stream_info = *reader.Read<StreamInfo>();
    }

    if (track_info_table_ref.offset != -1 || track_info_table_ref.offset != 0) {
        reader.Seek(ref_start + stminfo_ref.offset);

        track_info_table.count = *reader.Read<uint32_t>();
        track_info_table.items.resize(track_info_table.count);

        for (auto &item : track_info_table.items)
            item = *reader.Read<TrackInfo>();
    }

    if (channel_info_table_ref.offset != -1 || channel_info_table_ref.offset != 0) {
        reader.Seek(ref_start + channel_info_table_ref.offset);

        channel_info_table.count = *reader.Read<uint32_t>();
        channel_info_table.items.resize(channel_info_table.count);

        for (auto &item : channel_info_table.items) {
            item = *reader.Read<Reference>();
            size_t next_ref = reader.Tell();

            reader.Seek(ref_start + channel_info_table_ref.offset);

            size_t dsp_ref_start = reader.Tell();
            Reference dsp_ref = *reader.Read<Reference>();

            reader.Seek(dsp_ref_start + dsp_ref.offset);
            dsp_adpcm_info.push_back(*reader.Read<DspAdpcmInfo>());

            reader.Seek(next_ref);
        }
            
    }
}

PrefetchDataBlock::PrefetchDataBlock(oead::util::BinaryReader& reader)
{
    header = *reader.Read<BlockHeader>();

    std::array<uint8_t, 4> sign {'P', 'D', 'A', 'T'};
    assert(header.signature == sign);

    prefetch_data.count = *reader.Read<uint32_t>();
    prefetch_data.items.resize(prefetch_data.count);

    size_t pref_start = reader.Tell();

    for (auto &entry : prefetch_data.items)
        entry = *reader.Read<PrefetchData>();

    for (auto &entry : prefetch_data.items) {
        reader.Seek(pref_start + entry.to_prefetch_samples.offset);
        sample_data.resize(entry.prefetch_size);
        
        for (uint8_t &byte : sample_data)
            byte = *reader.Read<uint8_t>();
    }

}

PrefetchFile::PrefetchFile(oead::util::BinaryReader& reader)
{
    size_t file_start = reader.Tell();
    header = AudioHeader{reader};

    std::array<uint8_t, 4> sign{'F', 'S', 'T', 'P'};
    assert(header.signature == sign);

    for (auto &ref : header.block_refs) {
        reader.Seek(file_start + ref.offset);
        if (ref.type == 0x4000)
            info = InfoBlock{reader};
        else if (ref.type == 0x4004)
            pdat = PrefetchDataBlock{reader};
    }
}

}