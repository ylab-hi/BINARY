//
// Created by li002252 on 8/2/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_VCF_RECORD_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_VCF_RECORD_HPP_

#include <binary/exception.hpp>
#include <binary/info_field.hpp>
#include <binary/types.hpp>
#include <binary/utils.hpp>

namespace binary {

  using namespace types;

  class VcfRecord {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::tuple<std::string, pos_t>;

    friend auto operator<<(std::ostream& os, VcfRecord const& record) -> std::ostream&;

    template <typename DataType>
    friend auto get_info_field(const std::string& key, VcfRecord const& vcf_record) ->
        typename binary::InfoField<DataType>::result_type;

    VcfRecord() = default;
    explicit VcfRecord(std::shared_ptr<htsFile> file)
        : file_{file}, header_{bcf_hdr_read(file_.lock().get()), utils::bcf_hdr_deleter} {}

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
    [[nodiscard]] auto get_pos() const -> pos_t;
    [[nodiscard]] auto get_rlen() const -> pos_t;
    [[nodiscard]] auto is_valid() const -> bool;
    void set_end_of_file();

    // overload operators
    friend auto operator==(VcfRecord const& lhs, VcfRecord const& rhs) -> bool;
    auto operator++() -> VcfRecord&;
    auto operator++(int) -> VcfRecord;
    auto operator*() const -> value_type;

  private:
    std::weak_ptr<htsFile> file_{};  // may become weak pointer
    std::shared_ptr<bcf_hdr_t> header_{nullptr, utils::bcf_hdr_deleter};
    std::shared_ptr<bcf1_t> record_{bcf_init(), utils::bcf_record_deleter};
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

  auto get_info_field_int32(const std::string& key, VcfRecord const& vcf_record) -> pos_t;
  auto get_info_field_int32(std::initializer_list<std::string> keys, VcfRecord const& vcf_record)
      -> std::vector<pos_t>;
  auto get_info_field_string(const std::string& key, VcfRecord const& vcf_record) -> std::string;
  auto get_info_field_string(std::initializer_list<std::string> keys, VcfRecord const& vcf_record)
      -> std::vector<std::string>;

}  // namespace binary
#endif  // BINARY_LIBRARY_INCLUDE_BINARY_VCF_RECORD_HPP_
