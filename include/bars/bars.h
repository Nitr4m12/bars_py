#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <bars/amta.h>
#include <bars/fstp.h>
#include <bars/fwav.h>
#include <bars/common.h>
#include <oead/util/binary_reader.h>

/*
    sources:
    https://wiki.oatmealdome.me/Aal/BARS_(File_Format)
    https://github.com/Kinnay/Nintendo-File-Formats/wiki/AMTA-File-Format
    https://github.com/aboood40091/BCFSTM-BCFWAV-Converter
    https://github.com/NanobotZ/bfstp-fixer
*/

/*

    *.bars
    |
    |
    +------ FileHeader
    |      |
    |       +-- char signature[4] // "BARS" in ASCII
    |       +-- uint32_t file_size
    |       +-- uint16_t byte order_mark // FEFF for Big-Endian; FFFE for Little-Endian
    |       +-- uint16_t version
    |       +-- uint32_t asset_count
    |       +-- uint32_t crc32hashes[asset_count]
    |       +-- FileEntries // one for each file
    |           |
    |           +---- uint32_t amta_offset
    |           +---- uint32_t asset_offset
    +------ AmtaArray
    |
    +------ Prefetch and Wave Array



*/
#ifndef NSOUND_BARS_H
#define NSOUND_BARS_H

namespace NSound {
namespace Bars {
struct Header {
    std::array<uint8_t, 4> signature;

    uint32_t file_size;
    uint16_t bom;
    uint16_t version;
    uint32_t asset_count;

    std::vector<uint32_t> crc32hashes;  // size = this.asset_count; must be sorted

    struct FileEntry {
        uint32_t amta_offset;
        uint32_t asset_offset;
    };
    std::vector<FileEntry> file_entries;  // size = this.asset_count; same order as the CRC32 hashes
};

struct BarsFile {
    Header header;
    std::vector<Amta::AmtaFile> amta_array;
    std::vector<std::variant<Fstp::PrefetchFile, Fwav::WaveFile>> audio_files;
};

Header load_header(oead::util::BinaryReader& reader);

}   // namespace NSound::Bars

class Parser {
public:
    Parser(std::string file_name);
    void load();
private:
    oead::util::BinaryReader reader;
    std::vector<uint8_t> buffer;
};

}   // namespace NSound

#endif