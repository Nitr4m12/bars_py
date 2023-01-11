#include <array>
#include <cstdint>

#include "bars/common.h"
#include "bars/fstm.h"

#ifndef NSOUND_FSTP_H
#define NSOUND_FSTP_H

namespace NSound::Fstp {
struct PrefetchData {
    uint32_t start_frame;
    uint32_t prefetch_size;
    uint32_t reserved;

    // offset is relative to the start of the PrefetchData
    Reference to_prefetch_samples;
};

struct PrefetchDataBlock {
    BlockHeader header{'P', 'D', 'A', 'T', 0xc040};
    Table<PrefetchData> prefetch_data;
    std::vector<uint8_t> sample_data;

    PrefetchDataBlock() = default;
    PrefetchDataBlock(AudioReader& reader);
};

struct PrefetchFile {
    AudioHeader header;
    Fstm::InfoBlock info;
    PrefetchDataBlock pdat;

    binaryio::endian endianness;

    PrefetchFile() = default;
    PrefetchFile(std::vector<uint8_t>::iterator begin,
                 std::vector<uint8_t>::iterator end);

    std::vector<uint8_t> serialize();
};

} // namespace NSound::Fstp

#endif