#include <fstream>
#include <filesystem>
#include <stdexcept>

#include <bars/bars.h>

NSound::Header get_main_header(const std::string& file_name)
{
    std::vector<uint8_t> buffer(std::filesystem::file_size(file_name));
    {
        std::ifstream ifs {file_name, std::ios_base::binary};
        ifs.read((char*)buffer.data(), buffer.size());
    }

    oead::util::BinaryReader reader {buffer, oead::util::Endianness::Little};
    auto header {*reader.Read<NSound::Header>(0)};
    if (header.signature != std::array<uint8_t, 4>{'B', 'A', 'R', 'S'}) throw std::runtime_error("Invalid header");
    return header;
}

void extract_amta(const NSound::Header header, oead::util::BinaryReader reader)
{

    reader.Seek(header.asset_count * 4);

    std::vector<uint32_t> offsets(header.asset_count);
    for (int i {0}; i<offsets.size(); ++i)
        offsets[i] = *reader.Read<uint32_t>();


    for (int i {0}; i<header.asset_count; ++i) {
        auto amta {*reader.Read<NSound::AmtaHeader>(offsets[i*2])};

        for (int j {0}; j<4; ++j) {
            auto sub {*reader.Read<NSound::BlockHeader>()};

            if (sub.signature[0] != 'S')
                reader.Seek(reader.Tell() + sub.section_size);
            else {
                size_t size {sub.section_size};
                auto name = reader.ReadString(reader.Tell(), sub.section_size);
                std::ofstream test {name + ".amta"};
                uint8_t data;
                reader.Seek(offsets[i*2]);
                for (int i {0}; i<amta.size; ++i){
                    auto data = reader.Read<char>();
                    test.write((char*) &data, sizeof(*data));
                }
            }
        }

    }
}