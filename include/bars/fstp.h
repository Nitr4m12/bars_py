#include <array>
#include <cstdint>

#include <bars/common.h>
#include "bars/fstm.h"
#include "oead/util/binary_reader.h"

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
    BlockHeader header {'P', 'D', 'A', 'T', 0xc040};
    Table<PrefetchData> prefetch_data;
    std::vector<uint8_t> sample_data;

    PrefetchDataBlock() = default;
    PrefetchDataBlock(AudioReader& reader);
};

struct PrefetchFile {
    AudioHeader       header;
    Fstm::InfoBlock   info;
    PrefetchDataBlock pdat;

    PrefetchFile() = default;
    PrefetchFile(AudioReader& reader);
};

} // namespace NSound::Fstp

#endif