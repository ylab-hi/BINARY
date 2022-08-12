//
// Created by li002252 on 5/19/22.
//

#include <binary/vcf_reader.hpp>

namespace binary {

  struct VcfReader::impl {
    std::string file_path_{};
    std::shared_ptr<htsFile> fp{nullptr, utils::bcf_hts_file_deleter};
    mutable VcfRecord record{};
    VcfRecord end_record{};

    // late load
    std::unique_ptr<tbx_t, decltype(&utils::bcf_tbx_deleter)> idx{nullptr, &utils::bcf_tbx_deleter};
    std::unique_ptr<hts_itr_t, decltype(&utils::bcf_itr_deleter)> itr_ptr{nullptr,
                                                                          &utils::bcf_itr_deleter};
    kstring_t ks{};

    explicit impl(std::string file_path)
        : file_path_{std::move(file_path)},
          fp{hts_open(file_path_.c_str(), "r"), utils::bcf_hts_file_deleter},
          record{fp},
          end_record(record) {
      end_record.set_end_of_file();
    }

    impl(impl const&) = delete;
    auto operator=(impl const&) -> impl& = delete;
    impl(impl&&) = default;
    auto operator=(impl&&) -> impl& = default;

    ~impl() { free(ks.s); }

    [[nodiscard]] auto get_file_path() const -> const std::string& { return file_path_; }

    void check_record() const {
      if (!record.is_valid()) {
        throw VcfReaderError("Record is null");
      }
    }

    auto begin() -> VcfRecord& { return ++record; }
    auto end() const -> VcfRecord { return end_record; }

    auto check_query(std::string const& chrom) -> int {
      idx = {tbx_index_load(file_path_.c_str()), utils::bcf_tbx_deleter};

      if (!idx) {
        throw VcfReaderError("Query-> Failed to load index for vcf " + file_path_);
      }

      if (bcf_hdr_name2id(record.get_header(), chrom.c_str()) < 0) {
        throw VcfReaderError(chrom + "  is not in file " + file_path_);
      }
      auto tid = tbx_name2id(idx.get(), chrom.c_str());
      if (tid <= 0) {
        throw VcfReaderError(chrom + "  is not in file " + file_path_);
      }
      return tid;
    }

    auto iter_query_record() -> VcfRecord const& {
      int ret = tbx_itr_next(fp.get(), idx.get(), itr_ptr.get(), &ks);
      if (ret < -1) {
        throw VcfReaderError("Query-> Failed to query ");
      } else if (ret == -1) {
        return end_record;
      }
      // no problem
      vcf_parse1(&ks, record.get_header(), record.get_record());
      return record;
    }

    auto query(std::string const& chrom_, int64_t start_, int64_t end_) -> VcfRecord const& {
      auto tid = check_query(chrom_);  // may throw error
      itr_ptr = {tbx_itr_queryi(idx.get(), tid, start_, end_), utils::bcf_itr_deleter};
      if (!itr_ptr) {
        throw VcfReaderError("Query-> Failed to query " + chrom_ + ":" + std::to_string(start_)
                             + "-" + std::to_string(end_));
      }
      return iter_query_record();
    }
  };

  // VcfReader
  VcfReader::VcfReader(const std::string& file_path) : pimpl{std::make_unique<impl>(file_path)} {}

  auto VcfReader::operator=(VcfReader&&) noexcept -> VcfReader& = default;
  VcfReader::VcfReader(VcfReader&&) noexcept = default;
  VcfReader::~VcfReader() noexcept = default;

  // status
  auto VcfReader::get_file_path() const -> const std::string& { return pimpl->get_file_path(); }
  auto VcfReader::is_open() const -> bool { return pimpl->fp != nullptr; }
  auto VcfReader::is_closed() const -> bool { return pimpl->fp == nullptr; }
  auto VcfReader::has_index() const -> bool { return pimpl->idx != nullptr; }

  auto VcfReader::iter_query_record() -> VcfRecord const& { return pimpl->iter_query_record(); }
  auto VcfReader::query(const std::string& chrom, int64_t start, int64_t end) -> const_iterator {
    return pimpl->query(chrom, start, end);
  }

  auto VcfReader::begin() -> iterator { return pimpl->begin(); }
  auto VcfReader::begin() const -> const_iterator { return pimpl->begin(); }
  auto VcfReader::end() const -> value_type { return pimpl->end(); }
  void VcfReader::tell() { *this = VcfReader{get_file_path()}; }

}  // namespace binary