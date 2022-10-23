#include <array>
#include <cstdint>
#include <map>
#include <vector>

#include <oead/util/binary_reader.h>

#ifndef NSOUND_COMMON_H
#define NSOUND_COMMON_H

namespace NSound {
extern std::map<std::string, std::array<uint8_t, 4>> signatures;
extern std::map<std::string, int> reference_types;
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

    AudioHeader();
    AudioHeader(oead::util::BinaryReader& reader);
};

} // namespace NSound

#endif