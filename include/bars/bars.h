#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <bars/common.h>
#include <oead/util/binary_reader.h>

/*
    sources:
    https://wiki.oatmealdome.me/Aal/BARS_(File_Format)
    https://github.com/Kinnay/Nintendo-File-Formats/wiki/AMTA-File-Format
    https://github.com/aboood40091/BCFSTM-BCFWAV-Converter
    https://github.com/NanobotZ/bfstp-fixer
*/

/*

    *.bars
    |
    |
    +------ FileHeader
    |      |
    |       +-- char signature[4] // "BARS" in ASCII
    |       +-- uint32_t file_size
    |       +-- uint16_t byte order_mark // FEFF for Big-Endian; FFFE for Little-Endian
    |       +-- uint16_t version
    |       +-- uint32_t asset_count
    |       +-- uint32_t crc32hashes[asset_count]
    |       +-- FileEntries // one for each file
    |           |
    |           +---- uint32_t amta_offset
    |           +---- uint32_t asset_offset
    +------ AmtaArray
    |
    +------ Prefetch and Wave Array



*/
#ifndef NSOUND_BARS_H
#define NSOUND_BARS_H

namespace NSound {
extern std::map<std::string, int> reference_types;

namespace Bars {
struct TrackInfo {
  uint8_t volume;
  uint8_t pan;
  uint16_t unknown;
};

struct ChannelInfo {
  Reference sample_ref;
  Reference adpcm_info_ref;
  uint32_t reserved;
};

struct DspContext {
  std::array<uint16_t, 16> coefficients;
  uint16_t predictor_scale;
  uint16_t pre_sample;
  uint16_t pre_sample2;
};

struct DspLoopContext {
  uint16_t predictor_scale;
  uint16_t loop_pre_sample;
  uint16_t loop_pre_sample2;
};

struct AdpcmInfo {
  DspContext context;
  DspLoopContext loop_context;
};

struct ImaContext {
  uint16_t data;
  uint16_t table_index;
};

struct RegionInfo {
  uint16_t region_size;
  uint8_t padding1[2];
  uint16_t region_flag;
  uint8_t padding2[2];
  int32_t region_offset;
  uint32_t og_loop_start;
  uint32_t og_loop_end;
};

struct StreamInfo {
  uint8_t codec;
  bool loop_flag;
  uint8_t channel_count;
  uint8_t region_count;
  uint32_t sample_rate;
  uint32_t loop_start;
  uint32_t sample_count;
  uint32_t block_count;
  uint32_t block_size;
  uint32_t block_sample_count;
  uint32_t last_block_size;
  uint32_t last_block_sample_count;
  uint32_t last_block_padding_size;
  uint32_t seek_size;
  uint32_t sisc;
  RegionInfo region_info;  // Used only if region_count > 0, else omitted
  uint32_t crc32hash;      // Used to check that the revision of a prefetch and stream files match
};

struct WaveInfo {
  uint8_t codec;
  uint8_t loop_flag;
  uint32_t sample_rate;
  uint32_t loop_start;
  uint32_t loop_end;
  uint32_t og_loop_start;
  // ReferenceTable  ref_table;
};

struct DataBlock {
  BlockHeader header;
  std::vector<uint16_t> data;
};

struct PrefetchData {
  uint32_t start_frame;
  uint32_t prefetch_size;
  uint8_t reserved;
  Reference to_prefetch_sample;
};

struct PrefetchDataBlock {
  BlockHeader header;
  ReferenceTable prefetch_data;
  std::vector<uint16_t> data;
};

struct PrefetchHeader {
  BlockHeader header;
  ReferenceTable prefetch_refs;
};

struct AudioHeader {
  std::array<uint8_t, 4> signature;
  uint16_t bom;
  uint16_t head_size;
  uint32_t version;
  uint32_t file_size;
  uint16_t block_count;
  uint16_t reserved;
  // Size = this.block_count
  std::vector<SizedReference> block_refs;
};

AudioHeader load_audio_header(oead::util::BinaryReader& reader);

struct Header {
  std::array<uint8_t, 4> signature;

  uint32_t file_size;
  uint16_t bom;
  uint16_t version;
  uint32_t asset_count;

  std::vector<uint32_t> crc32hashes;  // size = this.asset_count; must be sorted

  struct FileEntry {
    uint32_t amta_offset;
    uint32_t asset_offset;
  };
  std::vector<FileEntry> file_entries;  // size = this.asset_count; same order as the CRC32 hashes
};

Header load_header(oead::util::BinaryReader& reader);

}  // namespace Bars

class Parser {
public:
    Parser(std::string file_name);
    void load();
private:
    std::vector<uint8_t> buffer;
    oead::util::BinaryReader reader;
};

};  // namespace NSound

#endif