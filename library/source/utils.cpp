//
// Created by li002252 on 8/1/22.
//

#include <binary/utils.hpp>

namespace binary::utils {

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

  void bcf_record_deleter(bcf1_t* record) noexcept { bcf_destroy(record); }
  void bcf_hdr_deleter(bcf_hdr_t* hdr) noexcept { bcf_hdr_destroy(hdr); }
  void bcf_hts_file_deleter(htsFile* hts_file) noexcept { hts_close(hts_file); }
  void bcf_tbx_deleter(tbx_t* tbx) noexcept { tbx_destroy(tbx); }
  void bcf_itr_deleter(hts_itr_t* itr) noexcept { hts_itr_destroy(itr); }

}  // namespace binary::utils