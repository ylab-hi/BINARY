//
// Created by li002252 on 8/13/22.
//

#ifndef BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
#define BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
#include <htslib/tbx.h>
#include <htslib/vcf.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <binary/concepts.hpp>
#include <binary/exception.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

namespace binary::parser::vcf {
  using pos_t = std::uint32_t;
  using chrom_t = std::string;
  class BaseVcfRecord;

  namespace details {

    template <typename T> constexpr void bcf_deleter(T* record) noexcept {
      if constexpr (std::same_as<T, bcf1_t>) {
        bcf_destroy(record);
      } else if constexpr (std::same_as<T, bcf_hdr_t>) {
        bcf_hdr_destroy(record);
      } else if constexpr (std::same_as<T, htsFile>) {
        hts_close(record);
      } else if constexpr (std::same_as<T, hts_itr_t>) {
        hts_itr_destroy(record);
      } else if constexpr (std::same_as<T, tbx_t>) {
        tbx_destroy(record);
      } else if constexpr (std::same_as<T, kstring_t>) {
        free(record->s);
        delete record;
      } else {
        delete record;
      }
    }

    template <typename T> auto get_deleter() {
      if constexpr (std::same_as<T, kstring_t>) {
        return std::unique_ptr<T, decltype(&bcf_deleter<T>)>{new kstring_t{}, &bcf_deleter<T>};
      } else {
        return std::unique_ptr<T, decltype(&bcf_deleter<T>)>{nullptr, &bcf_deleter<T>};
      }
    }

    // #define BCF_HT_FLAG 0  header type
    // #define BCF_HT_INT  1
    // #define BCF_HT_REAL 2
    // #define BCF_HT_STR  3
    // #define BCF_HT_LONG (BCF_HT_INT | 0x100)  BCF_HT_INT, but for int64_t values; VCF only!

    constexpr int BINARY_BCF_HT_DEFAULT = -1;
    constexpr int BINARY_BCF_HT_FLAG = 0;
    constexpr int BINARY_BCF_HT_INT = 1;
    constexpr int BINARY_BCF_HT_REAL = 2;
    constexpr int BINARY_BCF_HT_STR = 3;
    constexpr int BINARY_BCF_HT_LONG = (BINARY_BCF_HT_INT | 0x100);

    template <typename Datatype>
    requires binary::concepts::IsAnyOf<Datatype, int, float, char, pos_t, int64_t>
    struct InfoField {
      Datatype* data{nullptr};
      int32_t count{};
      int data_id{BINARY_BCF_HT_DEFAULT};

      constexpr InfoField() {
        if constexpr (std::same_as<Datatype, pos_t>) {
          data_id = BINARY_BCF_HT_INT;
        } else if constexpr (std::same_as<Datatype, float>) {
          data_id = BINARY_BCF_HT_REAL;
        } else if constexpr (std::same_as<Datatype, char>) {
          data_id = BINARY_BCF_HT_STR;
        } else if constexpr (std::same_as<Datatype, int64_t>) {
          data_id = BINARY_BCF_HT_LONG;
        } else {
          data_id = BINARY_BCF_HT_DEFAULT;
        }
      };

      InfoField(InfoField const&) = delete;
      InfoField(InfoField&&) = delete;
      InfoField& operator=(InfoField const&) = delete;
      InfoField& operator=(InfoField&&) = delete;

      constexpr ~InfoField() { free(data); }

      constexpr auto result() {
        if constexpr (std::same_as<Datatype, char>) {
          return std::string(data, count - 1);
        } else {
          return *data;
        }
      }
    };

    template <typename DataType>
    constexpr auto get_info_field(std::string_view key, const bcf_hdr_t* hdr, bcf1_t* record) {
      auto info_field = InfoField<DataType>{};

      if (int ret = bcf_get_info_values(hdr, record, key.data(), (void**)(&info_field.data),
                                        &info_field.count, info_field.data_id);
          ret < 0) {
        throw VcfReaderError("Failed to get info " + std::string(key));
      }

      return info_field.result();
    }

    struct DataImpl {
      constexpr DataImpl() = default;
      explicit DataImpl(std::string_view file)
          : fp{hts_open(file.data(), "r"), &bcf_deleter<htsFile>},
            header{bcf_hdr_read(fp.get()), &bcf_deleter<bcf_hdr_t>},
            record{bcf_init1(), &bcf_deleter<bcf1_t>} {}

      DataImpl(DataImpl const&) = delete;
      auto operator=(DataImpl const&) -> DataImpl& = delete;
      DataImpl(DataImpl&&) noexcept = default;
      auto operator=(DataImpl&&) noexcept -> DataImpl& = default;

      decltype(get_deleter<htsFile>()) fp = get_deleter<htsFile>();
      decltype(get_deleter<bcf_hdr_t>()) header = get_deleter<bcf_hdr_t>();
      decltype(get_deleter<bcf1_t>()) record = get_deleter<bcf1_t>();
      decltype(get_deleter<hts_itr_t>()) itr_ptr = get_deleter<hts_itr_t>();
      decltype(get_deleter<tbx_t>()) idx_ptr = get_deleter<tbx_t>();
      decltype(get_deleter<kstring_t>()) ks_ptr = get_deleter<kstring_t>();
    };
  }  // namespace details

  class BaseVcfRecord {
  public:
    constexpr BaseVcfRecord() = default;
    explicit BaseVcfRecord(std::shared_ptr<details::DataImpl> const& data)
        : data_{data}, eof_{false} {
      next();
    }

    BaseVcfRecord(BaseVcfRecord const&) = default;
    BaseVcfRecord(BaseVcfRecord&&) noexcept = default;
    BaseVcfRecord& operator=(BaseVcfRecord const&) = default;
    BaseVcfRecord& operator=(BaseVcfRecord&&) noexcept = default;

    constexpr void set_eof() { eof_ = true; }

    void init_info(std::shared_ptr<details::DataImpl> const&) const {};

    virtual ~BaseVcfRecord() = default;

    void next() {
      if (auto data = data_.lock()) {
        if (int ret = bcf_read(data->fp.get(), data->header.get(), data->record.get()); ret < -1) {
          throw VcfReaderError("Failed to read line in vcf ");
        } else if (ret == -1) {
          set_eof();
        } else {
          update(data);
        }
      } else {
        throw VcfReaderError("Using dangling VcfRecord");
      }
    }

    friend auto operator<<(std::ostream& os, BaseVcfRecord const& record) -> std::ostream& {
      return os << "[BaseVcfRecord chrom: " << record.chrom << " pos: " << record.pos
                << " rlen: " << record.rlen << ']';
    }

    friend auto operator==(BaseVcfRecord const& lhs, BaseVcfRecord const& rhs) -> bool {
      return lhs.chrom == rhs.chrom && lhs.pos == rhs.pos && lhs.rlen == rhs.rlen;
    };

    std::weak_ptr<details::DataImpl> data_{};
    bool eof_{true};  //  default constructor is true

    std::string chrom{};
    pos_t pos{};
    pos_t rlen{};

  protected:
    void update(std::shared_ptr<details::DataImpl> const& data) {
      chrom = bcf_seqname_safe(data->header.get(), data->record.get());
      pos = static_cast<pos_t>(data->record->pos);
      rlen = static_cast<pos_t>(data->record->rlen);
      init_info(data);
    }
  };

  class VcfRecord : public BaseVcfRecord {
  public:
    constexpr VcfRecord() = default;
    explicit VcfRecord(std::shared_ptr<details::DataImpl> const& data) : BaseVcfRecord(data) {
      init_info(data);
    }
    VcfRecord(VcfRecord const&) = default;
    VcfRecord(VcfRecord&&) noexcept = default;
    VcfRecord& operator=(VcfRecord const&) = default;
    VcfRecord& operator=(VcfRecord&&) noexcept = default;

    void init_info(std::shared_ptr<details::DataImpl> const& data) {
      svtype = details::get_info_field<char>("SVTYPE", data->header.get(), data->record.get());
      svend = details::get_info_field<pos_t>("SVEND", data->header.get(), data->record.get());
    }

    friend auto operator<<(std::ostream& os, VcfRecord const& record) -> std::ostream& {
      return os << "[VcfRecord chrom: " << record.chrom << " pos: " << record.pos
                << " rlen: " << record.rlen << " svtype: " << record.svtype
                << " svend: " << record.svend << ']';
    }

    friend auto operator==(VcfRecord const& lhs, VcfRecord const& rhs) -> bool {
      return lhs.chrom == rhs.chrom && lhs.pos == rhs.pos && lhs.rlen == rhs.rlen
             && lhs.svtype == rhs.svtype && lhs.svend == rhs.svend;
    };

    pos_t svend{};
    std::string svtype{};
  };

  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  class VcfRanges {
  public:
    explicit VcfRanges(std::string file_path);

    VcfRanges(VcfRanges const&) = delete;
    auto operator=(VcfRanges const&) -> VcfRanges& = delete;
    constexpr VcfRanges(VcfRanges&&) noexcept = default;
    constexpr auto operator=(VcfRanges&&) noexcept -> VcfRanges& = default;

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
      constexpr iterator() = default;
      explicit constexpr iterator(std::shared_ptr<details::DataImpl> const& data)
          : value_{std::make_unique<value_type>(data)} {}

      constexpr iterator(iterator const& other) { clone(other); };

      constexpr auto operator=(iterator const& other) -> iterator& {
        clone(other);
        return *this;
      };

      constexpr iterator(iterator&&) noexcept = default;
      constexpr auto operator=(iterator&&) noexcept -> iterator& = default;

      constexpr ~iterator() = default;

      // public member functions
      auto operator->() const -> pointer { return value_.get(); }
      auto operator*() const -> value_type { return *value_; }
      auto operator++() -> iterator& {
        value_->next();
        return *this;
      }

      // old and new iterator will change at same time
      auto operator++(int) -> iterator {
        auto copy = *this;
        ++(*this);
        return copy;
      }

      friend auto operator==(iterator const& lhs, iterator const& rhs) -> bool = default;
      friend auto operator==(iterator const& lhs, std::default_sentinel_t) -> bool {
        if (lhs.value_ == nullptr || lhs.value_->eof_) {
          return true;
        }
        return false;
      }

    private:
      void clone(iterator const& other) { value_ = std::make_unique<value_type>(*(other.value_)); }

      std::unique_ptr<value_type> value_{};
    };

    [[nodiscard]] constexpr auto file_path() const -> const std::string&;
    [[nodiscard]] constexpr auto has_index() const -> bool;

    constexpr auto iter_query_record() const -> VcfRanges::iterator;
    constexpr auto query(std::string const& chrom, pos_t start, pos_t end) const
        -> VcfRanges::iterator;

    constexpr auto begin() const -> iterator;
    [[nodiscard]] constexpr auto end() const -> std::default_sentinel_t;

    friend auto operator==(VcfRanges const& lhs, VcfRanges const& rhs) -> bool {
      if (lhs.file_path_ != rhs.file_path_) return false;
      if (lhs.fp != rhs.fp) return false;
      return true;
    }

  private:
    constexpr void seek() const;
    constexpr auto check_query(std::string_view chrom) const -> int;

    std::string file_path_{};
    mutable std::shared_ptr<details::DataImpl> pdata_{nullptr};
  };

  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord> VcfRanges<DataType>::VcfRanges(
      std::string file_path)
      : file_path_(std::move(file_path)) {}

  /**
   * @brief  get file path of the vcf file
   * @return  vcf file path
   */
  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr auto VcfRanges<DataType>::file_path() const -> const std::string& { return file_path_; }

  /**
   * @brief check if the vcf file has index
   * @return bool
   */
  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr auto VcfRanges<DataType>::has_index() const -> bool {
    return pdata_->idx_ptr != nullptr;
  }

  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr auto VcfRanges<DataType>::check_query(std::string_view chrom) const -> int {
    if (!has_index()) {
      pdata_->idx_ptr.reset(tbx_index_load(file_path_.c_str()));
      if (!pdata_->idx_ptr) {
        throw VcfReaderError("Failed to load index for " + file_path_);
      }
    }

    if (bcf_hdr_name2id(pdata_->header.get(), chrom.data()) < 0) {
      throw VcfReaderError(std::string(chrom) + " is not in the vcf file " + file_path_);
    }
    auto tid = tbx_name2id(pdata_->idx_ptr.get(), chrom.data());
    assert(tid > 0);
    return tid;
  }

  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr auto VcfRanges<DataType>::iter_query_record() const -> VcfRanges::iterator {
    if (int ret = tbx_itr_next(pdata_->fp.get(), pdata_->idx_ptr.get(), pdata_->itr_ptr.get(),
                               pdata_->ks_ptr.get());
        ret < -1) {
      throw VcfReaderError("Query-> Failed to query ");
    } else if (ret == -1) {
      return iterator{};
    }
    // no problem

    vcf_parse1(pdata_->ks_ptr.get(), pdata_->header.get(), pdata_->record.get());
    return iterator(pdata_);
  }

  /**
   * @brief  query the vcf file if has index
   * @param chrom chromosome name
   * @param start start position
   * @param end end position
   * @return vcf record
   */
  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr auto VcfRanges<DataType>::query(std::string const& chrom, pos_t start, pos_t end) const
      -> VcfRanges::iterator {
    seek();  // seek to the first record and initialize the iterator

    auto tid = check_query(chrom);  // may throw error
    pdata_->itr_ptr.reset(tbx_itr_queryi(pdata_->idx_ptr.get(), tid, start, end));

    if (!pdata_->itr_ptr) {
      throw VcfReaderError("Query-> Failed to query " + chrom + ":" + std::to_string(start) + "-"
                           + std::to_string(end));
    }
    return iter_query_record();
  }

  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr auto VcfRanges<DataType>::begin() const -> VcfRanges::iterator {
    seek();
    return iterator{pdata_};
  }

  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr auto VcfRanges<DataType>::end() const -> std::default_sentinel_t {
    return std::default_sentinel;
  }
  template <typename DataType>
  requires std::derived_from<DataType, BaseVcfRecord>
  constexpr void VcfRanges<DataType>::seek() const {
    pdata_ = std::make_shared<details::DataImpl>(file_path_);
  }

}  // namespace binary::parser::vcf
#endif  // BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
