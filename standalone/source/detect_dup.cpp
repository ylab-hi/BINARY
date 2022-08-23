//
// Created by li002252 on 8/2/22.
//

#include "detect_dup.hpp"

#include "util.hpp"

void detect_dup(std::string const& vcf_path) {
  using namespace binary::parser::vcf;
  auto vcf_reader = VcfRanges<VcfRecord>{vcf_path};
}
