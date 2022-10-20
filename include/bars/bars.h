#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <oead/util/binary_reader.h>

/*
    sources:
    https://wiki.oatmealdome.me/Aal/BARS_(File_Format)
    https://github.com/Kinnay/Nintendo-File-Formats/wiki/AMTA-File-Format
    https://github.com/aboood40091/BCFSTM-BCFWAV-Converter
    https://github.com/NanobotZ/bfstp-fixer
*/

namespace NSound {
extern std::map<std::string, int> reference_types;

struct BlockHeader {
  std::array<uint8_t, 4> signature;
  uint32_t section_size;
};

struct Reference {
  uint16_t type;
  uint8_t padding[2];
  int32_t offset;
};

struct SizedReference {
  Reference ref;
  uint32_t size;
};

struct ReferenceTable {
  uint32_t count;
  std::vector<Reference> refs;
};

namespace Amta {
struct Data {
  enum Type : uint8_t { Wave = 0, Stream };

  BlockHeader header;

  // can be 0, but still have an entry in the string table
  uint32_t asset_name_offset;

  uint32_t sample_count;
  Type type;  // 0=Wave, 1=Stream
  uint8_t wave_channels;
  uint8_t used_stream_tracks;  // Up to 8
  uint8_t flags;
  float volume;
  uint32_t sample_rate;
  uint32_t loop_start_sample;
  uint32_t loop_end_sample;
  float loudness;

  struct StreamTrack {
    uint32_t channel_count;
    float volume;
  };
  std::array<StreamTrack, 8> stream_tracks;

  float amplitude_peak;  // Only in Version 4.0!!!!!
};

struct Marker {
  BlockHeader header;
  uint32_t entry_count;

  struct MarkerInfo {
    uint32_t id;

    // can be 0, but still have an entry in the string table
    uint32_t asset_name_offset;

    uint32_t start_pos;
    uint32_t length;
  };
  std::vector<MarkerInfo> marker_infos;  // size = this.entry_count
};

struct Ext_ {
  BlockHeader header;
  uint32_t entry_count;

  struct ExtEntry {
    uint32_t unknown[2];
  };
  std::vector<ExtEntry> ext_entries;  // size = this.entry_count
};

struct StringTable {
  BlockHeader header;
  std::string asset_name;
};

struct Header {
  std::array<uint8_t, 4> signature;

  uint16_t bom;
  uint16_t version;  // 0x100, 0x300 or 0x400
  uint32_t file_size;
  uint32_t data_offset;
  uint32_t marker_offset;
  uint32_t ext__offset;
  uint32_t string_table_offset;
};
}  // Namespace Amta

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
  std::vector<std::variant<StreamInfo, WaveInfo, BlockHeader, PrefetchDataBlock, DataBlock>> blocks;
};

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
}  // namespace Bars
};  // namespace NSound

// --------- Helper Functions --------------------------------------------------

NSound::Bars::Header get_main_header(const std::string& file_name);
void extract_amta(const NSound::Bars::Header header, oead::util::BinaryReader reader);

// -----------------------------------------------------------------------------