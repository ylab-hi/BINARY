//
// Created by li002252 on 8/1/22.
//
#ifndef BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_

#include <htslib/tbx.h>
#include <htslib/vcf.h>
#include <spdlog/spdlog.h>

#include <filesystem>

namespace binary::utils {

  auto check_file_path(std::initializer_list<std::string> const& file_paths) -> bool;
  [[maybe_unused]] auto check_file_path(std::string const& file_path) -> bool;

  void bcf_record_deleter(bcf1_t* record) noexcept;
  void bcf_hdr_deleter(bcf_hdr_t* hdr) noexcept;
  void bcf_hts_file_deleter(htsFile* hts_file) noexcept;
  void bcf_tbx_deleter(tbx_t* tbx) noexcept;
  void bcf_itr_deleter(hts_itr_t* itr) noexcept;

  //  const auto bcf_tbx_deleter = [](tbx_t* tbx) noexcept {
  //    if (tbx) tbx_destroy(tbx);
  //  };
  //  const auto bcf_itr_deleter = [](hts_itr_t* itr) noexcept {
  //    if (itr) hts_itr_destroy(itr);
  //  };

}  // namespace binary::utils

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_