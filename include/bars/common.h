#include <array>
#include <cstdint>
#include <vector>

#ifndef NSOUND_COMMON_H
#define NSOUND_COMMON_H

namespace NSound {
struct BlockHeader {
  std::array<uint8_t, 4> signature;
  uint32_t section_size;
};

struct Reference {
  uint16_t type;
  uint8_t padding[2];
  int32_t offset;
};

struct SizedReference : Reference {
  uint32_t size;
};

struct ReferenceTable {
  uint32_t count;
  std::vector<Reference> refs;
};
}

#endif