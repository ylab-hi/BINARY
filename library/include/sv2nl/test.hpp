//
// Created by li002252 on 5/31/22.
//

#ifndef SV2NL_INCLUDE_SV2NL_TEST_H_
#define SV2NL_INCLUDE_SV2NL_TEST_H_
#include <string>

#include "htslib/vcf.h"

namespace sv2nl {

  [[maybe_unused]] void read_tsv(std::string_view file_path);

  [[maybe_unused]] void read_vcf(std::string const& file_path);

  void test_vcf(const std::string& file_path);
}  // namespace sv2nl
#endif  // SV2NL_INCLUDE_SV2NL_TEST_H_