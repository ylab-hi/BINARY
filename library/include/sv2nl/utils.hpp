//
// Created by li002252 on 8/1/22.
//
#ifndef SV2NL_STANDALONE_INCLUDE_UTLS_HPP_
#define SV2NL_STANDALONE_INCLUDE_UTLS_HPP_

#include <spdlog/spdlog.h>

#include <filesystem>

namespace sv2nl::utils {
  auto check_file_path(std::initializer_list<std::string> const& file_paths) -> bool;
  [[maybe_unused]] auto check_file_path(std::string const& file_path) -> bool;
}  // namespace sv2nl::utils

#endif  // SV2NL_STANDALONE_INCLUDE_UTLS_HPP_
