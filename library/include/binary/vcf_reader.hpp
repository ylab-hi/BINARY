#ifndef BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_VCF_READER_H_
#define BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_VCF_READER_H_
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>

#include <binary/exception.hpp>
#include <binary/info_field.hpp>
#include <binary/utils.hpp>
#include <binary/vcf_record.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include "htslib/tbx.h"
#include "htslib/vcf.h"

namespace binary {

  class VcfReader {
  private:
    struct impl;
    std::unique_ptr<impl> pimpl;

  public:
    using iterator = VcfRecord&;
    using const_iterator = VcfRecord const&;
    using value_type = VcfRecord;

    VcfReader() = delete;
    explicit VcfReader(const std::string& file_path);
    ~VcfReader() noexcept;

    VcfReader(VcfReader&&) noexcept;
    auto operator=(VcfReader&&) noexcept -> VcfReader&;

    [[nodiscard]] auto get_file_path() const -> const std::string&;
    [[nodiscard]] auto is_open() const -> bool;
    [[nodiscard]] auto is_closed() const -> bool;
    [[nodiscard]] auto has_index() const -> bool;
    void tell();

    auto begin() -> iterator;
    [[nodiscard]] auto begin() const -> const_iterator;
    [[nodiscard]] auto end() const -> value_type;
    auto query(const std::string& chrom, int64_t start, int64_t end) -> const_iterator;
    auto iter_query_record() -> const_iterator;
  };

}  // namespace binary

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_UTLS_HPP_VCF_READER_H_
