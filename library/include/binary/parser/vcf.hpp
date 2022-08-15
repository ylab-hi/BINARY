//
// Created by li002252 on 8/13/22.
//

#ifndef BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
#define BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
#include <htslib/tbx.h>
#include <htslib/vcf.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>

#include <binary/exception.hpp>
#include <binary/info_field.hpp>
#include <binary/vcf_record.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

namespace binary::parser {

  using namespace types;

  class VcfRecord {
  public:
    VcfRecord() = default;
    explicit VcfRecord(std::shared_ptr<htsFile> const& file);

    // copy constructor and assignment operator
    VcfRecord(VcfRecord const& other) = default;
    auto operator=(VcfRecord const&) -> VcfRecord& = default;
    VcfRecord(VcfRecord&&) noexcept = default;
    auto operator=(VcfRecord&&) noexcept -> VcfRecord& = default;

    [[maybe_unused]] void check_header() const;  // will throw if header is not set
    // WARNING: all functions are unchecked for header pay attentions
    [[nodiscard]] auto fp() const -> htsFile*;
    [[nodiscard]] auto record() const -> bcf1_t*;
    [[nodiscard]] auto header() const -> bcf_hdr_t*;
    [[nodiscard]] auto chrom() const -> std::string;
    [[nodiscard]] auto pos() const -> pos_t;
    [[nodiscard]] auto rlen() const -> pos_t;
    [[nodiscard]] auto is_valid() const -> bool;
    void set_eof();

    // overload operators
    friend auto operator==(VcfRecord const& lhs, VcfRecord const& rhs) -> bool;
    friend auto operator<<(std::ostream& os, VcfRecord const& record) -> std::ostream&;

  private:
    std::weak_ptr<htsFile> fp_{};  // may become weak pointer
    std::shared_ptr<bcf_hdr_t> header_{nullptr, &utils::bcf_hdr_deleter};
    std::shared_ptr<bcf1_t> record_{nullptr, &utils::bcf_record_deleter};
  };

  class VcfReader {
  public:
    explicit VcfReader(std::string file_path);

    VcfReader(VcfReader const&) = delete;
    auto operator=(VcfReader const&) -> VcfReader& = delete;
    VcfReader(VcfReader&&) noexcept = default;
    auto operator=(VcfReader&&) noexcept -> VcfReader& = default;

    class iterator {
    public:
      friend class VcfReader;
      using iterator_concept = std::forward_iterator_tag;
      using iterator_category = std::forward_iterator_tag;
      using value_type = VcfRecord;
      using difference_type = std::ptrdiff_t;
      using pointer = VcfRecord*;
      using reference = VcfRecord&;

      // constructors
      iterator() = default;
      explicit iterator(std::shared_ptr<htsFile> const& file);
      iterator(iterator const& other) = default;
      auto operator=(iterator const& other) -> iterator& = default;
      iterator(iterator&& other) noexcept = default;
      auto operator=(iterator&& other) noexcept -> iterator& = default;

      // public member functions
      auto operator*() const -> value_type const&;
      auto operator*() -> value_type&;
      auto operator++() -> iterator&;

      friend auto operator==(iterator const& lhs, iterator const& rhs) -> bool {
        return lhs.value_ == rhs.value_;
      }

    private:
      value_type value_{};
    };

    [[nodiscard]] auto file_path() const -> const std::string&;
    [[nodiscard]] auto is_open() const -> bool;
    [[nodiscard]] auto has_index() const -> bool;

    auto iter_query_record() -> VcfReader::iterator&;
    auto query(std::string const& chrom, pos_t start, pos_t end) -> VcfReader::iterator&;

    auto begin() -> iterator&;
    auto end() -> iterator&;
    auto begin() const -> iterator const&;
    auto end() const -> iterator const&;

    friend auto operator==(VcfReader const& lhs, VcfReader const& rhs) -> bool;

  private:
    void seek() const;
    auto check_query(const std::string& chrom) -> int;

    std::string file_path_{};
    std::shared_ptr<htsFile> fp{nullptr, utils::bcf_hts_file_deleter};

    mutable iterator current_{};
    iterator sentinel_{};
    std::unique_ptr<tbx_t, decltype(&utils::bcf_tbx_deleter)> idx{nullptr, &utils::bcf_tbx_deleter};
    std::unique_ptr<hts_itr_t, decltype(&utils::bcf_itr_deleter)> itr_ptr{nullptr,
                                                                          &utils::bcf_itr_deleter};
    std::unique_ptr<kstring_t, decltype(&utils::bcf_kstring_deleter)> ks_ptr{
        new kstring_t{}, &utils::bcf_kstring_deleter};
  };

  template <typename DataType>
  auto get_info_field(const std::string& key, VcfRecord const& vcf_record) ->
      typename InfoField<DataType>::result_type {
    /**
  #define BCF_HT_FLAG 0  header type
  #define BCF_HT_INT  1
  #define BCF_HT_REAL 2
  #define BCF_HT_STR  3
  #define BCF_HT_LONG (BCF_HT_INT | 0x100)  BCF_HT_INT, but for int64_t values; VCF only!
  **/

    auto info_field = InfoField<DataType>{};
    auto data_id = info_field.data_id;

    DataType* data{nullptr};
    int32_t count{};

    if (int ret = bcf_get_info_values(vcf_record.header(), vcf_record.record(), key.c_str(),
                                      (void**)(&data), &count, data_id);
        ret < 0) {
      throw VcfReaderError("Failed to get info " + key);
    }

    typename InfoField<DataType>::result_type result = info_field.get_result(data, count);
    free(data);
    return result;
  }

  auto get_info_field_int32(const std::string& key, VcfRecord const& vcf_record) -> pos_t;
  auto get_info_field_int32(std::initializer_list<std::string> keys, VcfRecord const& vcf_record)
      -> std::vector<pos_t>;
  auto get_info_field_string(const std::string& key, VcfRecord const& vcf_record) -> std::string;
  auto get_info_field_string(std::initializer_list<std::string> keys, VcfRecord const& vcf_record)
      -> std::vector<std::string>;

}  // namespace binary::parser
#endif  // BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
