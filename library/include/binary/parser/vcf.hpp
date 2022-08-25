//
// Created by li002252 on 8/13/22.
//

#ifndef BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
#define BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
#include <htslib/tbx.h>
#include <htslib/vcf.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <binary/algorithm/interval_tree.hpp>
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

    template <typename T> auto deleter() {
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
    struct InfoGetter {
      Datatype* data{nullptr};
      int32_t count{};
      int data_id{BINARY_BCF_HT_DEFAULT};

      constexpr InfoGetter() {
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

      InfoGetter(InfoGetter const&) = delete;
      InfoGetter(InfoGetter&&) = delete;
      InfoGetter& operator=(InfoGetter const&) = delete;
      InfoGetter& operator=(InfoGetter&&) = delete;

      constexpr ~InfoGetter() { free(data); }

      constexpr auto result() {
        if constexpr (std::same_as<Datatype, char>) {
          return std::string(data, count - 1);
        } else {
          return *data;
        }
      }

      [[maybe_unused]] static constexpr auto result_type() {
        if constexpr (std::same_as<Datatype, char>) {
          return std::string{};
        } else {
          return Datatype{};
        }
      }
    };

    template <typename DataType>
    constexpr auto get_info_field(std::string_view key, const bcf_hdr_t* hdr, bcf1_t* record) {
      auto info_field = InfoGetter<DataType>{};

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

      decltype(deleter<htsFile>()) fp = deleter<htsFile>();
      decltype(deleter<bcf_hdr_t>()) header = deleter<bcf_hdr_t>();
      decltype(deleter<bcf1_t>()) record = deleter<bcf1_t>();
      decltype(deleter<hts_itr_t>()) itr_ptr = deleter<hts_itr_t>();
      decltype(deleter<tbx_t>()) idx_ptr = deleter<tbx_t>();
      decltype(deleter<kstring_t>()) ks_ptr = deleter<kstring_t>();
    };

    struct [[maybe_unused]] BaseInfoField {
      constexpr BaseInfoField() = default;
      constexpr BaseInfoField(BaseInfoField const&) = default;
      constexpr BaseInfoField(BaseInfoField&&) = default;
      constexpr BaseInfoField& operator=(BaseInfoField const&) = default;
      constexpr BaseInfoField& operator=(BaseInfoField&&) = default;
      virtual ~BaseInfoField() = default;
      virtual void update(std::shared_ptr<details::DataImpl> const& data) = 0;
    };

    template <typename T>
    concept InfoFieldConcept = requires(T t) {
      requires std::semiregular<T>;
      requires std::movable<T>;
      requires std::derived_from<T, details::BaseInfoField>;
      t.update(std::shared_ptr<details::DataImpl>{});
      std::cout << t;
    };

    template <typename... T> struct InfoFieldFactory : public details::BaseInfoField {
      std::tuple<decltype(InfoGetter<T>::result_type())...> data_tuple{};
      std::array<std::string, sizeof...(T)> keys_array{};

      template <typename... U>
      requires(std::convertible_to<U, std::string>&&...) explicit InfoFieldFactory(U... args)
          : keys_array{args...} {
        static_assert(sizeof...(T) == sizeof...(U), "Number of keys and values do not match");
      }
      InfoFieldFactory(InfoFieldFactory const&) = default;
      InfoFieldFactory& operator=(InfoFieldFactory const&) = default;
      InfoFieldFactory(InfoFieldFactory&&) noexcept = default;
      InfoFieldFactory& operator=(InfoFieldFactory&&) noexcept = default;
      ~InfoFieldFactory() override = default;

      template <typename... U> void init_keys(U... keys) {
        static_assert(sizeof...(U) == sizeof...(T), "Number of keys and values do not match");
        keys_array = {keys...};
      }

      void update(std::shared_ptr<DataImpl> const& data) override {
        data_tuple = [&]<std::size_t... I>(std::index_sequence<I...>) {
          return std::tuple(
              get_info_field<T>(keys_array[I], data->header.get(), data->record.get())...);
        }
        (std::make_index_sequence<sizeof...(T)>{});
      }
    };

  }  // namespace details

  using details::InfoFieldConcept;

  struct [[maybe_unused]] InfoField : public details::BaseInfoField {
    std::string svtype{};
    pos_t svend{};

    constexpr InfoField() = default;
    InfoField(InfoField const&) = default;
    InfoField(InfoField&&) = default;
    InfoField& operator=(InfoField const&) = default;
    InfoField& operator=(InfoField&&) = default;
    ~InfoField() override = default;

    void update(std::shared_ptr<details::DataImpl> const& data) override {
      svtype = details::get_info_field<char>("SVTYPE", data->header.get(), data->record.get());
      svend = details::get_info_field<pos_t>("SVEND", data->header.get(), data->record.get());
    }

    friend auto operator<<(std::ostream& os, InfoField const& info) -> std::ostream& {
      os << "svtype: " << info.svtype << " svend: " << info.svend;
      return os;
    }
  };

  template <InfoFieldConcept InfoType> class BaseVcfRecord {
  public:
    using info_type = InfoType;

    constexpr BaseVcfRecord() = default;
    explicit BaseVcfRecord(std::shared_ptr<details::DataImpl> const& data)
        : data_{data}, eof_{false} {
      next();
    }

    BaseVcfRecord(BaseVcfRecord const& other) { clone(other); }
    BaseVcfRecord& operator=(BaseVcfRecord const& other) {
      clone(other);
      return *this;
    };

    BaseVcfRecord& operator=(BaseVcfRecord&&) noexcept = default;
    BaseVcfRecord(BaseVcfRecord&&) noexcept = default;

    ~BaseVcfRecord() = default;

    constexpr void set_eof() { eof_ = true; }

    template <typename... T> void init_info_keys(T&&... args) {
      info_data->init_keys(std::forward<T>(args)...);
    }
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
                << " rlen: " << record.rlen << "info: " << *record.info_data << "]";
    }

    friend auto operator==(BaseVcfRecord const& lhs, BaseVcfRecord const& rhs) -> bool {
      return lhs.chrom == rhs.chrom && lhs.pos == rhs.pos && lhs.rlen == rhs.rlen;
    };

    std::weak_ptr<details::DataImpl> data_{};
    bool eof_{true};  //  default constructor is true

    std::string chrom{};
    pos_t pos{};
    pos_t rlen{};
    std::unique_ptr<InfoType> info_data{std::make_unique<InfoType>()};

  protected:
    void clone(BaseVcfRecord const& other) noexcept {
      chrom = other.chrom;
      pos = other.pos;
      rlen = other.rlen;
      info_data = std::make_unique<InfoType>(*other.info_data);
    }

    void update(std::shared_ptr<details::DataImpl> const& data) {
      chrom = bcf_seqname_safe(data->header.get(), data->record.get());
      pos = static_cast<pos_t>(data->record->pos);
      rlen = static_cast<pos_t>(data->record->rlen);
      info_data->update(data);
    }
  };

  template <typename T>
  concept RecordConcept = requires(T t) {
    requires std::semiregular<T>;
    requires std::movable<T>;
    t.chrom;
    t.pos;
    t.rlen;
    t.info_data;
    std::cout << t;
  };

  template <RecordConcept RecordType> class VcfRanges {
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
      using value_type = std::remove_cv_t<RecordType>;
      using difference_type = std::ptrdiff_t;
      using pointer = const RecordType*;
      using reference = const RecordType&;

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

    /**
     * Get file path
     * @return vcf file path
     */
    [[nodiscard]] constexpr auto file_path() const -> const std::string&;

    /**
     * Check is vcf file has index
     * @return bool
     */
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

  template <RecordConcept RecordType> VcfRanges<RecordType>::VcfRanges(std::string file_path)
      : file_path_(std::move(file_path)) {}

  /**
   * @brief  get file path of the vcf file
   * @return  vcf file path
   */
  template <RecordConcept RecordType> constexpr auto VcfRanges<RecordType>::file_path() const
      -> const std::string& {
    return file_path_;
  }

  /**
   * @brief check if the vcf file has index
   * @return bool
   */
  template <RecordConcept RecordType> constexpr auto VcfRanges<RecordType>::has_index() const
      -> bool {
    return pdata_ == nullptr ? false : pdata_->idx_ptr != nullptr;
  }

  template <RecordConcept RecordType>
  constexpr auto VcfRanges<RecordType>::check_query(std::string_view chrom) const -> int {
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

  template <RecordConcept RecordType>
  constexpr auto VcfRanges<RecordType>::iter_query_record() const -> VcfRanges::iterator {
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
  template <RecordConcept RecordType>
  constexpr auto VcfRanges<RecordType>::query(std::string const& chrom, pos_t start,
                                              pos_t end) const -> VcfRanges::iterator {
    seek();  // seek to the first record and initialize the iterator

    auto tid = check_query(chrom);  // may throw error
    pdata_->itr_ptr.reset(tbx_itr_queryi(pdata_->idx_ptr.get(), tid, start, end));

    if (!pdata_->itr_ptr) {
      throw VcfReaderError("Query-> Failed to query " + chrom + ":" + std::to_string(start) + "-"
                           + std::to_string(end));
    }
    return iter_query_record();
  }

  template <RecordConcept RecordType> constexpr auto VcfRanges<RecordType>::begin() const
      -> VcfRanges::iterator {
    seek();
    return iterator{pdata_};
  }

  template <RecordConcept RecordType> constexpr auto VcfRanges<RecordType>::end() const
      -> std::default_sentinel_t {
    return std::default_sentinel;
  }
  template <RecordConcept RecordType> constexpr void VcfRanges<RecordType>::seek() const {
    pdata_ = std::make_shared<details::DataImpl>(file_path_);
  }

  /*
   * Data structure for Interval tree
   * A minimal example of how to use the Interval tree
   */

  using namespace binary::algorithm;
  // add vcfInterval
  template <RecordConcept RecordType> class BaseVcfInterval : public tree::UIntInterval {
  public:
    constexpr BaseVcfInterval() = default;
    explicit BaseVcfInterval(RecordType&& vcf_record) : record(std::move(vcf_record)) {
      low = record.pos;
      high = record.info_data->svend;
    }

    BaseVcfInterval(tree::UIntInterval::key_type low_, tree::UIntInterval::key_type high_,
                    RecordType&& vcf_record)
        : tree::UIntInterval(low_, high_), record(std::move(vcf_record)) {}

    using tree::UIntInterval::UIntInterval;

    BaseVcfInterval(BaseVcfInterval const& other) = default;
    BaseVcfInterval& operator=(BaseVcfInterval const& other) = default;
    BaseVcfInterval(BaseVcfInterval&& other) noexcept = default;
    BaseVcfInterval& operator=(BaseVcfInterval&& other) noexcept = default;
    ~BaseVcfInterval() override = default;

    friend std::ostream& operator<<(std::ostream& os, BaseVcfInterval const& vcf_interval) {
      os << "[VcfInterval: " << vcf_interval.low << "-" << vcf_interval.high << " "
         << vcf_interval.record << "]";
      return os;
    }

    RecordType record{};
  };

  /*
   * Template alias
   */
  using VcfRecord = BaseVcfRecord<InfoField>;
  using VcfInterval = BaseVcfInterval<VcfRecord>;
  using VcfIntervalNode = tree::IntervalNode<VcfInterval>;

  template <InfoFieldConcept InfoFieldType> using BaseVcfIntervalNode
      = tree::IntervalNode<BaseVcfInterval<BaseVcfRecord<InfoFieldType>>>;

  static_assert(std::same_as<BaseVcfIntervalNode<InfoField>, VcfIntervalNode>,
                "BaseVcfInterval<VcfRecord> should be equal to VcfInterval");

}  // namespace binary::parser::vcf
#endif  // BUILDALL_LIBRARY_INCLUDE_BINARY_PARSER_VCF_HPP_
