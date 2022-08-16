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
#include <binary/parser/info_field.hpp>
#include <binary/utils.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

namespace binary::parser {

  // TODO: create a impl to hold vcf record data in every iterator

  using namespace types;
  class VcfRecord {
  public:
    VcfRecord() = default;
    explicit VcfRecord(std::shared_ptr<htsFile> const& file);

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
    auto operator*() const -> std::tuple<std::string, pos_t>;

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
    mutable std::shared_ptr<htsFile> fp{nullptr, utils::bcf_hts_file_deleter};

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

  namespace experimental {
    template <typename DataType = VcfRecord> class VcfRanges {
    public:
      explicit VcfRanges(std::string file_path);

      VcfRanges(VcfRanges const&) = delete;
      auto operator=(VcfRanges const&) -> VcfRanges& = delete;
      VcfRanges(VcfRanges&&) noexcept = default;
      auto operator=(VcfRanges&&) noexcept -> VcfRanges& = default;

      class iterator {
      public:
        friend class VcfRanges;
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::remove_cv_t<DataType>;
        using difference_type = std::ptrdiff_t;
        using pointer = const DataType*;
        using reference = const DataType&;

        // constructors
        iterator() = default;
        explicit iterator(std::shared_ptr<htsFile> const& file);
        explicit iterator(value_type value) : value_{std::move(value)} {}

        // public member functions
        auto operator*() const -> value_type { return value_; }
        auto operator++() -> iterator& {
          if (int ret = bcf_read(value_.fp(), value_.header(), value_.record()); ret < -1) {
            throw VcfReaderError("Failed to read line in vcf ");
          } else if (ret == -1) {
            value_.set_eof();
          }
          return *this;
        }

        // old and new iterator will change at same time
        auto operator++(int) -> iterator {
          auto copy = *this;
          ++(*this);
          return copy;
        }

        friend auto operator==(iterator const& lhs, iterator const& rhs) -> bool = default;

      private:
        value_type value_{};
      };

      [[nodiscard]] auto file_path() const -> const std::string&;
      [[nodiscard]] auto is_open() const -> bool;
      [[nodiscard]] auto has_index() const -> bool;

      auto iter_query_record() -> VcfRanges::iterator&;
      auto query(std::string const& chrom, pos_t start, pos_t end) -> VcfRanges::iterator&;

      auto begin() -> iterator&;
      auto end() -> iterator&;
      auto begin() const -> iterator const&;
      auto end() const -> iterator const&;

      friend auto operator==(VcfRanges const& lhs, VcfRanges const& rhs) -> bool {
        if (lhs.file_path_ != rhs.file_path_) return false;
        if (lhs.fp != rhs.fp) return false;
        return true;
      }

    private:
      void seek() const;
      auto check_query(const std::string& chrom) -> int;

      std::string file_path_{};
      mutable std::shared_ptr<htsFile> fp{nullptr, utils::bcf_hts_file_deleter};

      mutable iterator current_{};
      iterator sentinel_{};
      std::unique_ptr<tbx_t, decltype(&utils::bcf_tbx_deleter)> idx{nullptr,
                                                                    &utils::bcf_tbx_deleter};
      std::unique_ptr<hts_itr_t, decltype(&utils::bcf_itr_deleter)> itr_ptr{
          nullptr, &utils::bcf_itr_deleter};
      std::unique_ptr<kstring_t, decltype(&utils::bcf_kstring_deleter)> ks_ptr{
          new kstring_t{}, &utils::bcf_kstring_deleter};
    };

    template <typename DataType> VcfRanges<DataType>::VcfRanges(std::string file_path)
        : file_path_(std::move(file_path)),
          fp{hts_open(file_path_.c_str(), "r"), utils::bcf_hts_file_deleter} {}

    /**
     * @brief  get file path of the vcf file
     * @return  vcf file path
     */
    template <typename DataType> auto VcfRanges<DataType>::file_path() const -> const std::string& {
      return file_path_;
    }

    /**
     * @brief  check if the vcf file is open
     * @return  vcf file header
     */
    template <typename DataType> auto VcfRanges<DataType>::is_open() const -> bool {
      return fp != nullptr;
    }

    /**
     * @brief check if the vcf file has index
     * @return bool
     */
    template <typename DataType> auto VcfRanges<DataType>::has_index() const -> bool {
      return idx != nullptr;
    }

    template <typename DataType> auto VcfRanges<DataType>::check_query(const std::string& chrom)
        -> int {
      if (!has_index()) {
        idx.reset(tbx_index_load(file_path_.c_str()));
        if (!idx) {
          throw VcfReaderError("Failed to load index for " + file_path_);
        }
      }

      if (bcf_hdr_name2id(current_.value_.header(), chrom.c_str()) < 0) {
        throw VcfReaderError(chrom + " is not in the vcf file " + file_path_);
      }
      auto tid = tbx_name2id(idx.get(), chrom.c_str());
      assert(tid > 0);
      return tid;
    }

    template <typename DataType> auto VcfRanges<DataType>::iter_query_record()
        -> VcfRanges::iterator& {
      if (int ret = tbx_itr_next(fp.get(), idx.get(), itr_ptr.get(), ks_ptr.get()); ret < -1) {
        throw VcfReaderError("Query-> Failed to query ");
      } else if (ret == -1) {
        return sentinel_;
      }
      // no problem
      vcf_parse1(ks_ptr.get(), current_.value_.header(), current_.value_.record());
      return current_;
    }

    /**
     * @brief  query the vcf file if has index
     * @param chrom chromosome name
     * @param start start position
     * @param end end position
     * @return vcf record
     */
    template <typename DataType>
    auto VcfRanges<DataType>::query(std::string const& chrom, pos_t start, pos_t end)
        -> VcfRanges::iterator& {
      seek();  // seek to the first record and initialize the iterator

      auto tid = check_query(chrom);  // may throw error
      itr_ptr.reset(tbx_itr_queryi(idx.get(), tid, start, end));

      if (!itr_ptr) {
        throw VcfReaderError("Query-> Failed to query " + chrom + ":" + std::to_string(start) + "-"
                             + std::to_string(end));
      }
      return iter_query_record();
    }

    template <typename DataType> void VcfRanges<DataType>::seek() const {
      fp.reset(hts_open(file_path_.c_str(), "r"), utils::bcf_hts_file_deleter);
      current_ = iterator{fp};
    }

    template <typename DataType> auto VcfRanges<DataType>::begin() -> VcfRanges::iterator& {
      seek();
      return ++current_;
    }

    template <typename DataType> auto VcfRanges<DataType>::begin() const
        -> const VcfRanges::iterator& {
      seek();
      return ++current_;
    }

    template <typename DataType> auto VcfRanges<DataType>::end() -> VcfRanges::iterator& {
      return sentinel_;
    }
    template <typename DataType> auto VcfRanges<DataType>::end() const
        -> const VcfRanges::iterator& {
      return sentinel_;
    }

    template <typename DataType>
    VcfRanges<DataType>::iterator::iterator(const std::shared_ptr<htsFile>& file) : value_{file} {}

  }  // namespace experimental

}  // namespace binary::parser
#endif  // BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
