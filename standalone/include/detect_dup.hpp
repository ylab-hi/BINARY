//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_STANDALONE_INCLUDE_DETECT_DUP_HPP_
#define BINARY_STANDALONE_INCLUDE_DETECT_DUP_HPP_

#include <binary/csv.hpp>
#include <binary/parser/vcf.hpp>
#include <binary/types.hpp>
#include <concepts>
#include <tuple>

using binary::types::pos_t;

auto read_csv(std::string_view file_path) -> std::vector<std::tuple<std::string, pos_t, pos_t>>;

template <std::integral T> auto overlap(T /**unused**/, T p2, T p3, T /**unused**/) -> bool {
  if (p3 > p2) return false;
  return true;
}

template <std::integral T> auto enclose(T p1, T p2, T p3, T p4) -> bool {
  if (p1 < p3 && p2 > p4) return true;
  return false;
}

void detect_dup(std::string const& vcf_path, std::string const& csv_path);

#endif  // BINARY_STANDALONE_INCLUDE_DETECT_DUP_HPP_
