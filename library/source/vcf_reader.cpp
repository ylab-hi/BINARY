//
// Created by li002252 on 5/19/22.
//

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>
#include <sv2nl/exception.h>
#include <sv2nl/vcf_reader.h>

#include "tbx.h"
#include "vcf.h"

namespace sv2nl {

  struct VcfReader::impl {
    std::string file_path_{};
    htsFile* fp{nullptr};
    bcf_hdr_t* hdr{nullptr};
    bcf1_t* record{nullptr};
    // late load
    tbx_t* idx{nullptr};
    kstring_t str{};
    hts_itr_t* itr{nullptr};

    explicit impl(std::string const& file_path) : file_path_{file_path} { init(); }

    ~impl() {
      if (fp) {
        destroy();
      }
    }

    impl(impl const&) = delete;
    impl& operator=(impl const&) = delete;
    impl(impl&&) = default;
    impl& operator=(impl&&) = default;

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
      free(str.s);

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

    [[nodiscard]] const std::string& get_file_path() const { return file_path_; }

    void check_record() const {
      if (!record) {
        throw VcfReaderError("Record is null");
      }
    }

    [[nodiscard]] std::string get_chrom() const { return bcf_seqname_safe(hdr, record); }
    [[nodiscard]] int64_t get_pos() const { return record->pos; }
    [[nodiscard]] int64_t get_rlen() const { return record->rlen; }

    [[nodiscard]] std::string get_info_string(std::string const& key) const {
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
    [[nodiscard]] int32_t get_info_int(std::string const& key) const {
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

    [[nodiscard]] int next_record() const {
      // 0 on success; -1 on end of file; < -1 on critical error

      int ret = bcf_read(fp, hdr, record);
      if (ret < -1) {
        throw VcfReaderError("Failed to read line in vcf " + file_path_);
      }
      return ret;
    }

    void query(std::string const& chrom, int64_t start, int64_t end) {
      idx = tbx_index_load(file_path_.c_str());
      if (!idx) {
        throw VcfReaderError("Query-> Failed to load index for vcf " + file_path_);
      }

      if (bcf_hdr_name2id(hdr, chrom.c_str()) < 0) {
        spdlog::error("{} not found in header", chrom);
        return;
      }

      auto tid = tbx_name2id(idx, chrom.c_str());

      if (tid > 0) {
        itr = tbx_itr_queryi(idx, tid, start, end);
        while (tbx_itr_next(fp, idx, itr, &str) >= 0) {
          vcf_parse1(&str, hdr, record);
          print_record();
        }
      }
    }
  };

  VcfReader::VcfReader(std::string const& file_path) : pimpl{std::make_unique<impl>(file_path)} {}

  VcfReader& VcfReader::operator=(VcfReader&&) noexcept = default;
  VcfReader::VcfReader(VcfReader&&) noexcept = default;
  VcfReader::~VcfReader() noexcept = default;

  // status
  void VcfReader::open(const std::string& file_path) { pimpl->open(file_path); }
  const std::string& VcfReader::get_file_path() const { return pimpl->get_file_path(); }
  bool VcfReader::is_open() const { return pimpl->fp != nullptr; }
  bool VcfReader::is_closed() const { return pimpl->fp == nullptr; }
  bool VcfReader::has_index() const { return pimpl->idx != nullptr; }

  // getters
  std::string VcfReader::get_chrom() const { return pimpl->get_chrom(); }
  int64_t VcfReader::get_pos() const { return pimpl->get_pos(); }
  std::string VcfReader::get_info_string(std::string const& key) const {
    return pimpl->get_info_string(key);
  }
  int32_t VcfReader::get_info_int(std::string const& key) const { return pimpl->get_info_int(key); }

  // iterator
  int VcfReader::next_record() { return pimpl->next_record(); }
  int64_t VcfReader::get_rlen() const { return pimpl->get_rlen(); }
  void VcfReader::check_record() const { pimpl->check_record(); }
  void VcfReader::print_record() const { pimpl->print_record(); }

  void VcfReader::query(const std::string& chrom, int64_t start, int64_t end) {
    return pimpl->query(chrom, start, end);
  }

}  // namespace sv2nl