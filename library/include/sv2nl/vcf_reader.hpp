#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <sv2nl/exception.hpp>
#include <sv2nl/info_field.hpp>
#include <sv2nl/utils.hpp>
#include <sv2nl/vcf_record.hpp>
#include <tuple>
#include <utility>

#include "htslib/tbx.h"
#include "htslib/vcf.h"

namespace sv2nl {

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

}  // namespace sv2nl

#endif
