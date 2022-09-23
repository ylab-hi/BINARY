//
// Created by li002252 on 8/1/22.
//

#include <binary/utils.hpp>

namespace binary::utils {

  auto check_file_path(std::initializer_list<std::string_view> file_paths) -> bool {
    for (auto const& file_path : file_paths) {
      if (!std::filesystem::is_regular_file(file_path)) {
        spdlog::error("{} does not exist", file_path);
        return false;
      }
    }
    return true;
  }

  [[maybe_unused]] auto check_file_path(std::string_view file_path) -> bool {
    return std::filesystem::exists(file_path);
  }

  std::string trim_str(std::string_view str) {
    auto temp = str | trim;
    return {temp.begin(), temp.end()};
  }

  auto split_str(std::string_view str, char delim) -> std::vector<std::string> {
    auto temp_view = str | std::ranges::views::split(delim);
    auto result = std::vector<std::string>{};

    for (auto const& sub_str : temp_view) {
      result.emplace_back(sub_str.begin(), sub_str.end());
    }
    return result;
  }

}  // namespace binary::utils