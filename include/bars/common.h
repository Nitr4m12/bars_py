#include <array>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <vector>

#include <binaryio/reader.h>
#include <oead/util/binary_reader.h>


#ifndef NSOUND_COMMON_H
#define NSOUND_COMMON_H

namespace NSound {
class AudioReader;
constexpr int VALID_BOM = 0xFEFF;
struct BlockHeader {
    std::array<uint8_t, 4> signature;
    uint32_t section_size;
};

struct Reference {
    uint16_t type;
    int32_t offset;
};

struct SizedReference {
    uint16_t type;
    int32_t offset;
    uint32_t size;
};

template <typename T>
struct Table {
    uint32_t count;
    std::vector<T> items;
};

struct ReferenceTable : Table<Reference> {};
struct SizedReferenceTable : Table<Reference> {};

struct AudioHeader {
    std::array<uint8_t, 4> signature;
    uint16_t bom;
    uint16_t head_size;
    uint32_t version;
    uint32_t file_size;
    uint16_t block_count;
    uint16_t reserved;
    std::vector<SizedReference> block_refs;

    AudioHeader() = default;
    AudioHeader(AudioReader& reader);
};

class AudioReader : public binaryio::BinaryReader {
public:

    AudioReader(void* const begin, void* const end)
        : binaryio::BinaryReader(begin, end) {}

    template <typename T>
    Table<T> read_table() {
        Table<T> tbl;
        tbl.count = read<uint32_t>();
        tbl.items.resize(tbl.count);
        for (auto& item : tbl.items)
            item = read<T>();

        return tbl;
    }

    template <typename T>
    T read_reference(std::optional<size_t> offset = std::nullopt) {
        // Read a struct referenced by a Reference. If an offset
        // is provided, use that as a starting point from where
        // to seek

        Reference ref;
        size_t start_offset;
        size_t return_offset = tell();
        if (offset)
            start_offset = *offset;
        else
            start_offset = return_offset;

        ref = read<Reference>();
        seek(start_offset + ref.offset);

        T referenced = read<T>();

        seek(return_offset + sizeof(Reference));

        return referenced;
    }

    ReferenceTable read_ref_table() {
        ReferenceTable ref_tbl;
        ref_tbl.count = read<uint32_t>();
        ref_tbl.items.resize(ref_tbl.count);
        for (auto& item : ref_tbl.items)
            item = read<Reference>();

        return ref_tbl;
    }
};

class AudioWriter {
public:
    AudioWriter() = default;
    AudioWriter(oead::util::Endianness endian) : writer{endian} {}

    template <typename T>
    void write(T data) {
        writer.Write<T>(data);
    }

    template <>
    void write<AudioHeader>(AudioHeader data);

    void write_cstr(std::string_view str) { writer.WriteCStr(str); }

    template <typename T>
    void write_table(Table<T> data) {
        write<uint32_t>(data.count);
        for (auto& item : data.items)
            write<T>(item);
    }

    void align_up(size_t n) { writer.AlignUp(n); }

    void seek(size_t offset) { writer.Seek(offset); }

    size_t tell() const { return writer.Tell(); }

    oead::util::Endianness endian() const { return writer.Endian(); };

    std::vector<uint8_t> finalize() { return writer.Finalize(); }

private:
    oead::util::BinaryWriter writer{oead::util::Endianness::Little};
};

void write_reference(oead::util::BinaryWriter& writer, Reference& ref);
void write_sized_reference(oead::util::BinaryWriter& writer,
                           SizedReference& sref);
void write_audio_header(oead::util::BinaryWriter& writer, AudioHeader& header);

} // namespace NSound

#endif
