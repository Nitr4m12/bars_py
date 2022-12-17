#include <cassert>

#include <bars/fstp.h>
#include <cstdint>
#include <iostream>

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
            dsp_adpcm_info_array.push_back(*reader.Read<DspAdpcmInfo>());

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

void write_track_info(std::ostream& os, TrackInfo& track_info)
{
    os.write(reinterpret_cast<char*>(&track_info.volume), sizeof(uint8_t));
    os.write(reinterpret_cast<char*>(&track_info.pan), sizeof(uint8_t));
    os.write(reinterpret_cast<char*>(&track_info.span), sizeof(uint8_t));
    os.write(reinterpret_cast<char*>(&track_info.flags), sizeof(uint8_t));
}

void write_stream_info(std::ostream& os, StreamInfo& stream_info)
{
    os.write(reinterpret_cast<char*>(&stream_info.codec), sizeof(uint8_t));
    os.write(reinterpret_cast<char*>(&stream_info.is_loop), sizeof(bool));
    os.write(reinterpret_cast<char*>(&stream_info.channel_count), sizeof(uint8_t));
    os.write(reinterpret_cast<char*>(&stream_info.region_count), sizeof(uint8_t));
    os.write(reinterpret_cast<char*>(&stream_info.sample_rate), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.loop_start), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.sample_count), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.block_count), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.block_size), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.block_sample_count), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.last_block_size), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.last_block_padding_size), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.seek_size), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&stream_info.sisc), sizeof(uint32_t));

    write_reference(os, stream_info.to_sample_data);
    os.write(reinterpret_cast<char*>(&stream_info.region_info_size), sizeof(uint16_t));
    os.seekp(2, std::ios_base::cur);

    write_reference(os, stream_info.region_ref);
}

void write_channel_info(std::ostream& os, ChannelInfo& ch_info)
{
    os.write(reinterpret_cast<char*>(&ch_info.adpcm_info_ref.type), sizeof(uint16_t));
    os.seekp(2, std::ios_base::cur);
    os.write(reinterpret_cast<char*>(&ch_info.adpcm_info_ref.offset), sizeof(int32_t));
}

void write_dsp_context(std::ostream& os, DspContext& dsp_ctx)
{
    for (auto& coefficient : dsp_ctx.coefficients)
        os.write((char*) &coefficient, sizeof(uint16_t));

    os.write(reinterpret_cast<char*>(&dsp_ctx.predictor_scale), sizeof(uint16_t));
    os.write(reinterpret_cast<char*>(&dsp_ctx.pre_sample), sizeof(uint16_t));
    os.write(reinterpret_cast<char*>(&dsp_ctx.pre_sample2), sizeof(uint16_t));
}

void write_dsp_lcontext(std::ostream& os, DspLoopContext& dsp_loop_ctx)
{
    os.write(reinterpret_cast<char*>(&dsp_loop_ctx.predictor_scale), sizeof(uint16_t));
    os.write(reinterpret_cast<char*>(&dsp_loop_ctx.loop_pre_sample), sizeof(uint16_t));
    os.write(reinterpret_cast<char*>(&dsp_loop_ctx.loop_pre_sample2), sizeof(uint16_t));
}

void write_adpcm_info(std::ostream& os, DspAdpcmInfo& adpcm_info)
{
    write_dsp_context(os, adpcm_info.context);
    write_dsp_lcontext(os, adpcm_info.loop_context);
}

void write_info_block(std::ostream& os, InfoBlock& info_blk)
{
    os.write(reinterpret_cast<char*>(&info_blk.header.signature), 4);
    os.write(reinterpret_cast<char*>(&info_blk.header.section_size), sizeof(uint32_t));

    write_reference(os, info_blk.stminfo_ref);
    write_reference(os, info_blk.track_info_table_ref);
    write_reference(os, info_blk.channel_info_table_ref);
    write_stream_info(os, info_blk.stream_info);

    os.write(reinterpret_cast<char*>(&info_blk.track_info_table.count), sizeof(uint32_t));
    for (auto& item : info_blk.track_info_table.items)
        write_track_info(os, item);

    os.write(reinterpret_cast<char*>(&info_blk.channel_info_table.count), sizeof(uint32_t));
    for (auto& item : info_blk.channel_info_table.items)
        write_reference(os, item);

    for (auto &entry : info_blk.dsp_adpcm_info_array)
        write_adpcm_info(os, entry);
}

void write_prefetch_data(std::ostream& os, PrefetchData& pdat)
{
    os.write(reinterpret_cast<char*>(&pdat.start_frame), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&pdat.prefetch_size), sizeof(uint32_t));
    os.write(reinterpret_cast<char*>(&pdat.reserved), sizeof(uint32_t));

    write_reference(os, pdat.to_prefetch_samples);
}

void write_pdat_block(std::ostream& os, PrefetchDataBlock& pdat_blk)
{
    os.write(reinterpret_cast<char*>(&pdat_blk.header.signature), 4);
    os.write(reinterpret_cast<char*>(&pdat_blk.header.section_size), sizeof(uint32_t));

    os.write(reinterpret_cast<char*>(&pdat_blk.prefetch_data.count), sizeof(uint32_t));
    for (auto& item : pdat_blk.prefetch_data.items)
        write_prefetch_data(os, item);

    for (auto& sample : pdat_blk.sample_data)
        os.write(reinterpret_cast<char*>(&sample), sizeof(uint8_t));
}

void write(std::ostream& os, PrefetchFile& fstp)
{
    write_audio_header(os, fstp.header);
    write_info_block(os, fstp.info);
    write_pdat_block(os, fstp.pdat);
}

}