//
// Created by li002252 on 8/2/22.
//

#include "detect_dup.hpp"

#include "util.hpp"

void detect_dup(std::string const& vcf_path) {
  using namespace binary::parser;
  auto vcf_reader = VcfReader{vcf_path};

  assert(vcf_reader.is_open());
}
