#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <binaryio/reader.h>
#include <binaryio/writer.h>


#ifndef NSOUND_COMMON_H
#define NSOUND_COMMON_H

namespace NSound {
class AudioReader;
constexpr int VALID_BOM = 0xFEFF;
struct BlockHeader {
    std::array<uint8_t, 4> signature;
    uint32_t section_size;

    BINARYIO_DEFINE_FIELDS(BlockHeader, signature, section_size);
};

struct Reference {
    uint16_t type;
    int32_t offset;

    BINARYIO_DEFINE_FIELDS(Reference, type, offset);
};

struct SizedReference {
    uint16_t type;
    int32_t offset;
    uint32_t size;

    BINARYIO_DEFINE_FIELDS(SizedReference, type, offset, size);
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

class AudioWriter : public binaryio::BinaryWriter {
public:
    AudioWriter(binaryio::endian endianness)
        : binaryio::BinaryWriter{endianness} {}

    void write_audio_header(AudioHeader header);

    template <typename T>
    void write_table(Table<T> data) {
        write<uint32_t>(data.count);
        for (auto& item : data.items)
            write<T>(item);
    }
};
} // namespace NSound

#endif
