#include <cassert>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>


#include "bars/fstp.h"
#include "oead/util/binary_reader.h"
#include "oead/util/swap.h"

namespace NSound::Fstp {
PrefetchDataBlock::PrefetchDataBlock(AudioReader& reader)
{
    header = reader.read<BlockHeader>();

    size_t table_start {reader.tell()};
    prefetch_data = reader.read_table<PrefetchData>();

    if (prefetch_data.count != 1)
        // Do any stock ones even exists?
        throw std::runtime_error("Can't read prefetch files with more than one PrefetchData!");

    PrefetchData current_item {prefetch_data.items[0]};

    size_t offset {table_start + sizeof(uint32_t) + current_item.to_prefetch_samples.offset};
    reader.seek(offset);

    sample_data.resize(current_item.prefetch_size);

    for (int i {0}; i<current_item.prefetch_size; ++i)
        sample_data[i] = reader.read<uint8_t>();
}

PrefetchFile::PrefetchFile(AudioReader& reader)
{
    size_t file_start = reader.tell();
    header = {reader};

    for (auto &ref : header.block_refs) {
        reader.seek(file_start + ref.offset);
        if (ref.type == 0x4000)
            info = {reader};
        else if (ref.type == 0x4004)
            pdat = {reader};
    }
}

std::vector<uint8_t> write(PrefetchFile& fstp)
{
    oead::util::BinaryWriter writer{oead::util::Endianness::Little};
    writer.AlignUp(0x20);
    return writer.Finalize();
}

}