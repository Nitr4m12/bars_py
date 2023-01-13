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
#include <bars/common.h>

int main(int argc, char **argv)
{
    // 1. Parse file
    std::ifstream ifs {argv[1]};
    std::vector<uint8_t> buffer(std::filesystem::file_size(argv[1]));
    ifs.read((char*)buffer.data(), buffer.size());
    ifs.close();

    NSound::Bars::BarsFile bars{buffer};

    // 2. Write file
    buffer.clear();
    buffer = bars.serialize();
    std::ofstream ofs {"test.bars"};
    ofs.write((char*)buffer.data(), buffer.size());
    ofs.close();

    // 3. Write file with swapped endian
    buffer.clear();
    bars.swap_endianness();
    buffer = bars.serialize();
    ofs.open("test_be.bars");
    ofs.write((char*)buffer.data(), buffer.size());
    ofs.close();


    // 4. Parse the new file
    buffer.clear();
    buffer.resize(std::filesystem::file_size(argv[1]));
    ifs.open("test_be.bars");
    ifs.read((char*)buffer.data(), buffer.size());
    ifs.close();
    NSound::Bars::BarsFile bars2{buffer};

    // // 5. Write the new file as little endian
    buffer.clear();
    bars.swap_endianness();
    buffer = bars.serialize();
    ofs.open("test_be_swapped.bars");
    ofs.write((char*)buffer.data(), buffer.size());
    ofs.close();

    // 5. Get sub-file
    NSound::Bars::BarsFile::FileWithMetadata file {bars.get_file(0)};
    std::cout << "File " << file.metadata.strg.asset_name << " was found!" << '\n';


}
