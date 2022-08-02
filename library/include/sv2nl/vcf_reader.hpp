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
#include <tuple>
#include <utility>

#include "htslib/tbx.h"
#include "htslib/vcf.h"

namespace sv2nl {
  void bcf_record_deleter(bcf1_t* record) noexcept;
  void bcf_hdr_deleter(bcf_hdr_t* hdr) noexcept;
  void bcf_hts_file_deleter(htsFile* hts_file) noexcept;
  void bcf_tbx_deleter(tbx_t* tbx) noexcept;
  void bcf_itr_deleter(hts_itr_t* itr) noexcept;
  void bcf_kstring_deleter(kstring_t* ks) noexcept;

  class VcfRecord;

  class VcfReader {
  private:
    struct impl;
    std::unique_ptr<impl> pimpl;

  public:
    VcfReader() = delete;
    explicit VcfReader(const std::string& file_path);
    ~VcfReader() noexcept;

    VcfReader(VcfReader&&) noexcept;
    auto operator=(VcfReader&&) noexcept -> VcfReader&;

    [[nodiscard]] auto get_file_path() const -> const std::string&;

    [[nodiscard]] auto is_open() const -> bool;
    [[nodiscard]] auto is_closed() const -> bool;
    [[nodiscard]] auto has_index() const -> bool;
    void check_record() const;

    [[nodiscard]] auto get_chrom() const -> std::string;
    [[nodiscard]] auto get_pos() const -> int64_t;
    [[nodiscard]] auto get_rlen() const -> int64_t;

    auto begin() -> VcfRecord&;
    [[nodiscard]] auto begin() const -> VcfRecord const&;
    [[nodiscard]] auto end() const -> VcfRecord;
    auto query(const std::string& chrom, int64_t start, int64_t end) -> VcfRecord const&;
    auto iter_query_record() -> VcfRecord const&;
  };

  // VCfRecord
  class VcfRecord {
  public:
    friend class VcfReader;
    friend auto operator<<(std::ostream& os, VcfRecord const& record) -> std::ostream&;

    template <typename DataType>
    friend auto get_info_field(const std::string& key, VcfRecord const& vcf_record) ->
        typename InfoField<DataType>::result_type;

    using iterator_category = std::forward_iterator_tag;
    using value_type = std::tuple<std::string, int64_t>;

    VcfRecord() = default;
    explicit VcfRecord(std::shared_ptr<htsFile> file)
        : file_{file}, header_{bcf_hdr_read(file_.lock().get()), bcf_hdr_deleter} {}

    VcfRecord(std::shared_ptr<htsFile> file, std::shared_ptr<bcf_hdr_t> header,
              std::shared_ptr<bcf1_t> record)
        : file_{file}, header_(std::move(header)), record_(std::move(record)) {}

    VcfRecord(VcfRecord const& other) = default;
    auto operator=(VcfRecord const&) -> VcfRecord& = default;
    VcfRecord(VcfRecord&&) noexcept = default;
    auto operator=(VcfRecord&&) noexcept -> VcfRecord& = default;

    void check_header() const;  // will throw if header is not set
    // WARNING: all functions are unchecked for header pay attentions
    [[nodiscard]] auto get_record() const -> bcf1_t*;
    [[nodiscard]] auto get_header() const -> bcf_hdr_t*;
    [[nodiscard]] auto get_chrom() const -> std::string;
    [[nodiscard]] auto get_pos() const -> int64_t;
    [[nodiscard]] auto get_rlen() const -> int64_t;
    [[nodiscard]] auto is_valid() const -> bool;
    void set_end_of_file();

    // overload operators
    friend auto operator==(VcfRecord const& lhs, VcfRecord const& rhs) -> bool;

    auto operator++() -> VcfRecord&;
    auto operator++(int) -> VcfRecord;
    auto operator*() const -> value_type;

  protected:
    std::weak_ptr<htsFile> file_{};  // may become weak pointer
    std::shared_ptr<bcf_hdr_t> header_{nullptr, bcf_hdr_deleter};
    std::shared_ptr<bcf1_t> record_{bcf_init(), bcf_record_deleter};
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

    if (int ret = bcf_get_info_values(vcf_record.get_header(), vcf_record.get_record(), key.c_str(),
                                      (void**)(&data), &count, data_id);
        ret < 0) {
      throw VcfReaderError("Failed to get info " + key);
    }

    typename InfoField<DataType>::result_type result = info_field.get_result(data, count);
    free(data);
    return result;
  }

  auto get_info_field_int32(const std::string& key, VcfRecord const& vcf_record) -> int32_t;
  auto get_info_field_string(const std::string& key, VcfRecord const& vcf_record) -> std::string;

}  // namespace sv2nl

#endif
