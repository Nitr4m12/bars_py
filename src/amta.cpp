#include <bars/amta.h>
#include <bars/common.h>
#include <oead/util/binary_reader.h>

namespace NSound::Amta {
AmtaFile::AmtaFile(oead::util::BinaryReader& reader)
{
    size_t amta_start = reader.Tell();
    header = *reader.Read<Header>();
    data = *reader.Read<Data>(amta_start + header.data_offset);
    
    reader.Seek(amta_start + header.marker_offset);
    marker = Marker{reader};

    reader.Seek(amta_start + header.ext__offset);
    ext_ = Ext_{reader};

    reader.Seek(amta_start + header.string_table_offset);
    strg_table = StringTable{reader};
}

Marker::Marker(oead::util::BinaryReader& reader)
{
    header = *reader.Read<BlockHeader>();
    entry_count = *reader.Read<uint32_t>();

    if (entry_count > 0) {
        marker_infos.resize(entry_count);
        for (auto &entry : marker_infos)
            entry = *reader.Read<MarkerInfo>();
    }
}

Ext_::Ext_(oead::util::BinaryReader& reader)
{
    header = *reader.Read<BlockHeader>();
    entry_count = *reader.Read<uint32_t>();

    if (entry_count > 0) {
        ext_entries.resize(entry_count);
        for (auto &entry : ext_entries)
            entry = *reader.Read<ExtEntry>();
    }
}

StringTable::StringTable(oead::util::BinaryReader& reader)
{
    header = *reader.Read<BlockHeader>();
    asset_name = reader.ReadString(reader.Tell(), header.section_size);
}
} // namespace NSound::Amta
