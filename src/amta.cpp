#include "bars/amta.h"
#include "bars/common.h"

namespace NSound::Amta {
Strg::Strg(AudioReader& reader) 
{
    header = reader.read<BlockHeader>();
    asset_name = reader.read_string(header.section_size);
}

Ext_::Ext_(AudioReader& reader) 
{
    header = reader.read<BlockHeader>();
    entry_count = reader.read<uint32_t>();

    if (entry_count > 0) {
        ext_entries.resize(entry_count);
        for (auto& entry : ext_entries)
            entry = reader.read<ExtEntry>();
    }
}

Marker::Marker(AudioReader& reader) 
{
    header = reader.read<BlockHeader>();
    entry_count = reader.read<uint32_t>();

    if (entry_count > 0) {
        marker_infos.resize(entry_count);
        for (auto& entry : marker_infos)
            entry = reader.read<MarkerInfo>();
    }
}

AmtaFile::AmtaFile(AudioReader& reader) 
{
    std::size_t file_start {reader.tell()};

    header = reader.read<Header>();
    if (header.bom == 0xFFFE) {
        reader.swap_endianness();
        reader.seek(file_start);
        header = reader.read<Header>();
    }

    endianness = reader.endianness();

    reader.seek(file_start + header.data_offset);
    data = reader.read<Data>();

    reader.seek(file_start + header.marker_offset);
    marker = Marker{reader};

    reader.seek(file_start + header.ext_offset);
    ext = Ext_{reader};

    reader.seek(file_start + header.strg_offset);
    strg = Strg{reader};
}

void AmtaFile::serialize(AudioWriter& writer) 
{
    size_t amta_start{writer.tell()};
    writer.write(header.signature);
    writer.write(header.bom);
    writer.write(header.version);
    writer.write(header.file_size);
    writer.write(header.data_offset);
    writer.write(header.marker_offset);
    writer.write(header.ext_offset);
    writer.write(header.strg_offset);

    writer.seek(amta_start + header.data_offset);
    writer.write(data);

    writer.seek(amta_start + header.marker_offset);
    writer.write(marker.header);
    writer.write(marker.entry_count);
    for (auto& marker_info : marker.marker_infos)
        writer.write(marker_info);

    writer.seek(amta_start + header.ext_offset);
    writer.write(ext.header);
    writer.write(ext.entry_count);
    for (auto& entry : ext.ext_entries)
        writer.write(entry);

    writer.seek(amta_start + header.strg_offset);
    writer.write(strg.header);
    writer.write_cstring(strg.asset_name);
}
} // namespace NSound::Amta
