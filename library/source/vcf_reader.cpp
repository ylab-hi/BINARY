//
// Created by li002252 on 5/19/22.
//

#include "sv2nl/vcf_reader.h"

#include "spdlog/spdlog.h"
#include "vcf.h"

namespace sv2nl {
  class VcfReader::impl {
  public:
    explicit impl(std::string const& file_path) : file_path_{file_path} { init(); }

    ~impl() {
      if (fp) {
        close();
      }
    }

    impl(impl const&) = delete;
    impl& operator=(impl const&) = delete;
    impl(impl&&) = default;
    impl& operator=(impl&&) = default;

    void init() const {
      fp = hts_open(file_path_.c_str(), "r");
      if (!fp) {
        throw std::invalid_argument("Failed to open file " + file_path_);
      }
      hdr = bcf_hdr_read(fp);
      if (!hdr) {
        throw std::invalid_argument("Failed to init bcf_hdr_read");
      }
      line = bcf_init();
      if (!line) {
        throw std::invalid_argument("Failed to init bcf_init");
      }
    }

    void open(std::string const& file_path) const {
      if (fp) {
        throw std::invalid_argument("File is already opened " + file_path_);
      }
      file_path_ = file_path;
      init();
    }

    void close() const {
      if (fp == nullptr) {
        spdlog::error("File is already closed");
      }

      bcf_hdr_destroy(hdr);
      hdr = nullptr;
      int ret = hts_close(fp);
      fp = nullptr;
      if (ret < 0) {
        spdlog::error("Failed to close file");
      }
      bcf_destroy(line);
      line = nullptr;
    }
    [[nodiscard]] const std::string& get_file_path() const { return file_path_; }

    mutable std::string file_path_{};
    mutable htsFile* fp{nullptr};
    mutable bcf_hdr_t* hdr{nullptr};
    mutable bcf1_t* line{nullptr};
  };

  VcfReader::VcfReader(std::string const& file_path) : pimpl{std::make_unique<impl>(file_path)} {
    pimpl->init();
  }

  VcfReader& VcfReader::operator=(VcfReader&&) noexcept = default;
  VcfReader::VcfReader(VcfReader&&) noexcept = default;

  void VcfReader::open(const std::string& file_path) const { pimpl->open(file_path); }

  void VcfReader::close() const { pimpl->close(); }
  const std::string& VcfReader::get_file_path() const { return pimpl->get_file_path(); }
  bool VcfReader::is_open() const { return pimpl->fp != nullptr; }
  bool VcfReader::is_closed() const { return pimpl->fp == nullptr; }
}  // namespace sv2nl
