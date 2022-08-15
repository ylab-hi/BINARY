//
// Created by li002252 on 8/13/22.
//

#include <binary/parser/vcf.hpp>
#include <utility>

namespace binary::parser {

  VcfRecord::VcfRecord(const std::shared_ptr<htsFile>& file)
      : fp_{file},
        header_{bcf_hdr_read(file.get()), utils::bcf_hdr_deleter},
        record_{bcf_init(), utils::bcf_record_deleter} {}

  auto VcfRecord::record() const -> bcf1_t* { return record_.get(); }
  auto VcfRecord::header() const -> bcf_hdr_t* { return header_.get(); }
  auto VcfRecord::chrom() const -> std::string {
    return bcf_seqname_safe(header_.get(), record_.get());
  }
  auto VcfRecord::rlen() const -> pos_t { return record_->rlen; }
  auto VcfRecord::pos() const -> pos_t { return record_->pos; }
  [[maybe_unused]] void VcfRecord::check_header() const {
    if (!header_) throw VcfReaderError("Header is not set");
  }

  auto VcfRecord::is_valid() const -> bool { return record_ != nullptr; }

  auto operator<<(std::ostream& os, const VcfRecord& record) -> std::ostream& {
    os << "Chrom: " << record.chrom() << " Pos: " << record.pos()
       << " SVTYPE:" << get_info_field<char>("SVTYPE", record);
    return os;
  }

  auto operator==(const VcfRecord& lhs, const VcfRecord& rhs) -> bool {
    return lhs.record_ == rhs.record_;
  }

  void VcfRecord::set_eof() { record_.reset(); }
  auto VcfRecord::fp() const -> htsFile* { return fp_.lock().get(); }

  /**
   * @brief VcfReader constructor
   * @param file_path
   */
  VcfReader::VcfReader(std::string file_path)
      : file_path_(std::move(file_path)),
        fp{hts_open(file_path_.c_str(), "r"), utils::bcf_hts_file_deleter} {}

  /**
   * @brief  get file path of the vcf file
   * @return  vcf file path
   */
  auto VcfReader::file_path() const -> const std::string& { return file_path_; }
  /**
   * @brief  check if the vcf file is open
   * @return  vcf file header
   */
  auto VcfReader::is_open() const -> bool { return fp != nullptr; }
  /**
   * @brief check if the vcf file has index
   * @return bool
   */
  auto VcfReader::has_index() const -> bool { return idx != nullptr; }

  auto VcfReader::check_query(const std::string& chrom) -> int {
    if (!has_index()) {
      idx.reset(tbx_index_load(file_path_.c_str()));
      if (!idx) {
        throw VcfReaderError("Failed to load index for " + file_path_);
      }
    }

    if (bcf_hdr_name2id(current_.value_.header(), chrom.c_str()) < 0) {
      throw VcfReaderError(chrom + " is not in the vcf file " + file_path_);
    }
    auto tid = tbx_name2id(idx.get(), chrom.c_str());
    assert(tid > 0);
    return tid;
  }

  auto VcfReader::iter_query_record() -> VcfReader::iterator& {
    int ret = tbx_itr_next(fp.get(), idx.get(), itr_ptr.get(), ks_ptr.get());
    if (ret < -1) {
      throw VcfReaderError("Query-> Failed to query ");
    } else if (ret == -1) {
      return sentinel_;
    }
    // no problem
    vcf_parse1(ks_ptr.get(), current_.value_.header(), current_.value_.record());
    return current_;
  }

  /**
   * @brief  query the vcf file if has index
   * @param chrom chromosome name
   * @param start start position
   * @param end end position
   * @return vcf record
   */
  auto VcfReader::query(std::string const& chrom, pos_t start, pos_t end) -> VcfReader::iterator& {
    seek();  // seek to the first record and initialize the iterator

    auto tid = check_query(chrom);  // may throw error
    itr_ptr.reset(tbx_itr_queryi(idx.get(), tid, start, end));

    if (!itr_ptr) {
      throw VcfReaderError("Query-> Failed to query " + chrom + ":" + std::to_string(start) + "-"
                           + std::to_string(end));
    }
    return iter_query_record();
  }

  void VcfReader::seek() const { current_ = iterator{fp}; }

  auto operator==(const VcfReader& lhs, const VcfReader& rhs) -> bool {
    if (lhs.file_path_ != rhs.file_path_) return false;
    if (lhs.fp != rhs.fp) return false;
    return true;
  }

  auto VcfReader::begin() -> VcfReader::iterator& {
    seek();
    return ++current_;
  }

  auto VcfReader::begin() const -> const VcfReader::iterator& {
    seek();
    return ++current_;
  }

  auto VcfReader::end() -> VcfReader::iterator& { return sentinel_; }
  auto VcfReader::end() const -> const VcfReader::iterator& { return sentinel_; }

  VcfReader::iterator::iterator(const std::shared_ptr<htsFile>& file) : value_{file} {}

  auto VcfReader::iterator::operator*() const -> const VcfReader::iterator::value_type& {
    return value_;
  }

  auto VcfReader::iterator::operator*() -> VcfReader::iterator::value_type& { return value_; }
  auto VcfReader::iterator::operator++() -> VcfReader::iterator& {
    int ret = bcf_read(value_.fp(), value_.header(), value_.record());
    if (ret < -1) {
      throw VcfReaderError("Failed to read line in vcf ");
    } else if (ret == -1) {
      value_.set_eof();
    }
    return *this;
  }

  auto get_info_field_int32(const std::string& key, VcfRecord const& vcf_record) -> pos_t {
    return get_info_field<pos_t>(key, vcf_record);
  }

  auto get_info_field_string(const std::string& key, VcfRecord const& vcf_record) -> std::string {
    return get_info_field<char>(key, vcf_record);
  }

  auto get_info_field_int32(std::initializer_list<std::string> keys, const VcfRecord& vcf_record)
      -> std::vector<pos_t> {
    std::vector<pos_t> values;

    for (auto const& key : keys) {
      values.push_back(get_info_field_int32(key, vcf_record));
    }
    return values;
  }

  auto get_info_field_string(std::initializer_list<std::string> keys, VcfRecord const& vcf_record)
      -> std::vector<std::string> {
    std::vector<std::string> values;
    for (auto const& key : keys) {
      values.push_back(get_info_field_string(key, vcf_record));
    }

    return values;
  }
}  // namespace binary::parser
