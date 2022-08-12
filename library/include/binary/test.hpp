//
// Created by li002252 on 5/31/22.
//

#ifndef BINARY_INCLUDE_BINARY_TEST_H_
#define BINARY_INCLUDE_BINARY_TEST_H_
#include <string>

#include "htslib/vcf.h"

namespace binary {

  [[maybe_unused]] void read_tsv(std::string_view file_path);

  [[maybe_unused]] void read_vcf(std::string const& file_path);

  void test_vcf(const std::string& file_path);

}  // namespace binary
#endif  // BINARY_INCLUDE_BINARY_TEST_H_
