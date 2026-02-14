#include <stdexcept>

#include "bars/bars.h"

namespace NSound::Bars {
ResourceHeader::ResourceHeader(AudioReader& reader) 
{
    signature = reader.read<typeof(signature)>();
    file_size = reader.read<uint32_t>();
    bom = reader.read<uint16_t>();
    if (bom == 0xFFFE) {
        reader.swap_endianness();
        reader.seek(0);
        *this = {reader};
        return;
    }
    version = reader.read<uint16_t>();
    asset_count = reader.read<uint32_t>();

    crc32hashes.resize(asset_count);
    file_entries.resize(asset_count);

    for (auto& hash : crc32hashes)
        hash = reader.read<uint32_t>();

    for (auto& entry : file_entries) {
        entry.amta_offset = reader.read<uint32_t>();
        entry.asset_offset = reader.read<uint32_t>();
    }
}

BarsFile::BarsFile(AudioReader& reader) 
{
    m_header = {reader};
    endianness = reader.endianness();

    {
        // Check
        ResourceHeader ref;

        if (m_header.signature != ref.signature)
            throw std::runtime_error("Invalid header!");
        if (m_header.bom != VALID_BOM)
            throw std::runtime_error("Invalid Byte-Order Mark");
    }

    m_files.resize(m_header.asset_count);
    for (int i{0}; i < m_header.asset_count; ++i) {
        reader.seek(m_header.file_entries[i].amta_offset);
        m_files[i].metadata = Amta::AmtaFile{reader};

        reader.seek(m_header.file_entries[i].asset_offset);

        std::string sign{reader.read<char>()};
        sign.push_back(reader.read<char>());
        sign.push_back(reader.read<char>());
        sign.push_back(reader.read<char>());

        reader.seek(m_header.file_entries[i].asset_offset);
        if (sign == "FSTP")
            m_files[i].audio = Fstp::PrefetchFile{reader};
        else if (sign == "FWAV")
            m_files[i].audio = Fwav::WaveFile{reader};
        else
            throw std::runtime_error("Invalid asset header");
    }
}

void BarsFile::swap_endianness() {
    if (endianness == binaryio::endian::little)
        endianness = binaryio::endian::big;
    else
        endianness = binaryio::endian::little;

    for (auto& file : m_files) {
        file.metadata.endianness = endianness;
        switch (file.metadata.data.type) {
        case Amta::Data::Type::Wave: {
            Fwav::WaveFile& fwav = std::get<Fwav::WaveFile, Fstp::PrefetchFile>(file.audio);
            fwav.endianness = endianness;
            break;
        }
        case Amta::Data::Type::Stream: {
            Fstp::PrefetchFile& fstp = std::get<Fstp::PrefetchFile>(file.audio);
            fstp.endianness = endianness;
            break;
        }
        default:
            break;
        }
    }
}

void BarsFile::serialize(AudioWriter& writer) 
{
    writer.set_endianness(endianness);

    writer.write(m_header.signature);
    writer.write(m_header.file_size);
    writer.write(m_header.bom);
    writer.write(m_header.version);
    writer.write(m_header.asset_count);

    for (auto& crc32_hash : m_header.crc32hashes)
        writer.write(crc32_hash);

    for (auto& file_entry : m_header.file_entries)
        writer.write(file_entry);

    for (int i{0}; i < m_files.size(); ++i) {
        writer.seek(m_header.file_entries[i].amta_offset);
        m_files[i].metadata.serialize(writer);
    }

    for (int i{0}; i < m_files.size(); ++i) {
        writer.seek(m_header.file_entries[i].asset_offset);
        switch (m_files[i].metadata.data.type) {
        case Amta::Data::Type::Wave: {
            Fwav::WaveFile fwav = std::get<Fwav::WaveFile>(m_files[i].audio);
            fwav.serialize(writer);
            break;
        }
        case Amta::Data::Type::Stream: {
            Fstp::PrefetchFile fstp = std::get<Fstp::PrefetchFile>(m_files[i].audio);
            fstp.serialize(writer);
            break;
        }
        default:
            throw std::runtime_error("Invalid file type!");
        }
    }
}

} // namespace NSound::Bars
