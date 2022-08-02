//
// Created by li002252 on 5/19/22.
//

#include <sv2nl/vcf_reader.h>

namespace sv2nl {

  struct VcfReader::impl {
    std::string file_path_{};
    htsFile* fp{nullptr};
    bcf_hdr_t* hdr{nullptr};
    bcf1_t* record{nullptr};
    // late load
    tbx_t* idx{nullptr};
    kstring_t kstr{};
    hts_itr_t* itr{nullptr};

    explicit impl(std::string file_path) : file_path_{std::move(file_path)} { init(); }

    ~impl() {
      if (fp) {
        destroy();
      }
    }

    impl(impl const&) = delete;
    auto operator=(impl const&) -> impl& = delete;
    impl(impl&&) = default;
    auto operator=(impl&&) -> impl& = default;

    void init() {
      fp = hts_open(file_path_.c_str(), "r");
      hdr = bcf_hdr_read(fp);
      record = bcf_init();
    }

    void destroy() noexcept {
      if (int ret = hts_close(fp); ret < 0) {
        spdlog::error("Failed to destroy file");
      }

      bcf_hdr_destroy(hdr);

      if (record) {
        bcf_destroy(record);
      }
      if (idx) {
        tbx_destroy(idx);
      }
      if (itr) {
        hts_itr_destroy(itr);
      }
      free(kstr.s);

      hdr = nullptr;
      fp = nullptr;
      record = nullptr;
      idx = nullptr;
      itr = nullptr;
    }

    void open(std::string const& file_path) {
      if (fp) {
        throw std::invalid_argument("File is already opened " + file_path_);
      }
      file_path_ = file_path;
      init();
    }

    [[nodiscard]] auto get_file_path() const -> const std::string& { return file_path_; }

    void check_record() const {
      if (!record) {
        throw VcfReaderError("Record is null");
      }
    }

    [[nodiscard]] auto get_chrom() const -> std::string { return bcf_seqname_safe(hdr, record); }
    [[nodiscard]] auto get_pos() const -> int64_t { return record->pos; }
    [[nodiscard]] auto get_rlen() const -> int64_t { return record->rlen; }

    [[nodiscard]] auto get_info_string(std::string const& key) const -> std::string {
      std::string value;
      char* data = nullptr;
      int32_t count = 0;

      if (int ret = bcf_get_info_string(hdr, record, key.c_str(), &data, &count); ret < 0) {
        spdlog::error("Failed to get info string {}", key);
      } else {
        value = data;
        free(data);
      }
      return value;
    }

    [[nodiscard]] auto get_info_int(std::string const& key) const -> int32_t {
      int32_t value{-1};
      int32_t* data{nullptr};
      int32_t count{};

      if (int ret = bcf_get_info_int32(hdr, record, key.c_str(), &data, &count); ret < 0) {
        spdlog::error("Failed to get info int {}", key);
      } else {
        value = *data;
        free(data);
      }
      return value;
    }

    [[maybe_unused]] void print_record() const {
      fmt::print("Chrom: {} Pos: {}\n", get_chrom(), get_pos());
    }

    [[nodiscard]] auto next_record() const -> int {
      // 0 on success; -1 on end of file; < -1 on critical error

      int ret = bcf_read(fp, hdr, record);
      if (ret < -1) {
        throw VcfReaderError("Failed to read line in vcf " + file_path_);
      }
      return ret;
    }

    auto check_query(std::string const& chrom) -> int {
      idx = tbx_index_load(file_path_.c_str());
      if (!idx) {
        throw VcfReaderError("Query-> Failed to load index for vcf " + file_path_);
      }

      if (bcf_hdr_name2id(hdr, chrom.c_str()) < 0) {
        throw VcfReaderError(chrom + "  is not in file " + file_path_);
      }
      auto tid = tbx_name2id(idx, chrom.c_str());
      if (tid <= 0) {
        throw VcfReaderError(chrom + "  is not in file " + file_path_);
      }
      return tid;
    }

    void query(std::string const& chrom, int64_t start, int64_t end) {
      auto tid = check_query(chrom);  // may throw error
      itr = tbx_itr_queryi(idx, tid, start, end);
      while (tbx_itr_next(fp, idx, itr, &kstr) >= 0) {
        vcf_parse1(&kstr, hdr, record);
        print_record();
      }
    }
  };

  VcfReader::VcfReader(std::string const& file_path) : pimpl{std::make_unique<impl>(file_path)} {}

  auto VcfReader::operator=(VcfReader&&) noexcept -> VcfReader& = default;
  VcfReader::VcfReader(VcfReader&&) noexcept = default;
  VcfReader::~VcfReader() noexcept = default;

  // status
  void VcfReader::open(const std::string& file_path) { pimpl->open(file_path); }
  auto VcfReader::get_file_path() const -> const std::string& { return pimpl->get_file_path(); }
  auto VcfReader::is_open() const -> bool { return pimpl->fp != nullptr; }
  auto VcfReader::is_closed() const -> bool { return pimpl->fp == nullptr; }
  auto VcfReader::has_index() const -> bool { return pimpl->idx != nullptr; }

  // getters
  auto VcfReader::get_chrom() const -> std::string { return pimpl->get_chrom(); }
  auto VcfReader::get_pos() const -> int64_t { return pimpl->get_pos(); }
  auto VcfReader::get_info_string(std::string const& key) const -> std::string {
    return pimpl->get_info_string(key);
  }

  auto VcfReader::get_info_int(std::string const& key) const -> int32_t {
    return pimpl->get_info_int(key);
  }

  // iterator
  auto VcfReader::next_record() -> int { return pimpl->next_record(); }
  auto VcfReader::get_rlen() const -> int64_t { return pimpl->get_rlen(); }
  void VcfReader::check_record() const { pimpl->check_record(); }
  void VcfReader::print_record() const { pimpl->print_record(); }

  void VcfReader::query(const std::string& chrom, int64_t start, int64_t end) {
    return pimpl->query(chrom, start, end);
  }
  auto VcfReader::get_header() const -> bcf_hdr_t* { return pimpl->hdr; }
  auto VcfReader::get_record() const -> bcf1_t* { return pimpl->record; }

}  // namespace sv2nl