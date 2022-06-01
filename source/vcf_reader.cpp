//
// Created by li002252 on 5/19/22.
//

#include <sv2nl/vcf_reader.h>

namespace sv2nl {

  void VcfReader::init() {
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
  void VcfReader::open(std::string file_path) {
    if (fp) {
      spdlog::warn("VcfReader cannot open file before {} closed", file_path_);
    }
    file_path_ = std::move(file_path);
    init();
  }

  void VcfReader::close() {
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

}  // namespace sv2nl
