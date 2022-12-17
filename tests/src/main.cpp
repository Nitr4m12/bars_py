#include <cstdint>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <variant>

#include <bars/bars.h>
#include <bars/amta.h>
#include <bars/fstp.h>
#include <oead/util/binary_reader.h>
#include "oead/util/swap.h"

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
    oead::util::BinaryWriter writer{oead::util::Endianness::Little};
    for (auto entry : bars.file_entries()) {
        std::ofstream ofs {entry.metadata.strg_table.asset_name + ".amta"};
        std::vector<uint8_t> data = NSound::Amta::write(writer, entry.metadata);

        for (uint8_t& byte : data)
            ofs.write(reinterpret_cast<char*>(&byte), sizeof(uint8_t));
    }

    // 3. Save Fstp
    for (auto entry : bars.file_entries()) {
        try {
            NSound::Fstp::PrefetchFile fstp_file = std::get<NSound::Fstp::PrefetchFile>(entry.audio);
            std::ofstream ofs {entry.metadata.strg_table.asset_name + ".fstp"};
            NSound::Fstp::write(ofs, fstp_file);
        }
        catch (std::bad_variant_access&) {
            std::cerr << "Not an FSTP file!" << '\n';
        }

    }

}
