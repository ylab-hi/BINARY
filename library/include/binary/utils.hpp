//
// Created by li002252 on 8/1/22.
//
#ifndef BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_

#include <spdlog/spdlog.h>

#include <filesystem>

namespace binary::utils {

  auto check_file_path(std::initializer_list<std::string> const& file_paths) -> bool;
  [[maybe_unused]] auto check_file_path(std::string const& file_path) -> bool;

  /**
   * @brief print the values of a tuple
   * @tparam T the type of the tuple
   * @param tup the tuple to print
   */
  template <typename... T>
  [[maybe_unused]] constexpr void print_tuple(const std::tuple<T...>& tup) {
    (void)std::initializer_list<int>{
        (spdlog::trace("std::tuple values {} ", std::get<T>(tup)), 0)...};
  }

  [[maybe_unused]] inline void set_debug() { spdlog::set_level(spdlog::level::debug); }
  [[maybe_unused]] inline void set_trace() { spdlog::set_level(spdlog::level::trace); }
  [[maybe_unused]] inline void set_info() { spdlog::set_level(spdlog::level::info); }

}  // namespace binary::utils

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_
