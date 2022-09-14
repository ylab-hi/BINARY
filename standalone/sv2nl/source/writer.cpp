//
// Created by li002252 on 8/24/22.
//

#include "writer.hpp"

#include <spdlog/spdlog.h>

namespace sv2nl {

  void Writer::write_header() {
    if (header_.empty()) return;
    std::lock_guard lock{mutex_};
    ofs_ << header_ << '\n';
  }
  void Writer::write(const std::string& line) {
    std::lock_guard lock{mutex_};
    ofs_ << line << '\n';
  }

  std::string Writer::format_keys(const Sv2nlVcfRecord& record) {
    if (record.info->svtype == "TRA" || record.info->svtype == "BND") {
      return fmt::format("{},{}\t{}\t{}\t{}", record.chrom, record.info->chr2, record.pos + 1,
                         record.info->svend, record.info->svtype);
    }
    return fmt::format("{}\t{}\t{}\t{}", record.chrom, record.pos + 1, record.info->svend,
                       record.info->svtype);
  }  // namespace v2
}  // namespace sv2nl