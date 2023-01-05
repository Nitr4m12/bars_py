#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cassert>

#include <bars/bars.h>

namespace NSound::Bars {
ResourceHeader::ResourceHeader(AudioReader& reader)
{
    signature = reader.read<typeof(signature)>();
    file_size = reader.read<uint32_t>();
    bom = reader.read<uint16_t>();
    version = reader.read<uint16_t>();
    asset_count = reader.read<uint32_t>();

    crc32hashes.resize(asset_count);
    file_entries.resize(asset_count);

    for (auto &hash : crc32hashes)
        hash = reader.read<uint32_t>();

    for (auto &entry : file_entries) {
        entry.amta_offset = reader.read<uint32_t>();
        entry.asset_offset = reader.read<uint32_t>();
    }
}

BarsFile::BarsFile(AudioReader& reader)
    :mHeader{reader}
{

    {
        // Check
        ResourceHeader def;

        if (mHeader.signature != def.signature) throw std::runtime_error("Invalid header!");
        if (mHeader.bom != VALID_BOM) throw std::runtime_error("Invalid Byte-Order Mark");
    }

    mFiles.resize(mHeader.asset_count);
    for (int i {0}; i<mHeader.asset_count; ++i) {
        reader.seek(mHeader.file_entries[i].amta_offset);
        mFiles[i].metadata = {reader};

        reader.seek(mHeader.file_entries[i].asset_offset);
        std::string sign {reader.read_string(4)};

        reader.seek(mHeader.file_entries[i].asset_offset);
        if (sign == "FSTP")
            mFiles[i].audio = Fstp::PrefetchFile{reader};
        else if (sign == "FWAV")
            mFiles[i].audio = Fwav::WaveFile{reader};
        else
            throw std::runtime_error("Invalid asset header");
    }
}

std::vector<uint8_t> BarsFile::serialize()
{
    AudioWriter writer;

    writer.write<typeof(mHeader.signature)>(mHeader.signature);
    writer.write<uint32_t>(mHeader.file_size);
    writer.write<uint16_t>(mHeader.bom);
    writer.write<uint16_t>(mHeader.version);
    writer.write<uint32_t>(mHeader.asset_count);

    for (auto& crc32_hash : mHeader.crc32hashes)
        writer.write<uint32_t>(crc32_hash);

    for (auto& file_entry : mHeader.file_entries)
        writer.write<ResourceHeader::FileEntry>(file_entry);

    for (int i {0}; i<mFiles.size(); ++i) {
        std::vector<uint8_t> amta_bytes = mFiles[i].metadata.serialize();
        std::vector<uint8_t> asset_bytes;
        switch (mFiles[i].metadata.data.type) {
        case Amta::Data::Type::Wave:
        {
            Fwav::WaveFile fwav = std::get<Fwav::WaveFile>(mFiles[i].audio);
            asset_bytes = fwav.serialize();
            break;
        }
        case Amta::Data::Type::Stream:
        {
            Fstp::PrefetchFile fstp = std::get<Fstp::PrefetchFile>(mFiles[i].audio);
            asset_bytes = fstp.serialize();
            break;
        }
        default:
            throw std::runtime_error("Invalid file type!");
        }

        writer.seek(mHeader.file_entries[i].amta_offset);
        for (uint8_t byte : amta_bytes)
            writer.write<uint8_t>(byte);

        writer.seek(mHeader.file_entries[i].asset_offset);
        for (uint8_t byte : asset_bytes)
            writer.write<uint8_t>(byte);
    }

    return writer.finalize();

}

} // namespace Bars
