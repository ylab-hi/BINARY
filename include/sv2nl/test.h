//
// Created by li002252 on 5/31/22.
//

#ifndef SV2NL_INCLUDE_SV2NL_TEST_H_
#define SV2NL_INCLUDE_SV2NL_TEST_H_
#include <string>

[[maybe_unused]] void test();

[[maybe_unused]] void read_tsv(std::string_view file_path);

[[maybe_unused]] void read_vcf(std::string const& file_path);

#endif  // SV2NL_INCLUDE_SV2NL_TEST_H_
