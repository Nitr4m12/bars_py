#include <cstdint>
#include <stdexcept>

#include "bars/fstp.h"

namespace NSound::Fstp {
PrefetchDataBlock::PrefetchDataBlock(AudioReader& reader) {
    header = reader.read<BlockHeader>();

    size_t table_start{reader.tell()};
    prefetch_data = reader.read_table<PrefetchData>();

    if (prefetch_data.count != 1)
        // Do any stock ones even exists?
        throw std::runtime_error(
            "Can't read prefetch files with more than one PrefetchData!");

    PrefetchData current_item{prefetch_data.items[0]};

    size_t offset{table_start + sizeof(uint32_t) +
                  current_item.to_prefetch_samples.offset};
    reader.seek(offset);

    sample_data.resize(current_item.prefetch_size);

    for (int i{0}; i < current_item.prefetch_size; ++i)
        sample_data[i] = reader.read<uint8_t>();
}

PrefetchFile::PrefetchFile(std::vector<uint8_t>::iterator begin,
                           std::vector<uint8_t>::iterator end) {

    AudioReader reader{begin.base(), end.base()};

    header = {reader};
    if (header.bom == 0xFFFE) {
        reader.swap_endianness();
        reader.seek(0);
        header = {reader};
    }

    endianness = reader.endianness();

    for (auto& ref : header.block_refs) {
        reader.seek(ref.offset);
        if (ref.type == 0x4000)
            info = {reader};
        else if (ref.type == 0x4004)
            pdat = {reader};
    }
}

std::vector<uint8_t> PrefetchFile::serialize() {
    AudioWriter writer{endianness};

    writer.write_audio_header(header);
    for (auto& ref : header.block_refs) {
        writer.seek(ref.offset);
        switch (ref.type) {
        case 0x4000: {
            writer.write<BlockHeader>(info.header);

            size_t ref_array_start = writer.tell();
            writer.write<Reference>(info.stminfo_ref);
            writer.write<Reference>(info.track_info_table_ref);
            writer.write<Reference>(info.channel_info_table_ref);

            if (info.stminfo_ref.offset != -1) {
                writer.seek(ref_array_start + info.stminfo_ref.offset);
                writer.write<Fstm::StreamInfo>(info.stream_info);
            }

            if (info.track_info_table_ref.offset != -1) {
                writer.seek(ref_array_start + info.track_info_table_ref.offset);
                writer.write_table<Fstm::TrackInfo>(info.track_info_table);
            }

            if (info.channel_info_table_ref.offset != -1) {
                writer.seek(ref_array_start +
                            info.channel_info_table_ref.offset);

                size_t channel_info_table_start = writer.tell();

                writer.write_table<Reference>(info.channel_info_table);

                if (info.channel_info_table.count > 0) {
                    for (int i{0}; i < info.channel_info_table.count; ++i) {
                        Reference channel_info =
                            info.channel_info_table.items[i];

                        writer.seek(channel_info_table_start +
                                    channel_info.offset);

                        size_t ref_start = writer.tell();

                        writer.write<Reference>(info.dsp_adpcm_ref_array[i]);

                        writer.seek(ref_start +
                                    info.dsp_adpcm_ref_array[i].offset);

                        writer.write<Fstm::DspAdpcmInfo>(
                            info.dsp_adpcm_info_array[i]);
                    }
                }
            }
            break;
        }
        case 0x4004: {
            writer.write<BlockHeader>(pdat.header);

            size_t pref_data_start = writer.tell();

            writer.write_table<PrefetchData>(pdat.prefetch_data);

            writer.seek(pref_data_start +
                        pdat.prefetch_data.items[0].to_prefetch_samples.offset);
            for (auto& sample : pdat.sample_data)
                writer.write<uint8_t>(sample);
        }
        default:
            break;
        }
    }

    return writer.finalize();
}
} // namespace NSound::Fstp