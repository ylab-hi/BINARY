//
// Created by li002252 on 8/24/22.
//

#include "writer.hpp"

#include <spdlog/spdlog.h>

namespace sv2nl {

  void Writer::write_header() {
    if (header_.empty()) return;
    ofs_ << header_ << '\n';
  }

  void Writer::write(const std::string& line) { ofs_ << line << '\n'; }

  std::string Writer::get_keys(const Sv2nlVcfRecord& record) {
    return fmt::format("{}\t{}\t{}\t{}", record.chrom, record.pos, record.info->svend,
                       record.info->svtype);
  }
}  // namespace sv2nl