//
// Created by li002252 on 8/24/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
#define BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_

#include <thread_pool/thread_pool.h>

#include <array>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "helper.hpp"
#include "options.hpp"
#include "threadsafe_map.hpp"
#include "vcf_info.hpp"
#include "writer.hpp"

namespace sv2nl {

  namespace fs = std::filesystem;

  using ThreadPool = dp::thread_pool<dp::details::default_function_type>;

  constexpr std::string_view HEADER = "chrom\tpos\tend\tsvtype\tchrom\tpos\tend\tsvtype";

  /**
   * @brief Detect mapping relationship for non-linear variation (NL) and SV (SV)
   * @param non_linear_file
   * @param sv_file
   *
   * @algorithm
   *
   * 1. Check overlap between NL and SV for TDUP and INV
   * 2. Must have information: chrom, pos
   * 3. Needed INFO fields: SVTYPE, SVEND
   * 4. Handle for every chromosome
   * 5. Write to one output file
   */

  struct mapper_options {
    std::string_view nl_file_;
    std::string_view sv_file_;
    std::string_view output_file_;
    std::string_view nl_type_;
    std::string_view sv_type_;
    uint32_t diff_;

    mapper_options& nl_file(std::string_view file);

    mapper_options& sv_file(std::string_view file);

    mapper_options& output_file(std::string_view file);

    mapper_options& nl_type(std::string_view type);

    mapper_options& sv_type(std::string_view type);

    mapper_options& diff(uint32_t num);
  };

  template <typename Derived> class Mapper {
  public:
    friend Derived;

    explicit Mapper(mapper_options const& opts)
        : nl_vcf_file_(opts.nl_file_),
          sv_vcf_file_(opts.sv_file_),
          writer_(opts.output_file_, HEADER),
          nl_type_(opts.nl_type_),
          sv_type_(opts.sv_type_),
          diff_(opts.diff_) {}

    Mapper(std::string_view non_linear_file, std::string_view sv_file, std::string_view output_file,
           std::string_view nl_type, std::string_view sv_type)
        : nl_vcf_file_(non_linear_file),
          sv_vcf_file_(sv_file),
          writer_(output_file, HEADER),
          nl_type_(nl_type),
          sv_type_(sv_type) {}

    virtual ~Mapper() = default;
    Derived* derived() { return static_cast<Derived*>(this); }
    Derived const* derived() const { return static_cast<Derived const*>(this); }

    void close_writer() const noexcept { writer_.close(); }

    auto map(ThreadPool& pool) const -> void { return derived()->map_delegate(pool); }

    auto map_delegate(ThreadPool& pool) const -> void;

    static auto build_tree(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges,
                           std::string_view svtype) -> Sv2nlVcfIntervalTree;

    [[maybe_unused]] static auto build_tree(std::initializer_list<std::string_view> chroms,
                                            const Sv2nlVcfRanges& vcf_ranges,
                                            std::string_view svtype) -> Sv2nlVcfIntervalTree;

    [[nodiscard]] bool find(std::string const&, std::vector<Sv2nlVcfRecord>&) const;
    [[nodiscard]] bool find(Sv2nlVcfRecord const&, std::vector<Sv2nlVcfRecord>&) const;

    void store(std::string const&, std::vector<Sv2nlVcfRecord>&) const;
    void store(Sv2nlVcfRecord const&, std::vector<Sv2nlVcfRecord>&) const;

  protected:
    void map_impl(std::string_view chrom) const;

    // Define pure virtual function
    // bool check_condition;
    // void map_impl(std::string_view, Sv2nlVcfIntervalTree const&) const;

    fs::path nl_vcf_file_;
    fs::path sv_vcf_file_;
    mutable Writer writer_;
    std::string nl_type_;
    std::string sv_type_;
    uint32_t diff_;
    mutable ThreadSafeMap<std::string, std::vector<Sv2nlVcfRecord>> cache_{};
  };

  template <typename Derived>
  bool Mapper<Derived>::find(std::string const& key, std::vector<Sv2nlVcfRecord>& value) const {
    if (cache_.find(key, value)) {
      spdlog::debug("[find] cache hit {}", key);
      return true;
    }

    return false;
  }

  template <typename Derived>
  void Mapper<Derived>::store(std::string const& key, std::vector<Sv2nlVcfRecord>& value) const {
    cache_.insert(key, value);
  }

  template <typename Derived>
  auto Mapper<Derived>::build_tree(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges,
                                   std::string_view svtype) -> Sv2nlVcfIntervalTree {
    auto interval_tree = Sv2nlVcfIntervalTree{};

    auto sorted_ = [](auto const& res) { return validate_record(res); };

    auto chrom_view = vcf_ranges | std::views::filter([&chrom, &svtype](auto const& sv_vcf_record) {
                        return sv_vcf_record.chrom == chrom && sv_vcf_record.info->svtype == svtype;
                      })
                      | std::views::transform(sorted_);

    interval_tree.insert_node(chrom_view);

    // no copy need to  move
    return interval_tree;
  }

  template <typename Derived>
  [[maybe_unused]] auto Mapper<Derived>::build_tree(std::initializer_list<std::string_view> chroms,
                                                    const Sv2nlVcfRanges& vcf_ranges,
                                                    std::string_view svtype)
      -> Sv2nlVcfIntervalTree {
    auto interval_tree = Sv2nlVcfIntervalTree{};

    auto chrom_view
        = vcf_ranges | std::views::filter([&](auto const& sv_vcf_record) {
            return std::ranges::any_of(chroms, [&](auto it) { return sv_vcf_record.chrom == it; })
                   && (sv_vcf_record.info->svtype == svtype);
          });
    interval_tree.insert_node(chrom_view);

    return interval_tree;
  }

  template <typename Derived> bool Mapper<Derived>::find(const Sv2nlVcfRecord& vcf_record,
                                                         std::vector<Sv2nlVcfRecord>& value) const {
    auto key = format_map_key(vcf_record);
    return find(key, value);
  }

  template <typename Derived>
  void Mapper<Derived>::store(const Sv2nlVcfRecord& vcf_record,
                              std::vector<Sv2nlVcfRecord>& value) const {
    auto key = format_map_key(vcf_record);
    store(key, value);
  }

  template <typename Derived> void Mapper<Derived>::map_impl(std::string_view chrom) const {
    // avoid data trace cause now vcf ranges is not thread safe
    auto&& nl_vcf_ranges = Sv2nlVcfRanges(nl_vcf_file_.string());
    auto sv_vcf_ranges = Sv2nlVcfRanges(sv_vcf_file_.string());

    auto interval_tree = build_tree(chrom, sv_vcf_ranges, sv_type_);

    auto chrom_view
        = nl_vcf_ranges | std::views::filter([&](auto const& nl_vcf_record) {
            return nl_vcf_record.chrom == chrom && (nl_vcf_record.info->svtype == nl_type_);
          });

    for (auto nl_vcf_record : chrom_view) {
      spdlog::debug("process nl record {} ", nl_vcf_record);
      std::vector<Sv2nlVcfRecord> overlaps_vector{};

      // check if the result of the record is  cached
#ifdef SV2NL_USE_CACHE
      if (!find(nl_vcf_record, overlaps_vector)) {
#endif
        overlaps_vector.push_back(nl_vcf_record);
        auto validated_nl_vcf_record = validate_record(nl_vcf_record);

        auto&& res = interval_tree.find_overlaps(validated_nl_vcf_record);
        auto res_view
            = res | std::views::filter([&](auto const& vcf_interval) {
                return derived()->check_condition(validated_nl_vcf_record, vcf_interval.record);
              })
              | std::views::transform([](auto const& vcf_interval) { return vcf_interval.record; });

        std::ranges::move(res_view, std::back_inserter(overlaps_vector));

#ifdef SV2NL_USE_CACHE
        if (overlaps_vector.size() > 1) {
          store(nl_vcf_record, overlaps_vector);
          // Do not output same nl key record
          writer_.write(std::move(overlaps_vector));
        }
      }
#endif
    }
  }

  template <typename Derived> auto Mapper<Derived>::map_delegate(ThreadPool& pool) const -> void {
    auto&& nl_chroms = Sv2nlVcfRanges(nl_vcf_file_.string()).chroms();

    for (auto chrom : nl_chroms | std::views::filter([](auto it) {
                        return std::ranges::find(it, '_') == it.end();
                      })) {
      pool.enqueue_detach([&](std::string_view chrom_) { derived()->map_impl(chrom_); }, chrom);
    }
  }

  class DupMapper : public Mapper<DupMapper> {
  public:
    friend class Mapper<DupMapper>;
    using Mapper::Mapper;

    ~DupMapper() override = default;

  private:
    [[nodiscard]] bool check_condition(Sv2nlVcfRecord const& nl_vcf_record,
                                       Sv2nlVcfRecord const& sv_vcf_record) const;
  };

  class InvMapper : public Mapper<InvMapper> {
  public:
    friend class Mapper<InvMapper>;
    using Mapper::Mapper;

    ~InvMapper() override = default;

  private:
    bool check_condition(Sv2nlVcfRecord const& nl_vcf_record,
                         Sv2nlVcfRecord const& sv_vcf_record) const;
  };

  class TraMapper : public Mapper<TraMapper> {
  public:
    friend class Mapper<TraMapper>;
    using Mapper::Mapper;

    ~TraMapper() override = default;

    void map_delegate(ThreadPool& pool) const;

  private:
    std::shared_ptr<Sv2nlVcfIntervalTree> build_sv_tree() const;

    void map_impl(std::string_view chrom,
                  const std::shared_ptr<Sv2nlVcfIntervalTree>& vcf_tree_ptr) const;
    bool check_condition(Sv2nlVcfRecord const& nl_vcf_record,
                         Sv2nlVcfRecord const& sv_vcf_record) const;
  };

}  // namespace sv2nl
#endif  // BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
