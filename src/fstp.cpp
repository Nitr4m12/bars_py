#include <cassert>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>


#include "bars/fstp.h"
#include "oead/util/binary_reader.h"
#include "oead/util/swap.h"

namespace NSound::Fstp {
PrefetchDataBlock read_pdat_block(oead::util::BinaryReader& reader)
{
    PrefetchDataBlock pdat_block;
    pdat_block.header = *reader.Read<BlockHeader>();

    std::array<uint8_t, 4> sign {'P', 'D', 'A', 'T'};
    assert(pdat_block.header.signature == sign);

    size_t table_start {reader.Tell()};
    pdat_block.prefetch_data = read_table<PrefetchData>(reader);

    if (pdat_block.prefetch_data.count != 1)
        // Do any stock ones even exists?
        throw std::runtime_error("Can't read prefetch files with more than one PrefetchData!");

    // Calculate the offset by adding the start of the table + count
    // and the amount of PrefetchData we have already read and adding
    // that to the offset of the current entry in the table
    PrefetchData current_item {pdat_block.prefetch_data.items[0]};

    size_t offset {table_start + sizeof(uint32_t) + sizeof(PrefetchData) + current_item.to_prefetch_samples.offset};
    reader.Seek(offset);

    pdat_block.sample_data.resize(current_item.prefetch_size);

    for (int i {0}; i<current_item.prefetch_size; ++i)
        pdat_block.sample_data[i] = *reader.Read<uint8_t>();

    return pdat_block;
}

InfoBlock read_info_block(oead::util::BinaryReader& reader)
{
    InfoBlock info_block;
    info_block.header = *reader.Read<BlockHeader>();

    size_t ref_array_start = reader.Tell();

    info_block.stminfo_ref = *reader.Read<Reference>();
    if (info_block.stminfo_ref.offset != -1) {
        reader.Seek(ref_array_start + info_block.stminfo_ref.offset);
        info_block.stream_info = *reader.Read<StreamInfo>();
        reader.Seek(ref_array_start + sizeof(Reference));
    }

    info_block.track_info_table_ref = *reader.Read<Reference>();
    if (info_block.track_info_table_ref.offset != -1) {
        reader.Seek(ref_array_start + info_block.track_info_table_ref.offset);
        info_block.track_info_table = read_table<TrackInfo>(reader);
        reader.Seek(ref_array_start + sizeof(Reference) * 2);
    }
    else
        info_block.track_info_table = Table<TrackInfo>{0};

    info_block.channel_info_table_ref = *reader.Read<Reference>();
    if (info_block.channel_info_table_ref.offset != -1) {
        reader.Seek(ref_array_start + info_block.channel_info_table_ref.offset);
        size_t channel_info_table_start = reader.Tell();

        info_block.channel_info_table = read_table<Reference>(reader);
        info_block.dsp_adpcm_info_array.resize(info_block.channel_info_table.count);


        if (info_block.channel_info_table.count > 0) {
            for (int i {0}; i<info_block.channel_info_table.count; ++i) {
                Reference channel_info = info_block.channel_info_table.items[i];
                reader.Seek(channel_info_table_start + channel_info.offset);

                size_t current_offset = reader.Tell();
                Reference dsp_adpcm_ref = *reader.Read<Reference>();

                reader.Seek(current_offset + dsp_adpcm_ref.offset);

                info_block.dsp_adpcm_info_array[i] = *reader.Read<DspAdpcmInfo>();
            }
        }
    }

    return info_block;
}

PrefetchFile read(oead::util::BinaryReader& reader)
{
    PrefetchFile fstp_file;
    size_t file_start = reader.Tell();
    fstp_file.header = AudioHeader{reader};

    std::array<uint8_t, 4> sign{'F', 'S', 'T', 'P'};
    assert(fstp_file.header.signature == sign);

    for (auto &ref : fstp_file.header.block_refs) {
        reader.Seek(file_start + ref.offset);
        if (ref.type == 0x4000)
            fstp_file.info = read_info_block(reader);
        else if (ref.type == 0x4004)
            fstp_file.pdat = read_pdat_block(reader);
    }

    return fstp_file;
}

void write_track_info(oead::util::BinaryWriter& writer, TrackInfo& track_info)
{
    writer.Write<uint8_t>(track_info.volume);
    writer.Write<uint8_t>(track_info.pan);
    writer.Write<uint8_t>(track_info.span);
    writer.Write<uint8_t>(track_info.flags);
}

void write_stream_info(oead::util::BinaryWriter& writer, StreamInfo& stream_info)
{
    writer.Write<uint8_t>(stream_info.codec);
    writer.Write<uint8_t>(stream_info.is_loop);
    writer.Write<uint8_t>(stream_info.channel_count);
    writer.Write<uint8_t>(stream_info.region_count);
    writer.Write<uint32_t>(stream_info.sample_rate);
    writer.Write<uint32_t>(stream_info.loop_start);
    writer.Write<uint32_t>(stream_info.sample_count);
    writer.Write<uint32_t>(stream_info.block_count);
    writer.Write<uint32_t>(stream_info.block_size);
    writer.Write<uint32_t>(stream_info.block_sample_count);
    writer.Write<uint32_t>(stream_info.last_block_size);
    writer.Write<uint32_t>(stream_info.last_block_sample_count);
    writer.Write<uint32_t>(stream_info.last_block_padding_size);
    writer.Write<uint32_t>(stream_info.seek_size);
    writer.Write<uint32_t>(stream_info.sisc);

    write_reference(writer, stream_info.to_sample_data);

    writer.Write<uint16_t>(stream_info.region_info_size);
    writer.Seek(writer.Tell() + 2);

    write_reference(writer, stream_info.region_ref);
}

void write_dsp_context(oead::util::BinaryWriter& writer, DspContext& dsp_ctx)
{
    for (auto& coefficient : dsp_ctx.coefficients)
        writer.Write<uint16_t>(coefficient);

    writer.Write<uint16_t>(dsp_ctx.predictor_scale);
    writer.Write<uint16_t>(dsp_ctx.pre_sample);
    writer.Write<uint16_t>(dsp_ctx.pre_sample2);
}

void write_dsp_lcontext(oead::util::BinaryWriter& writer, DspLoopContext& dsp_loop_ctx)
{
    writer.Write(dsp_loop_ctx.predictor_scale);
    writer.Write(dsp_loop_ctx.loop_pre_sample);
    writer.Write(dsp_loop_ctx.loop_pre_sample2);
}

void write_adpcm_info(oead::util::BinaryWriter& writer, DspAdpcmInfo& adpcm_info)
{
    write_dsp_context(writer, adpcm_info.context);
    write_dsp_lcontext(writer, adpcm_info.loop_context);
}

void write_info_block(oead::util::BinaryWriter& writer, InfoBlock& info_blk)
{
    writer.Write("INFO");
    writer.Write<uint32_t>(info_blk.header.section_size);

    write_reference(writer, info_blk.stminfo_ref);
    write_reference(writer, info_blk.track_info_table_ref);
    write_reference(writer, info_blk.channel_info_table_ref);
    write_stream_info(writer, info_blk.stream_info);

    writer.Write<uint32_t>(info_blk.track_info_table.count);
    for (auto& track_info : info_blk.track_info_table.items)
        write_track_info(writer, track_info);

    writer.Write<uint32_t>(info_blk.channel_info_table.count);
    for (auto& channel_info : info_blk.channel_info_table.items)
        write_reference(writer, channel_info);

    // TODO: Write array of references to each dsp_adpcm_info entry
    for (auto& entry : info_blk.dsp_adpcm_info_array) {
        write_adpcm_info(writer, entry);
    }
}

void write_prefetch_data(oead::util::BinaryWriter& writer, PrefetchData& pdat)
{

    writer.Write<uint32_t>(pdat.start_frame);
    writer.Write<uint32_t>(pdat.prefetch_size);
    writer.Write<uint32_t>(pdat.reserved);

    write_reference(writer, pdat.to_prefetch_samples);
}

void write_pdat_block(oead::util::BinaryWriter& writer, PrefetchDataBlock& pdat_blk)
{
    writer.Write("PDAT");
    writer.Write<uint32_t>(pdat_blk.header.section_size);
    writer.Write<uint32_t>(pdat_blk.prefetch_data.count);
    size_t prefetch_data_start = writer.Tell();
    for (auto& item : pdat_blk.prefetch_data.items)
        write_prefetch_data(writer, item);

    for (auto& sample : pdat_blk.sample_data)
        writer.Write<uint8_t>(sample);
}

std::vector<uint8_t> write(PrefetchFile& fstp)
{
    oead::util::BinaryWriter writer{oead::util::Endianness::Little};
    write_audio_header(writer, fstp.header);
    writer.AlignUp(0x20);
    write_info_block(writer, fstp.info);
    writer.AlignUp(0x20);
    write_pdat_block(writer, fstp.pdat);
    writer.AlignUp(0x20);
    return writer.Finalize();
}

}