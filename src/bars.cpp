#include <filesystem>
#include "bars/bars.h"
#include <iostream>
#include <fstream>

void extract_amta_files(const std::string& file_name)
{
    BARS::Header header;
    std::ifstream ifs {file_name};
    header.read(ifs);
    ifs.seekg(header.file_count * 4, std::ios_base::cur);
    BARS::TRKStruct track_struct;
    track_struct.read(ifs, header.file_count);

    for (int i {0}; i<header.file_count; ++i) {
        ifs.seekg(track_struct.offsets[i*2]);

        BARS::AmtaHeader amta;
        amta.read(ifs);

        for (int j {0}; j<4; ++j) {
            BARS::BlkHeader sub;
            sub.read(ifs);

            if (sub.magic[0] != 'S')
                ifs.seekg(sub.section_size, std::ios_base::cur);
            else {
                std::string name;
                size_t size {sub.section_size};
                name.resize(size);
                ifs.read(&name[0], size);
                name.pop_back();
                std::ofstream test {name + ".amta"};
                uint8_t data;
                ifs.seekg(track_struct.offsets[i*2]);
                for (int i {0}; i<amta.size; ++i){
                    ifs.read((char*)& data, sizeof(data));
                    test.write((char*)& data, sizeof(data));
                }
            }
        }

    }
}