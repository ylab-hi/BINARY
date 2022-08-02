//
// Created by li002252 on 5/19/22.
//

#include <sv2nl/vcf_reader.hpp>

namespace sv2nl {
  void bcf_record_deleter(bcf1_t* record) noexcept {
    if (record) bcf_destroy(record);
  };
  void bcf_hdr_deleter(bcf_hdr_t* hdr) noexcept {
    if (hdr) bcf_hdr_destroy(hdr);
  };
  void bcf_hts_file_deleter(htsFile* hts_file) noexcept {
    if (hts_file) hts_close(hts_file);
  }

  void bcf_tbx_deleter(tbx_t* tbx) noexcept {
    if (tbx) tbx_destroy(tbx);
  }

  void bcf_itr_deleter(hts_itr_t* itr) noexcept {
    if (itr) hts_itr_destroy(itr);
  }

  void bcf_kstring_deleter(kstring_t* ks) noexcept {
    if (ks) free(ks->s);
  }

  struct VcfReader::impl {
    std::string file_path_{};
    std::shared_ptr<htsFile> fp{nullptr, bcf_hts_file_deleter};
    mutable VcfRecord record{};
    VcfRecord end_record{};

    // late load
    tbx_t* idx{nullptr};
    kstring_t kstr{};
    hts_itr_t* itr{nullptr};

    explicit impl(std::string file_path)
        : file_path_{std::move(file_path)},
          fp{hts_open(file_path_.c_str(), "r"), bcf_hts_file_deleter},
          record{fp},
          end_record(record) {
      end_record.set_end_of_file();
    }

    ~impl() { destroy(); }

    impl(impl const&) = delete;
    auto operator=(impl const&) -> impl& = delete;
    impl(impl&&) = default;
    auto operator=(impl&&) -> impl& = default;

    void destroy() noexcept {
      if (idx) {
        tbx_destroy(idx);
      }
      if (itr) {
        hts_itr_destroy(itr);
      }
      free(kstr.s);

      fp = nullptr;
      idx = nullptr;
      itr = nullptr;
    }

    [[nodiscard]] auto get_file_path() const -> const std::string& { return file_path_; }

    void check_record() const {
      if (!record.is_valid()) {
        throw VcfReaderError("Record is null");
      }
    }

    // TODO: Need to change
    [[nodiscard]] auto get_chrom() const -> std::string { return record.get_chrom(); }
    [[nodiscard]] auto get_pos() const -> int64_t { return record.get_pos(); }
    [[nodiscard]] auto get_rlen() const -> int64_t { return record.get_rlen(); }

    [[maybe_unused]] void print_record() const { std::cout << record << "query\n"; }

    auto begin() -> VcfRecord& { return ++record; }
    auto end() const -> VcfRecord { return end_record; }

    auto check_query(std::string const& chrom) -> int {
      idx = tbx_index_load(file_path_.c_str());
      if (!idx) {
        throw VcfReaderError("Query-> Failed to load index for vcf " + file_path_);
      }

      if (bcf_hdr_name2id(record.get_header(), chrom.c_str()) < 0) {
        throw VcfReaderError(chrom + "  is not in file " + file_path_);
      }
      auto tid = tbx_name2id(idx, chrom.c_str());
      if (tid <= 0) {
        throw VcfReaderError(chrom + "  is not in file " + file_path_);
      }
      return tid;
    }

    auto iter_query_record() -> VcfRecord const& {
      int ret = tbx_itr_next(fp.get(), idx, itr, &kstr);
      if (ret < -1) {
        throw VcfReaderError("Query-> Failed to query ");
      } else if (ret == -1) {
        return end_record;
      }
      // no problem
      vcf_parse1(&kstr, record.get_header(), record.get_record());
      return record;
    }

    auto query(std::string const& chrom_, int64_t start_, int64_t end_) -> VcfRecord const& {
      auto tid = check_query(chrom_);  // may throw error
      itr = tbx_itr_queryi(idx, tid, start_, end_);
      if (!itr) {
        throw VcfReaderError("Query-> Failed to query " + chrom_ + ":" + std::to_string(start_)
                             + "-" + std::to_string(end_));
      }
      return iter_query_record();
    }
  };

  // VcfReader
  VcfReader::VcfReader(const std::string& file_path) : pimpl{std::make_unique<impl>(file_path)} {}

  auto VcfReader::operator=(VcfReader&&) noexcept -> VcfReader& = default;
  VcfReader::VcfReader(VcfReader&&) noexcept = default;
  VcfReader::~VcfReader() noexcept = default;

  // status
  auto VcfReader::get_file_path() const -> const std::string& { return pimpl->get_file_path(); }
  auto VcfReader::is_open() const -> bool { return pimpl->fp != nullptr; }
  auto VcfReader::is_closed() const -> bool { return pimpl->fp == nullptr; }
  auto VcfReader::has_index() const -> bool { return pimpl->idx != nullptr; }

  // getters
  auto VcfReader::get_chrom() const -> std::string { return pimpl->get_chrom(); }
  auto VcfReader::get_pos() const -> int64_t { return pimpl->get_pos(); }
  auto VcfReader::get_rlen() const -> int64_t { return pimpl->get_rlen(); }
  void VcfReader::check_record() const { pimpl->check_record(); }

  auto VcfReader::iter_query_record() -> VcfRecord const& { return pimpl->iter_query_record(); }
  auto VcfReader::query(const std::string& chrom, int64_t start, int64_t end) -> VcfRecord const& {
    return pimpl->query(chrom, start, end);
  }

  auto VcfReader::begin() -> VcfRecord& { return pimpl->begin(); }
  auto VcfReader::begin() const -> VcfRecord const& { return pimpl->begin(); }
  auto VcfReader::end() const -> VcfRecord { return pimpl->end(); }

  // VcfRecord
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