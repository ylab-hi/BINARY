//
// Created by li002252 on 8/2/22.
//

#include "sv2nl/vcf_record.hpp"

namespace sv2nl {

  auto VcfRecord::get_record() const -> bcf1_t* { return record_.get(); }
  auto VcfRecord::get_header() const -> bcf_hdr_t* { return header_.get(); }
  auto VcfRecord::get_chrom() const -> std::string {
    return bcf_seqname_safe(header_.get(), record_.get());
  }
  auto VcfRecord::get_rlen() const -> int64_t { return record_->rlen; }
  auto VcfRecord::get_pos() const -> int64_t { return record_->pos; }
  void VcfRecord::check_header() const {
    if (!header_) throw VcfReaderError("Header is not set");
  }
  auto VcfRecord::is_valid() const -> bool { return record_ != nullptr && header_ != nullptr; }
  auto operator<<(std::ostream& os, const VcfRecord& record) -> std::ostream& {
    os << "Chrom: " << record.get_chrom() << " Pos: " << record.get_pos()
       << " SVTYPE:" << get_info_field<char>("SVTYPE", record);
    return os;
  }
  auto VcfRecord::operator++() -> VcfRecord& {
    int ret = bcf_read(file_.lock().get(), header_.get(), record_.get());

    if (ret < -1) {
      throw VcfReaderError("Failed to read line in vcf ");
    } else if (ret == -1) {
      set_end_of_file();  // record_ = nullptr;  mean end of file
    }

    return *this;
  }
  auto VcfRecord::operator++(int) -> VcfRecord {
    VcfRecord old = *this;
    ++*this;
    return old;
  }
  auto VcfRecord::operator*() const -> VcfRecord::value_type {
    return std::make_tuple(get_chrom(), get_pos());
  }
  auto operator==(const VcfRecord& lhs, const VcfRecord& rhs) -> bool {
    if (lhs.header_ != rhs.header_)
      throw VcfReaderError("VcfRecord from differ files cannot be compared");
    return lhs.record_ == rhs.record_;
  }
  void VcfRecord::set_end_of_file() { record_ = nullptr; }

  auto get_info_field_int32(const std::string& key, VcfRecord const& vcf_record) -> int32_t {
    auto data = get_info_field<int32_t>(key, vcf_record);
    return data;
  }

  auto get_info_field_string(const std::string& key, VcfRecord const& vcf_record) -> std::string {
    auto data = get_info_field<char>(key, vcf_record);
    return data;
  }

}  // namespace sv2nl