#include <array>
#include <bars/amta.h>
#include <bars/common.h>
#include <cstddef>
#include <cstdint>
#include <oead/util/binary_reader.h>
#include <cassert>
#include "oead/util/swap.h"

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

    reader.Seek(amta_start + header.ext_offset);
    ext = Ext_{reader};

    reader.Seek(amta_start + header.string_table_offset);
    strg_table = StringTable{reader};
}

void write_strg_section(oead::util::BinaryWriter& writer, StringTable& strg)
{
    writer.Write("STRG");
    writer.Write<uint32_t>(strg.asset_name.size() + 1);
    writer.WriteCStr(strg.asset_name);
}

void write_ext_section(oead::util::BinaryWriter& writer, Ext_& ext)
{
    writer.Write("EXT_");

    size_t ext_size {0};
    if (ext.entry_count > 0)
        ext_size = sizeof(BlockHeader) + sizeof(Ext_::entry_count) + (sizeof(Ext_::ExtEntry) * ext.ext_entries.size());

    writer.Write(ext_size);

    for (auto& ext_entry : ext.ext_entries)
        writer.Write<Ext_::ExtEntry>(ext_entry);
}

void write_marker_section(oead::util::BinaryWriter& writer, Marker& mark)
{
    writer.Write("MARK");

    size_t mark_size {0};
    if (mark.entry_count > 0)
        mark_size = sizeof(BlockHeader) + sizeof(Marker::entry_count) + (sizeof(Marker::MarkerInfo) * mark.marker_infos.size());

    writer.Write<uint32_t>(mark_size);

    for (auto& marker_info : mark.marker_infos)
        writer.Write<Marker::MarkerInfo>(marker_info);
}

void write_data_section(oead::util::BinaryWriter& writer, Data& data, const int& version)
{
    writer.Write("DATA");
    writer.Write(sizeof(Data));
    writer.Write<uint32_t>(data.asset_name_offset);
    writer.Write<uint32_t>(data.sample_count);
    writer.Write<Data::Type>(data.type);
    writer.Write<uint8_t>(data.wave_channels);
    writer.Write<uint8_t>(data.used_stream_tracks);
    writer.Write<uint8_t>(data.flags);
    writer.Write<float>(data.volume);
    writer.Write<uint32_t>(data.sample_rate);
    writer.Write<uint32_t>(data.loop_start_sample);
    writer.Write<uint32_t>(data.loop_end_sample);
    writer.Write<float>(data.loudness);

    for (auto& stream_track : data.stream_tracks)
        writer.Write<Data::StreamTrack>(stream_track);

    if (version >= 4)
        writer.Write<float>(data.amplitude_peak);
}

std::vector<uint8_t> write(AmtaFile& amta)
{
    oead::util::BinaryWriter writer{oead::util::Endianness::Little};
    size_t file_start = writer.Tell();

    writer.Write("AMTA");
    writer.Write<uint16_t>(0xFEFF);
    writer.Write<uint16_t>(amta.header.version);
    // file_size and offsets are set once we have written all
    writer.Seek(file_start + sizeof(Header));

    write_data_section(writer, amta.data, amta.header.version);
    write_marker_section(writer, amta.marker);
    write_ext_section(writer, amta.ext);
    write_strg_section(writer, amta.strg_table);

    writer.AlignUp(0x20);
    return writer.Finalize();
}
} // namespace NSound::Amta
