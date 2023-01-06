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
    {
        ifs.read((char*)buffer.data(), buffer.size());
    }

    NSound::Bars::BarsFile bars{buffer};

    // 2. Write file
    buffer.clear();
    buffer = bars.serialize();
    std::ofstream ofs {"test.bars"};
    ofs.write((char*)buffer.data(), buffer.size());

    // 3. Get file
    NSound::Bars::BarsFile::FileWithMetadata file {bars.get_file(0)};
    std::cout << "File " << file.metadata.strg.asset_name << " was found!" << '\n';


}
