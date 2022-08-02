//
// Created by li002252 on 8/1/22.
//

#include <sv2nl/utils.hpp>

namespace sv2nl::utils {

  auto check_file_path(std::initializer_list<std::string> const& file_paths) -> bool {
    for (auto const& file_path : file_paths) {
      if (!std::filesystem::is_regular_file(file_path)) {
        spdlog::error("{} does not exist", file_path);
        return false;
      }
    }
    return true;
  }

  [[maybe_unused]] auto check_file_path(std::string const& file_path) -> bool {
    return std::filesystem::exists(file_path);
  }

}  // namespace sv2nl::utils