#include <cstdint>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <variant>

#include <bars/bars.h>
#include <bars/amta.h>
#include <bars/fstp.h>
#include <bars/fwav.h>
#include "bars/common.h"
#include "oead/util/swap.h"

int main(int argc, char **argv)
{
    // TODO: Add a function to read references, and return the struct
    // TODO: Fix FSTP saving. Probably with the above.

    // 1. Parse file
    std::ifstream ifs {argv[1]};
    std::vector<uint8_t> buffer(std::filesystem::file_size(argv[1]));
    {
        ifs.read((char*)buffer.data(), buffer.size());
    }

    NSound::AudioReader reader{buffer, oead::util::Endianness::Little};

    NSound::Bars::BarsFile bars{reader};
}
