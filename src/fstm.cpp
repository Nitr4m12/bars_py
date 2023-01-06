#include "bars/fstm.h"

namespace NSound::Fstm {
InfoBlock::InfoBlock(AudioReader& reader) {
    header = reader.read<BlockHeader>();

    size_t ref_array_start = reader.tell();

    stminfo_ref = reader.read<Reference>();
    if (stminfo_ref.offset != -1) {
        reader.seek(ref_array_start + stminfo_ref.offset);
        stream_info = reader.read<StreamInfo>();
        reader.seek(ref_array_start + sizeof(Reference));
    }

    track_info_table_ref = reader.read<Reference>();
    if (track_info_table_ref.offset != -1) {
        reader.seek(ref_array_start + track_info_table_ref.offset);
        track_info_table = reader.read_table<TrackInfo>();
        reader.seek(ref_array_start + sizeof(Reference) * 2);
    }
    else
        track_info_table = Table<TrackInfo>{0};

    channel_info_table_ref = reader.read<Reference>();
    if (channel_info_table_ref.offset != -1) {
        reader.seek(ref_array_start + channel_info_table_ref.offset);
        size_t channel_info_table_start = reader.tell();

        channel_info_table = reader.read_ref_table();
        dsp_adpcm_ref_array.resize(channel_info_table.count);
        dsp_adpcm_info_array.resize(channel_info_table.count);

        if (channel_info_table.count > 0) {
            for (int i{0}; i < channel_info_table.count; ++i) {
                Reference channel_info = channel_info_table.items[i];
                reader.seek(channel_info_table_start + channel_info.offset);

                size_t current_offset = reader.tell();
                dsp_adpcm_ref_array[i] = reader.read<Reference>();

                reader.seek(current_offset + dsp_adpcm_ref_array[i].offset);

                dsp_adpcm_info_array[i] = reader.read<DspAdpcmInfo>();
            }
        }
    }
}
} // namespace NSound::Fstm