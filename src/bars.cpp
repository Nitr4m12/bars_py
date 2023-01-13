#include <stdexcept>

#include <bars/bars.h>

namespace NSound::Bars {
void ResourceHeader::init(AudioReader& reader) {
    signature = reader.read<typeof(signature)>();
    file_size = reader.read<uint32_t>();
    bom = reader.read<uint16_t>();
    if (bom == 0xFFFE) {
        reader.swap_endianness();
        reader.seek(0);
        init(reader);
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

BarsFile::BarsFile(std::vector<uint8_t>& buffer) {
    AudioReader reader {buffer.begin().base(), buffer.end().base()};

    m_header.init(reader);
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
        m_files[i].metadata = {buffer.begin() + reader.tell(), buffer.end()};
        reader.seek(m_header.file_entries[i].asset_offset);
        std::string sign{reader.read_string(4)};

        reader.seek(m_header.file_entries[i].asset_offset);
        if (sign == "FSTP")
            m_files[i].audio = Fstp::PrefetchFile{
                buffer.begin() + reader.tell(), buffer.end()};
        else if (sign == "FWAV")
            m_files[i].audio =
                Fwav::WaveFile{buffer.begin() + reader.tell(), buffer.end()};
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
            Fstp::PrefetchFile& fstp =
                std::get<Fstp::PrefetchFile>(file.audio);
            fstp.endianness = endianness;
            break;
        }
        default:
            break;
        }
    }
}

std::vector<uint8_t> BarsFile::serialize() {
    AudioWriter writer{endianness};

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
        std::vector<uint8_t> amta_bytes = m_files[i].metadata.serialize();
        std::vector<uint8_t> asset_bytes;
        switch (m_files[i].metadata.data.type) {
        case Amta::Data::Type::Wave: {
            Fwav::WaveFile fwav = std::get<Fwav::WaveFile>(m_files[i].audio);
            asset_bytes = fwav.serialize();
            break;
        }
        case Amta::Data::Type::Stream: {
            Fstp::PrefetchFile fstp =
                std::get<Fstp::PrefetchFile>(m_files[i].audio);
            asset_bytes = fstp.serialize();
            break;
        }
        default:
            throw std::runtime_error("Invalid file type!");
        }

        writer.seek(m_header.file_entries[i].amta_offset);
        for (uint8_t byte : amta_bytes)
            writer.write<uint8_t>(byte);

        writer.seek(m_header.file_entries[i].asset_offset);
        for (uint8_t byte : asset_bytes)
            writer.write<uint8_t>(byte);
    }

    return writer.finalize();
}

} // namespace NSound::Bars
