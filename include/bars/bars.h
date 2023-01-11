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

#include <oead/util/binary_reader.h>
#include <oead/util/hash.h>

#include "bars/amta.h"
#include "bars/common.h"
#include "bars/fstp.h"
#include "bars/fwav.h"

namespace NSound::Bars {
struct ResourceHeader {
    std::array<uint8_t, 4> signature{'B', 'A', 'R', 'S'};

    uint32_t file_size{0};
    uint16_t bom{0xFEFF};
    uint16_t version{0x101};
    uint32_t asset_count{0};

    // size = this.asset_count; must be sorted
    std::vector<uint32_t> crc32hashes;

    struct FileEntry {
        uint32_t amta_offset;
        uint32_t asset_offset;
    };
    // size = this.asset_count; same order as the CRC32 hashes
    std::vector<FileEntry> file_entries;

    ResourceHeader() = default;
    ResourceHeader(AudioReader& reader);
};

class BarsFile {
public:
    binaryio::endian endianness;

    BarsFile(std::vector<uint8_t>& buffer);

    std::vector<uint8_t> serialize();

    struct FileWithMetadata {
        Amta::AmtaFile metadata;
        std::variant<Fstp::PrefetchFile, Fwav::WaveFile> audio;
    };

    std::vector<FileWithMetadata> get_files() { return mFiles; }
    FileWithMetadata get_file(int idx) { return mFiles[idx]; }
    FileWithMetadata get_file(std::string name) {
        uint32_t hash{oead::util::crc32(name)};
        int idx{lookup(mHeader.crc32hashes, hash)};
        if (idx < 0)
            throw std::runtime_error("BarsFile: File not found");
        return mFiles[idx];
    }

private:
    ResourceHeader mHeader;
    std::vector<FileWithMetadata> mFiles;

    static int lookup(std::vector<uint32_t> hashes, int key) {
        int first{0};
        int last{static_cast<int>(hashes.size() - 1)};

        // https://github.com/krenyy/botw_havok/blob/master/botw_havok/cli/hkrb_extract.py#L30
        while (first <= last) {
            int idx = (first + last) / 2;
            if (hashes[idx] == key)
                return idx;
            else if (hashes[idx] < key)
                first = idx + 1;
            else
                last = idx - 1;
        }
        return -1;
    }
};

} // namespace NSound::Bars

#endif // NSOUND_BARS_H
