#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <span>

#include <oead/util/binary_reader.h>

#ifndef NSOUND_COMMON_H
#define NSOUND_COMMON_H

namespace NSound {
constexpr int VALID_BOM = 0xFEFF;
extern std::map<std::string, std::array<uint8_t, 4>> signatures;
struct BlockHeader {
    std::array<uint8_t, 4> signature;
    uint32_t section_size;
};

struct Reference {
    uint16_t type;
    uint8_t padding[2];
    int32_t offset;
};

template<typename T>
struct Table {
    uint32_t count;
    std::vector<T> items;
};

struct SizedReference {
    uint16_t type;
    uint8_t padding[2];
    int32_t offset;
    uint32_t size;
};

struct AudioHeader {
    std::array<uint8_t, 4> signature;
    uint16_t bom;
    uint16_t head_size;
    uint32_t version;
    uint32_t file_size;
    uint16_t block_count;
    uint16_t reserved;
    // Size = this.block_count
    std::vector<SizedReference> block_refs;

    AudioHeader() = default;
    AudioHeader(oead::util::BinaryReader& reader);
};

class AudioReader {
public:

    AudioReader(std::span<uint8_t> data, oead::util::Endianness endian)
        :reader{data, endian} {}

    template <typename T>
    T read() { return *reader.Read<T>(); }

    template <typename T>
    T read_at(size_t offset) { return *reader.Read<T>(offset); }

    template<typename T>
    Table<T> read_table()
    {
        Table<T> tbl;
        tbl.count = *reader.Read<uint32_t>();
        tbl.items.resize(tbl.count);
        for (auto& item : tbl.items)
            item = *reader.Read<T>();

        return tbl;
    }

    void seek(size_t offset) { reader.Seek(offset); }

private:
    oead::util::BinaryReader reader;
};

template<typename T>
Table<T> read_table(oead::util::BinaryReader& reader)
{
    Table<T> tbl;
    tbl.count = *reader.Read<uint32_t>();
    tbl.items.resize(tbl.count);
    for (auto& item : tbl.items)
        item = *reader.Read<T>();

    return tbl;
}

void write_reference(oead::util::BinaryWriter& writer, Reference& ref);
void write_sized_reference(oead::util::BinaryWriter& writer, SizedReference& sref);
void write_audio_header(oead::util::BinaryWriter& writer, AudioHeader& header);

} // namespace NSound

#endif
