//
// Created by li002252 on 9/1/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_HELPER_HPP_
#define BUILDALL_STANDALONE_SV2NL_HELPER_HPP_

#include <vcf_info.hpp>

namespace sv2nl {

  [[nodiscard]] bool is_contained(const Sv2nlVcfRecord& target, const Sv2nlVcfRecord& source);

  Sv2nlVcfRecord validate_record(const Sv2nlVcfRecord& record) noexcept;

  std::pair<std::string, std::string> get_2chroms(Sv2nlVcfRecord const& record);
  std::string get_chr2(Sv2nlVcfRecord const& record);
}  // namespace sv2nl
#endif  // BUILDALL_STANDALONE_SV2NL_HELPER_HPP_
