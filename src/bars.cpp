#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cassert>

#include <bars/bars.h>

namespace NSound::Bars {
Header::Header(oead::util::BinaryReader& reader) 
{
    signature = *reader.Read<typeof(signature)>();
    file_size = *reader.Read<uint32_t>();
    bom = *reader.Read<uint16_t>();
    version = *reader.Read<uint16_t>();
    asset_count = *reader.Read<uint32_t>();

    crc32hashes.resize(asset_count);
    file_entries.resize(asset_count);

    for (auto &hash : crc32hashes)
        hash = *reader.Read<uint32_t>();

    for (auto &entry : file_entries) {
        entry.amta_offset = *reader.Read<uint32_t>();
        entry.asset_offset = *reader.Read<uint32_t>();
    }
}

BarsFile::BarsFile(oead::util::BinaryReader& reader)
    : header{reader}
{
    std::array<uint8_t, 4> sign {'B', 'A', 'R', 'S'};

    // Checks
    if (header.signature != sign) throw std::runtime_error("Invalid header");
    if (header.bom != VALID_BOM) throw std::runtime_error("Invalid Byte-Order Mark");

    files.resize(header.asset_count);
    for (int i {0}; i<header.asset_count; ++i) {
        reader.Seek(header.file_entries[i].amta_offset);
        files[i].metadata = {reader};

        reader.Seek(header.file_entries[i].asset_offset);
        if (std::strcmp((char*)&reader.span()[reader.Tell()], "FSTP") == 0)
            files[i].audio = Fstp::PrefetchFile{reader};
        else if (std::strcmp((char*)&reader.span()[reader.Tell()], "FWAV") == 0)
            files[i].audio = Fwav::WaveFile{reader};
    }
}

} // namespace Bars
