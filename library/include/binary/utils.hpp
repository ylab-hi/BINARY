//
// Created by li002252 on 8/1/22.
//
#ifndef BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_

#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ranges>

namespace binary::utils {

  // TODO: use binary search if range is sorted
  template <std::ranges::input_range Container, typename Value>
    requires std::equality_comparable_with<std::ranges::range_value_t<Container>, Value>
  auto index(Container const& container, Value&& value) -> std::size_t {
    auto it = std::ranges::find(container, std::forward<Value>(value));
    if (it == std::ranges::end(container)) {
      return -1;
    }
    return std::ranges::distance(std::ranges::begin(container), it);
  }

  auto check_file_path(std::initializer_list<std::string_view> file_paths) -> bool;
  [[maybe_unused]] auto check_file_path(std::string_view file_path) -> bool;

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

  template <std::ranges::input_range StringRange>
    requires std::convertible_to<std::ranges::range_value_t<StringRange>, std::string>
  void merge_files(StringRange&& files, std::string_view output_file, std::string_view header = "",
                   bool is_deleted = true, bool is_skipped_header = true) {
    auto output = std::ofstream(output_file.data());

    if (!header.empty()) {
      output << header << '\n';
    }

    for (auto const& file : files) {
      // check files exist
      if (std::filesystem::exists(file)) {
        auto input = std::ifstream(file);
        if (is_skipped_header) {
          std::string line;
          std::getline(input, line);
        }
        output << input.rdbuf();
        input.close();
        // remove original files
        if (is_deleted) {
          std::filesystem::remove(file);
        }
      }

      output.close();
    }
  }

  [[maybe_unused]] inline void set_debug() { spdlog::set_level(spdlog::level::debug); }
  [[maybe_unused]] inline void set_trace() { spdlog::set_level(spdlog::level::trace); }
  [[maybe_unused]] inline void set_info() { spdlog::set_level(spdlog::level::info); }

#ifdef __GNUC__
#  if __GNUC__ >= 12
  // trim from left
  inline constexpr auto trim_front = std::views::drop_while(::isspace);

  inline constexpr auto trim_back
      = std::views::reverse | std::views::drop_while(::isspace) | std::views::reverse;

  inline constexpr auto trim = trim_front | trim_back;

  auto split_str(std::string_view str, char delim) -> std::vector<std::string>;

  std::string trim_str(std::string_view str);
#  endif
#endif

}  // namespace binary::utils

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_
