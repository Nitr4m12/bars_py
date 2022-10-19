#include <fstream>
#include <filesystem>
#include <stdexcept>

#include <bars/bars.h>
#include <oead/util/binary_reader.h>

void extract_amta_files(const std::string& file_name)
{
    std::vector<uint8_t> buffer(std::filesystem::file_size(file_name));
    {
        std::ifstream ifs {file_name, std::ios_base::binary};
        char byte;
        for (int i {0}; i < buffer.size(); ++i) {
            ifs.read(&byte, sizeof(uint8_t));
            buffer[i] = byte;
        }
    }


    oead::util::BinaryReader reader {buffer, oead::util::Endianness::Little};
    auto header {reader.Read<BARS::Header>(0)};
    std::cout << "Loading..." << '\n';
    if (header->signature != std::array<uint8_t, 4>{'B', 'A', 'R', 'S'}) throw std::runtime_error("Invalid header");
    reader.Seek(header->file_count * 4);

    std::vector<uint32_t> offsets(header->file_count);
    for (int i {0}; i<offsets.size(); ++i)
        offsets[i] = *reader.Read<uint32_t>();


    for (int i {0}; i<header->file_count; ++i) {
        auto amta {reader.Read<BARS::AmtaHeader>(offsets[i*2])};

        for (int j {0}; j<4; ++j) {
            auto sub {reader.Read<BARS::BlkHeader>()};

            if (sub->signature[0] != 'S')
                reader.Seek(reader.Tell() + sub->section_size);
            else {
                size_t size {sub->section_size};
                auto name = reader.ReadString(reader.Tell(), sub->section_size);
                std::ofstream test {name + ".amta"};
                uint8_t data;
                reader.Seek(offsets[i*2]);
                for (int i {0}; i<amta->size; ++i){
                    auto data = reader.Read<char>();
                    test.write((char*) &data, sizeof(*data));
                }
            }
        }

    }
}