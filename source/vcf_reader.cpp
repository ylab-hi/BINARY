//
// Created by li002252 on 5/19/22.
//

#include <sv2nl/vcf_reader.h>

#include <utility>

#include "spdlog/spdlog.h"
#include "vcf.h"
namespace sv2nl {
  struct VcfReader::impl {
    explicit impl(std::string file_path) : file_path_{std::move(file_path)} { init(); }

    ~impl() {
      if (fp) {
        close();
      }
    }

    void init() {
      fp = hts_open(file_path_.c_str(), "r");
      if (!fp) {
        spdlog::error("Failed to open {}", file_path_);
        std::exit(1);
      }
      hdr = bcf_hdr_read(fp);
      if (!hdr) {
        spdlog::error("Failed to init bcf_hdr_read");
        std::exit(1);
      }
      line = bcf_init();
      if (!line) {
        spdlog::error("Failed to init bcf_init");
        std::exit(1);
      }
    }

    void open(std::string file_path) {
      if (fp) {
        spdlog::warn("VcfReader cannot open {} before {} closed", file_path, file_path_);
      }
      file_path_ = std::move(file_path);
      init();
    }

    void close() {
      if (fp == nullptr) {
        spdlog::warn("VcfReader already closed");
      }

      bcf_hdr_destroy(hdr);
      hdr = nullptr;
      int ret = hts_close(fp);
      fp = nullptr;
      if (ret < 0) {
        spdlog::error("hts_close() failed");
      }
      bcf_destroy(line);
    }

    std::string file_path_;
    htsFile* fp{nullptr};
    bcf_hdr_t* hdr{nullptr};
    bcf1_t* line{nullptr};
  };

  VcfReader::VcfReader(std::string file_path)
      : pimpl(std::make_unique<impl>(std::move(file_path))) {
    pimpl->init();
  }

  void VcfReader::open(std::string file_path) { pimpl->open(std::move(file_path)); }

  void VcfReader::close() { pimpl->close(); }

  VcfReader::~VcfReader() = default;

}  // namespace sv2nl
