//
// Created by li002252 on 9/1/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_HELPER_HPP_
#define BUILDALL_STANDALONE_SV2NL_HELPER_HPP_

#include "vcf_info.hpp"

namespace sv2nl {

  /**
   * Check if target  contains source
   * @return bool
   */
  inline bool is_contained(const Sv2nlVcfRecord& target, const Sv2nlVcfRecord& source) {
    spdlog::debug("[is contained] target {}-{} source {}-{}", target.pos, target.info->svend,
                  source.pos, source.info->svend);

    assert(target.pos <= target.info->svend);
    assert(source.pos <= source.info->svend);

    return target.pos <= source.pos && target.info->svend >= source.info->svend;
  }

  /**
   * Make sure vcf record's pos >= svend in order to find overlaps in interval tree
   * @param record
   */
  inline Sv2nlVcfRecord validate_record(const Sv2nlVcfRecord& record) noexcept {
    auto res = record;
    if (res.pos > res.info->svend) std::swap(res.pos, res.info->svend);
    return res;
  }

  inline std::string get_chr2(const Sv2nlVcfRecord& record) {
    assert(record.info->svtype == "TRA" || record.info->svtype == "BND");
    return record.info->chr2;
  }

  inline std::pair<std::string, std::string> get_2chroms(const Sv2nlVcfRecord& record) {
    auto chr2 = get_chr2(record);
    return record.chrom > chr2 ? std::make_pair(chr2, record.chrom)
                               : std::make_pair(record.chrom, chr2);
  }

  inline std::string format_map_key(const Sv2nlVcfRecord& record) {
    if (record.info->svtype == "TRA" || record.info->svtype == "BND") {
      auto [chr1, chr2] = get_2chroms(record);
      return fmt::format("{}-{}-{}-{}", chr1, chr2, record.pos, record.info->svend);
    } else {
      return fmt::format("{}-{}-{}", record.chrom, record.pos, record.info->svend);
    }
  }

}  // namespace sv2nl
#endif  // BUILDALL_STANDALONE_SV2NL_HELPER_HPP_
