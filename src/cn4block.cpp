/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <ctime>
#define BOOST_NO_AUTO_PTR
#include <boost/endian/conversion.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/locale.hpp>
#include "cn4block.h"
#include "ca4block.h"
#include "sd4block.h"
#include "cg4block.h"


namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCx = 1;
constexpr size_t kIndexName = 2;
constexpr size_t kIndexSi = 3;
constexpr size_t kIndexCc = 4;
constexpr size_t kIndexData = 5;
constexpr size_t kIndexUnit = 6;
constexpr size_t kIndexMd = 7;
constexpr size_t kIndexAt = 8;

constexpr uint32_t kDefaultXFlag = 0x1000; ///< Default X-axis defined for this channel

std::string MakeTypeString(uint8_t type) {
  switch (type) {
    case 0: return "Fixed";
    case 1: return "VLSD";
    case 2: return "Master";
    case 3: return "Virtual Master";
    case 4: return "Synchronization";
    case 5: return "MLSD";
    case 6: return "Virtual Data";
    default:break;
  }
  return "Unknown";
}

std::string MakeSyncString(uint8_t type) {
  switch (type) {
    case 0: return "None";
    case 1: return "Time";
    case 2: return "Angle";
    case 3: return "Distance";
    case 4: return "Index";
    default:break;
  }
  return "Unknown";
}

std::string MakeDataTypeString(uint8_t type) {
  switch (type) {
    case 0: return "Unsigned Integer (LE)";
    case 1: return "Unsigned Integer (BE)";
    case 2: return "Signed Integer (LE)";
    case 3: return "Signed Integer (BE)";
    case 4: return "Float (LE)";
    case 5: return "Float (BE)";
    case 6: return "String ASCII";
    case 7: return "String UTF-8";
    case 8: return "String UTF-16 (LE)";
    case 9: return "String UTF-16 (BE)";
    case 10: return "Byte Array";
    case 11: return "MIME Sample";
    case 12: return "MIME Stream";
    case 13: return "CAN Open Date";
    case 14: return "CAN Open Time";
    default:break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint32_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "All Invalid";
  }
  if (flag & 0x02) {
    s << (s.str().empty() ? "Invalid" : ",Invalid");
  }
  if (flag & 0x04) {
    s << (s.str().empty() ? "Decimal" : ",Decimal");
  }
  if (flag & 0x08) {
    s << (s.str().empty() ? "Range" : ",Range");
  }
  if (flag & 0x10) {
    s << (s.str().empty() ? "Limit" : ",Limit");
  }
  if (flag & 0x20) {
    s << (s.str().empty() ? "Extended" : ",Extended");
  }
  if (flag & 0x40) {
    s << (s.str().empty() ? "Discrete" : ",Discrete");
  }
  if (flag & 0x80) {
    s << (s.str().empty() ? "Calibration" : ",Calibration");
  }
  if (flag & 0x100) {
    s << (s.str().empty() ? "Calculated" : ",Calculated");
  }
  if (flag & 0x200) {
    s << (s.str().empty() ? "Virtual" : ",Virtual");
  }
  if (flag & 0x400) {
    s << (s.str().empty() ? "Event" : ",Event");
  }
  if (flag & 0x800) {
    s << (s.str().empty() ? "Monotonous" : ",Monotonous");
  }
  if (flag & 0x1000) {
    s << (s.str().empty() ? "X" : ",X");
  }
  return s.str();
}

///< Helper function that recursively copies all data bytes to a
/// destination buffer.
size_t CopyDataToBuffer( const mdf::detail::IBlock* data, std::FILE* from_file,
                         std::vector<uint8_t>& buffer, size_t& buffer_index ) {
  if (data == nullptr) {
    return 0;
  }
  size_t count = 0;
  const auto* db = dynamic_cast< const mdf::detail::DataBlock* > (data);
  const auto* dl = dynamic_cast< const mdf::detail::DataListBlock* > (data);
  if (db != nullptr) {
    count += db->CopyDataToBuffer(from_file, buffer, buffer_index);
  } else if (dl != nullptr) {
    for (const auto& block : dl->DataBlockList()) {
      count += CopyDataToBuffer(block.get(), from_file, buffer, buffer_index);
    }
  }
  return count;
}

} // end namespace

namespace mdf::detail {

int64_t Cn4Block::DataLink() const {
  return Link(kIndexData);
}

std::vector<int64_t> Cn4Block::AtLinkList() const {
  std::vector<int64_t> link_list;
  for (uint16_t ii = 0; ii < nof_attachments_; ++ii) {
    link_list.push_back(Link(kIndexAt + ii));
  }
  return link_list;
}

std::vector<int64_t> Cn4Block::XAxisLinkList() const {
  std::vector<int64_t> link_list;
  for (uint16_t ii = 0; ii < 3 && (flags_ & kDefaultXFlag) != 0; ++ii) {
    link_list.push_back(Link(kIndexAt + nof_attachments_ + ii));
  }
  return link_list;
}

int64_t Cn4Block::Index() const {
  return FilePosition();
}

void Cn4Block::Name(const std::string &name) {
  name_ = name;
}

std::string Cn4Block::Name() const {
  return name_;
}

void Cn4Block::DisplayName(const std::string &name) {
}

std::string Cn4Block::DisplayName() const {
  return std::string();
}

void Cn4Block::Description(const std::string &description) {
  md_comment_ = std::make_unique<Md4Block>(description);
}

std::string Cn4Block::Description() const {
  return MdText();
}

const IChannelConversion *Cn4Block::ChannelConversion() const {
  return cc_block_.get();
}
ChannelDataType Cn4Block::DataType() const {
  return static_cast<ChannelDataType>(data_type_);
}
ChannelType Cn4Block::Type() const {
  return static_cast<ChannelType>(type_);
}
size_t Cn4Block::DataBytes() const {
  return (bit_count_ / 8) + (bit_count_ % 8 > 0 ? 1 : 0);
}
uint8_t Cn4Block::Decimals() const {
  auto max = static_cast<uint8_t>( DataBytes() == 4 ?
                                   std::numeric_limits<float>::max_digits10 :
                                   std::numeric_limits<double>::max_digits10);
  return std::min(precision_, max);
}
bool Cn4Block::IsDecimalUsed() const {
  return flags_ & 0x04;
}

bool Cn4Block::IsUnitValid() const {
  return Link(kIndexUnit) != 0;
}

std::string Cn4Block::Unit() const {
  if (!unit_) {
    return {};
  }
  if (unit_->IsTxtBlock()) {
    return unit_->Text();
  }
  return unit_->TxComment();
}

void Cn4Block::GetBlockProperty(BlockPropertyList &dest) const {
  IBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next CN", ToHexString(Link(kIndexNext)), "Link to next channel", BlockItemType::LinkItem );
  dest.emplace_back("Composition CA/CN", ToHexString(Link(kIndexCx)), "Link to composition",BlockItemType::LinkItem );
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), "Link to channel name", BlockItemType::LinkItem );
  dest.emplace_back("Source SI", ToHexString(Link(kIndexSi)), "Link to source information", BlockItemType::LinkItem );
  dest.emplace_back("Conversion CC", ToHexString(Link(kIndexCc)), "Link to channel conversion", BlockItemType::LinkItem );
  dest.emplace_back("Signal Data", ToHexString(Link(kIndexData)), "Link to signal data",BlockItemType::LinkItem );
  dest.emplace_back("Unit MD", ToHexString(Link(kIndexUnit)), "Link to unit",BlockItemType::LinkItem );
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), "Link to meta data",BlockItemType::LinkItem );
  for (size_t at = 0; at < nof_attachments_; ++at) {
    dest.emplace_back("Attachment AT", ToHexString(Link(kIndexAt + at)), "Reference to attachment",BlockItemType::LinkItem );
  }
  if (Link(kIndexAt + nof_attachments_) > 0) {
    dest.emplace_back("Reference DG", ToHexString(Link(kIndexAt + nof_attachments_)), "Reference to x-axis data block",BlockItemType::LinkItem );
    dest.emplace_back("Reference CG", ToHexString(Link(kIndexAt + nof_attachments_ + 1)), "Reference to x-axis channel group",BlockItemType::LinkItem );
    dest.emplace_back("Reference CN", ToHexString(Link(kIndexAt + nof_attachments_ + 2)), "Reference to x-axis channel",BlockItemType::LinkItem );
  }
  dest.emplace_back("", "", "",BlockItemType::BlankItem );

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name", name_);
  dest.emplace_back("Unit", Unit());
  dest.emplace_back("Channel Type", MakeTypeString(type_));
  dest.emplace_back("Synchronization Type", MakeSyncString(sync_type_));
  dest.emplace_back("Data Type", MakeDataTypeString(data_type_));
  dest.emplace_back("Bit Offset", std::to_string(bit_offset_));
  dest.emplace_back("Byte Offset", std::to_string(byte_offset_));
  dest.emplace_back("Bit Count", std::to_string(bit_count_));
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Invalid Bit Position", std::to_string(invalid_bit_pos_));
  dest.emplace_back("Decimals", std::to_string(precision_));
  dest.emplace_back("Nof Attachments", std::to_string(nof_attachments_));
  dest.emplace_back("Range Min", ToString(range_min_));
  dest.emplace_back("Range Max", ToString(range_max_));
  dest.emplace_back("Limit Min", ToString(limit_min_));
  dest.emplace_back("Limit Max", ToString(limit_max_));
  dest.emplace_back("Extended Limit Min", ToString(limit_ext_min_));
  dest.emplace_back("Extended Limit Max", ToString(limit_ext_max_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

size_t Cn4Block::Read(std::FILE *file) {
  size_t bytes = ReadHeader4(file);
  bytes += ReadNumber(file, type_);
  bytes += ReadNumber(file, sync_type_);
  bytes += ReadNumber(file, data_type_);
  bytes += ReadNumber(file, bit_offset_);
  bytes += ReadNumber(file, byte_offset_);
  bytes += ReadNumber(file, bit_count_);
  bytes += ReadNumber(file, flags_);
  bytes += ReadNumber(file, invalid_bit_pos_);
  bytes += ReadNumber(file, precision_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(file, reserved, 1);
  bytes += ReadNumber(file, nof_attachments_);
  bytes += ReadNumber(file, range_min_);
  bytes += ReadNumber(file, range_max_);
  bytes += ReadNumber(file, limit_min_);
  bytes += ReadNumber(file, limit_max_);
  bytes += ReadNumber(file, limit_ext_min_);
  bytes += ReadNumber(file, limit_ext_max_);

  name_ = ReadTx4(file, kIndexName);

  if (Link(kIndexCx) > 0) {
    SetFilePosition(file, Link(kIndexCx));
    auto block_type = ReadBlockType(file);

    SetFilePosition(file, Link(kIndexCx));
    if (block_type == "CA") {
      cx_block_ = std::make_unique<Ca4Block>();
      cx_block_->Init(*this);
      cx_block_->Read(file);
    } else if (block_type == "CN") {
      cx_block_ = std::make_unique<Cn4Block>();
      cx_block_->Init(*this);
      cx_block_->Read(file);
    }
  }

  if (Link(kIndexSi) > 0) {
    SetFilePosition(file, Link(kIndexSi));
    si_block_ = std::make_unique<Si4Block>();
    si_block_->Init(*this);
    si_block_->Read(file);
  }

  if (Link(kIndexCc) > 0) {
    SetFilePosition(file, Link(kIndexCc));
    cc_block_ = std::make_unique<Cc4Block>();
    cc_block_->Init(*this);
    cc_block_->ChannelDataType(data_type_);
    cc_block_->Read(file);
  }

  // Need ot check if the data block is owned by this CN block or if it is a reference only
  ReadBlockList(file, kIndexData);

  if (Link(kIndexUnit) > 0) {
    SetFilePosition(file, Link(kIndexUnit));
    unit_ = std::make_unique<Md4Block>();
    unit_->Init(*this);
    unit_->Read(file);
  }
  ReadMdComment(file,kIndexMd);

  return bytes;
}

const IBlock *Cn4Block::Find(fpos_t index) const {
  if (si_block_) {
    const auto* p = si_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  if (cc_block_) {
    const auto* p = cc_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  if (unit_) {
    const auto* p = unit_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  if (cx_block_) {
    const auto* p = cx_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  return DataListBlock::Find(index);
}

void Cn4Block::ReadData(std::FILE *file) const {
  size_t count = 0;
  for (const auto& b : DataBlockList()) {
    const auto *dl = dynamic_cast<const DataListBlock *>(b.get());
    const auto *db = dynamic_cast<const DataBlock *>(b.get());
    if (dl != nullptr) {
     count += dl->DataSize();
    } else if (db != nullptr) {
      count += db->DataSize();
    }
  }

  size_t index = 0;
  data_list_.resize(count, 0);
  for (const auto& block : DataBlockList()) {
    const auto *dl = dynamic_cast<const DataListBlock *>(block.get());
    const auto *db = dynamic_cast<const DataBlock *>(block.get());
    if (dl != nullptr) {
      CopyDataToBuffer(dl, file, data_list_, index);
    } else if (db != nullptr) {
      db->CopyDataToBuffer(file, data_list_, index);
    }
  }
}
size_t Cn4Block::BitCount() const {
  return bit_count_;
}
size_t Cn4Block::BitOffset() const {
  return bit_offset_;
}
size_t Cn4Block::ByteOffset() const {
  return byte_offset_;
}

bool Cn4Block::GetTextValue(const std::vector<uint8_t> &record_buffer, std::string &dest) const {
  auto offset = ByteOffset();
  std::vector<uint8_t> temp;
  bool valid = true;
  dest.clear();
  if (Type() == ChannelType::VariableLength) {
    // Index into the local data buffer
    uint64_t index = 0;
    valid = GetUnsignedValue(record_buffer, index);
    if (index + 4 > data_list_.size()) {
      return false;
    }
    boost::endian::little_uint32_buf_t length;
    memcpy(length.data(), data_list_.data() + index, 4 );
    temp.resize(length.value(), 0);
    if (index + 4 + length.value() <= data_list_.size()) {
      memcpy(temp.data(), data_list_.data() + index + 4, length.value());
    }
    offset = 0;
  } else {
    temp = record_buffer;
  }

  switch (DataType()) {
    case ChannelDataType::StringAscii: {
      // Convert ASCII to UTF8
      std::ostringstream s;
      for (size_t ii = offset; ii < temp.size(); ++ii) {
        char in = static_cast<char>(temp[ii]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      try {
        dest = boost::locale::conv::to_utf<char>(s.str(), "Latin1");
      } catch (const std::exception&) {
        valid = false; // Conversion error
        dest = s.str();
      }
      break;
    }

    case ChannelDataType::StringUTF8: {
      // No conversion needed
      std::ostringstream s;
      for (size_t ii = offset; ii < temp.size(); ++ii) {
        char in = static_cast<char>(temp[ii]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      dest = s.str();
      break;
    }

    case ChannelDataType::StringUTF16Le: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= temp.size(); ii += 2) {
        auto* d = temp.data() + ii;
        boost::endian::little_uint16_buf_t data;
        memcpy(data.data(), temp.data() + ii, 2);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = boost::locale::conv::utf_to_utf<char>(s.str());
      } catch (const std::exception&) {
        valid = false; // Conversion error
      }
      break;
    }

    case ChannelDataType::StringUTF16Be: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= temp.size(); ii += 2) {
        auto* d = temp.data() + ii;
        boost::endian::big_uint16_buf_t data;
        memcpy(data.data(), d, 2);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = boost::locale::conv::utf_to_utf<char>(s.str());
      } catch (const std::exception&) {
        valid = false; // Conversion error
      }
      break;
    }
    default:
      break;
  }
  return valid;
}


void Cn4Block::Unit(const std::string &unit) {

}
void Cn4Block::Type(ChannelType type) {

}
void Cn4Block::DataType(ChannelDataType type) {

}
void Cn4Block::DataBytes(size_t nof_bytes) {

}
void Cn4Block::SamplingRate(double sampling_rate) {

}
double Cn4Block::SamplingRate() const {
  return 0;
}

std::vector<uint8_t> &Cn4Block::SampleBuffer() const {
  return cg_block_->SampleBuffer();
}
void Cn4Block::Init(const IBlock &id_block) {
  IBlock::Init(id_block);
  cg_block_ = dynamic_cast<const Cg4Block*>(&id_block);
}

} // namespace mdf::detail