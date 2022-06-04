//
// Created by li002252 on 5/19/22.
//

#include "sv2nl/vcf_reader.h"

#include <utility>

#include "spdlog/spdlog.h"
#include "vcf.h"

namespace sv2nl {
  class VcfReader::impl {
  public:
    explicit impl(std::string file_path) : file_path_{std::move(file_path)} { init(); }

    ~impl() {
      if (fp) {
        close();
      }
    }
    impl(impl&&) = default;
    impl& operator=(impl&&) = default;

    void init() {
      fp = hts_open(file_path_.c_str(), "r");
      if (!fp) {
        throw std::runtime_error("Failed to open file " + file_path_);
      }
      hdr = bcf_hdr_read(fp);
      if (!hdr) {
        throw std::runtime_error("Failed to init bcf_hdr_read");
      }
      line = bcf_init();
      if (!line) {
        throw std::runtime_error("Failed to init bcf_init");
      }
    }

    void open(std::string file_path) {
      if (fp) {
        throw std::runtime_error("File is already opened " + file_path_);
      }
      file_path_ = std::move(file_path);
      init();
    }

    void close() {
      if (fp == nullptr) {
        throw std::runtime_error("File is already closed");
      }

      bcf_hdr_destroy(hdr);
      hdr = nullptr;
      int ret = hts_close(fp);
      fp = nullptr;
      if (ret < 0) {
        throw std::runtime_error("Failed to close file");
      }
      bcf_destroy(line);
      line = nullptr;
    }
    [[nodiscard]] const std::string& get_file_path() const { return file_path_; }

    std::string file_path_{};
    htsFile* fp{nullptr};
    bcf_hdr_t* hdr{nullptr};
    bcf1_t* line{nullptr};
  };

  VcfReader::VcfReader(std::string file_path)
      : pimpl{std::make_unique<impl>(std::move(file_path))} {
    pimpl->init();
  }

  VcfReader& VcfReader::operator=(VcfReader&&) noexcept = default;
  VcfReader::VcfReader(VcfReader&&) noexcept = default;
  VcfReader::~VcfReader() = default;

  void VcfReader::open(std::string file_path) { pimpl->open(std::move(file_path)); }

  void VcfReader::close() { pimpl->close(); }
  const std::string& VcfReader::get_file_path() const { return pimpl->get_file_path(); }
  bool VcfReader::is_open() const { return pimpl->fp != nullptr; }
  bool VcfReader::is_closed() const { return pimpl->fp == nullptr; }
}  // namespace sv2nl
