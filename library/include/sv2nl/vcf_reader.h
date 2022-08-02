#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>
#include <sv2nl/exception.h>
#include <sv2nl/info_field.h>

#include <memory>
#include <string>
#include <utility>

#include "htslib/tbx.h"
#include "htslib/vcf.h"

namespace sv2nl {

  class VcfReader {
  private:
    struct impl;
    std::unique_ptr<impl> pimpl;

    [[maybe_unused]] void open(const std::string& file_path);

    template <typename DataType>
    friend auto get_info_field(const std::string& key, VcfReader const& vcf_reader) ->
        typename InfoField<DataType>::result_type;

    [[nodiscard]] auto get_header() const -> bcf_hdr_t*;
    [[nodiscard]] auto get_record() const -> bcf1_t*;

  public:
    VcfReader() = delete;
    explicit VcfReader(std::string const& file_path);
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
    [[nodiscard]] auto get_info_int(const std::string& key) const -> int32_t;
    [[nodiscard]] auto get_info_string(const std::string& key) const -> std::string;

    auto next_record() -> int;
    void print_record() const;

    void query(const std::string& chrom, int64_t start, int64_t end);
  };

  template <typename DataType>
  auto get_info_field(const std::string& key, VcfReader const& vcf_reader) ->
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

    if (int ret = bcf_get_info_values(vcf_reader.get_header(), vcf_reader.get_record(), key.c_str(),
                                      (void**)(&data), &count, data_id);
        ret < 0) {
      throw VcfReaderError("Failed to get info " + key);
    }

    typename InfoField<DataType>::result_type result = info_field.get_result(data, count);
    free(data);
    return result;
  }

}  // namespace sv2nl

#endif
