#include <iostream>
#include <fstream>

#include <bars/bars.h>
#include <bars/amta.h>
#include <bars/fstp.h>
#include <bars/fwav.h>
#include <bars/common.h>

int main(int argc, char **argv)
{
    // 1. Parse file
    std::ifstream ifs {argv[1]};
    NSound::AudioReader reader {ifs};
    NSound::Bars::BarsFile bars{reader};
    ifs.close();

    // 2. Write file
    std::ofstream ofs {"test.bars"};
    NSound::AudioWriter writer {ofs};
    bars.serialize(writer);
    ofs.close();

    // 3. Write file with swapped endian
    ofs.open("test_swap.bars");
    NSound::AudioWriter writer2 {ofs};
    bars.swap_endianness();
    bars.serialize(writer2);
    ofs.close();


    // 4. Parse the new file
    // buffer.clear();
    // buffer.resize(std::filesystem::file_size(argv[1]));
    // ifs.open("test_be.bars");
    // ifs.read((char*)buffer.data(), buffer.size());
    // ifs.close();
    // NSound::Bars::BarsFile bars2{buffer};

    // // 5. Write the new file as little endian
    // buffer.clear();
    // bars.swap_endianness();
    // buffer = bars.serialize();
    // ofs.open("test_be_swapped.bars");
    // ofs.write((char*)buffer.data(), buffer.size());
    // ofs.close();

    // 5. Get sub-file
    std::cout << "File " << bars.get_file(0).metadata.strg.asset_name << " was found!" << '\n';


}
