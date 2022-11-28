#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#include <bars/bars.h>
#include <bars/amta.h>
#include <oead/util/binary_reader.h>

int main(int argc, char **argv)
{
    // 1. Parse file
    std::ifstream ifs {argv[1]};
    std::vector<uint8_t> buffer(std::filesystem::file_size(argv[1]));
    {
        ifs.read((char*)buffer.data(), buffer.size());
    }

    oead::util::BinaryReader reader{buffer, oead::util::Endianness::Little};

    NSound::Bars::BarsFile bars{reader};

    // 2. Save Amta
    for (auto entry : bars.file_entries()) {
        std::ofstream ofs {entry.metadata.strg_table.asset_name + ".amta"};
        NSound::Amta::save_file(ofs, entry.metadata);
    }

}
