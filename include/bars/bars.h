/*
    sources:
    https://wiki.oatmealdome.me/Aal/BARS_(File_Format)
    https://github.com/Kinnay/Nintendo-File-Formats/wiki/AMTA-File-Format
    https://github.com/aboood40091/BCFSTM-BCFWAV-Converter
    https://github.com/NanobotZ/bfstp-fixer
*/

#ifndef NSOUND_BARS_H
#define NSOUND_BARS_H

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <bars/amta.h>
#include <bars/common.h>
#include <bars/fstp.h>
#include <bars/fwav.h>
#include <oead/util/binary_reader.h>

namespace NSound::Bars {
struct Header {
    std::array<uint8_t, 4> signature {'B', 'A', 'R', 'S'};

    uint32_t file_size {0};
    uint16_t bom {0xFEFF};
    uint16_t version {0x101};
    uint32_t asset_count {0};

    // size = this.asset_count; must be sorted
    std::vector<uint32_t> crc32hashes;

    struct FileEntry {
        uint32_t amta_offset;
        uint32_t asset_offset;
    };
    // size = this.asset_count; same order as the CRC32 hashes
    std::vector<FileEntry> file_entries;

    Header() = default;
    Header(oead::util::BinaryReader& reader);
};

class BarsFile {
public:
    struct FileWithMetadata {
        Amta::AmtaFile  metadata;
        std::variant<Fstp::PrefetchFile, Fwav::WaveFile> audio;
    };

    std::vector<FileWithMetadata> file_entries() { return files; }

    BarsFile();
    BarsFile(oead::util::BinaryReader& reader);
private:
    Header header;
    std::vector<FileWithMetadata> files;
};

}   // namespace NSound::Bars

#endif // NSOUND_BARS_H
