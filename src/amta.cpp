#include <bars/amta.h>
#include <bars/common.h>
#include <oead/util/binary_reader.h>
#include <cassert>

namespace NSound::Amta {
Marker::Marker(oead::util::BinaryReader& reader)
{
    header = *reader.Read<BlockHeader>();
    assert(header.signature == signatures["MARK"]);

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
    assert(header.signature == signatures["EXT_"]);

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
    assert(header.signature == signatures["STRG"]);

    asset_name = reader.ReadString(reader.Tell(), header.section_size);
}

AmtaFile::AmtaFile(oead::util::BinaryReader& reader)
{
    size_t amta_start = reader.Tell();
    header = *reader.Read<Header>();
    assert(header.signature == signatures["AMTA"]);
    
    data = *reader.Read<Data>(amta_start + header.data_offset);
    
    reader.Seek(amta_start + header.marker_offset);
    marker = Marker{reader};

    reader.Seek(amta_start + header.ext__offset);
    ext_ = Ext_{reader};

    reader.Seek(amta_start + header.string_table_offset);
    strg_table = StringTable{reader};
}

void save_file(std::ostream& os, AmtaFile& amta)
{
    os.write(reinterpret_cast<char*>(&amta.header), sizeof(Header));
    os.seekp(amta.header.data_offset);
    os.write(reinterpret_cast<char*>(&amta.data), sizeof(Data));
    os.seekp(amta.header.marker_offset);
    os.write(reinterpret_cast<char*>(&amta.marker.header), sizeof(BlockHeader));
    os.write(reinterpret_cast<char*>(&amta.marker.entry_count), sizeof(uint32_t));

    for (auto &marker_info : amta.marker.marker_infos)
        os.write(reinterpret_cast<char*>(&marker_info), sizeof(Marker::MarkerInfo));

    os.seekp(amta.header.ext__offset);
    os.write(reinterpret_cast<char*>(&amta.ext_.header), sizeof(BlockHeader));
    os.write(reinterpret_cast<char*>(&amta.ext_.entry_count), sizeof(uint32_t));

    for (auto &ext_entries : amta.ext_.ext_entries)
        os.write(reinterpret_cast<char*>(&ext_entries), sizeof(Ext_::ExtEntry));

    os.seekp(amta.header.string_table_offset);
    os.write(reinterpret_cast<char*>(&amta.strg_table.header), sizeof(BlockHeader));
    os.write(amta.strg_table.asset_name.c_str(), amta.strg_table.header.section_size);
}
} // namespace NSound::Amta
