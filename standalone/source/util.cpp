//
// Created by li002252 on 8/3/22.
//

#include "util.hpp"

auto add_chr(const std::string& chrom) -> std::string { return "chr" + chrom; }

auto Timer::elapsed() const -> double {
  return std::chrono::duration_cast<second_>(clock_::now() - beg_).count();
}
