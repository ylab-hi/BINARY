//
// Created by li002252 on 8/24/22.
//

#include "mapper.hpp"

#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <binary/parser/vcf.hpp>
#include <binary/utils.hpp>
#include <future>
#include <ranges>
#include <unordered_map>

namespace sv2nl {
  using namespace std::string_literals;

  mapper_options& mapper_options::nl_file(std::string_view file) {
    nl_file_ = file;
    return *this;
  }
  mapper_options& mapper_options::sv_file(std::string_view file) {
    sv_file_ = file;
    return *this;
  }
  mapper_options& mapper_options::output_file(std::string_view file) {
    output_file_ = file;
    return *this;
  }
  mapper_options& mapper_options::nl_type(std::string_view type) {
    nl_type_ = type;
    return *this;
  }
  mapper_options& mapper_options::sv_type(std::string_view type) {
    sv_type_ = type;
    return *this;
  }
  mapper_options& mapper_options::diff(uint32_t num) {
    diff_ = num;
    return *this;
  }

  mapper_options& mapper_options::use_strand(bool use) {
    use_strand_ = use;
    return *this;
  }

  bool DupMapper::check_condition(const Sv2nlVcfRecord& nl_vcf_record,
                                  const Sv2nlVcfRecord& sv_vcf_record) const {
    spdlog::debug("check condition with {}", sv_vcf_record);
    return is_contained(sv_vcf_record, nl_vcf_record)
           && distance_less(nl_vcf_record, sv_vcf_record, diff_);
  }

  bool InvMapper::check_condition(const Sv2nlVcfRecord& nl_vcf_record,
                                  const Sv2nlVcfRecord& sv_vcf_record) const {
    if (is_contained(sv_vcf_record, nl_vcf_record) || is_contained(nl_vcf_record, sv_vcf_record)
        || distance_larger(nl_vcf_record, sv_vcf_record, diff_))
      return false;

    // if not use strand
    if (!use_strand_) return true;

    // overlap left side of sv_vcf record
    if (nl_vcf_record.pos <= sv_vcf_record.pos) {
      // check if nl vcf record is +-
      spdlog::debug("{}-{} left side check condition with {}", nl_vcf_record.info->strand1,
                    nl_vcf_record.info->strand2, sv_vcf_record);
      return nl_vcf_record.info->strand1 && !nl_vcf_record.info->strand2;
    }

    spdlog::debug("{}-{} right side check condition with {}", nl_vcf_record.info->strand1,
                  nl_vcf_record.info->strand2, sv_vcf_record);
    // overlap right side of sv_vcf record
    // check if nl vcf record is -+
    return !nl_vcf_record.info->strand1 && nl_vcf_record.info->strand2;
  }

  void TraMapper::map_impl(std::string_view chrom,
                           const std::shared_ptr<Sv2nlVcfIntervalTree>& vcf_tree_ptr) const {
    // avoid data trace cause now vcf ranges is not thread safe
    spdlog::debug("[tra] chrom {} interval tree size: {}", chrom, vcf_tree_ptr->size());

    auto nl_vcf_ranges = Sv2nlVcfRanges(nl_vcf_file_.string());

    auto nl_chrom_view
        = nl_vcf_ranges | std::views::filter([&](auto const& nl_vcf_record) {
            return nl_vcf_record.chrom == chrom && nl_vcf_record.info->svtype == nl_type_;
          });

    for (auto nl_vcf_record : nl_chrom_view) {
      spdlog::debug("[tra] process nl vcf {}", nl_vcf_record);
      auto nl_2chroms = get_2chroms(nl_vcf_record);
      std::vector<Sv2nlVcfRecord> overlaps_vector{};

      // check if the result of the record is  cached
#ifdef SV2NL_USE_CACHE
      if (!find(nl_vcf_record, overlaps_vector)) {
#endif
        overlaps_vector.push_back(nl_vcf_record);
        auto validated_nl_vcf_record = validate_record(nl_vcf_record);

        auto&& res = vcf_tree_ptr->find_overlaps(validated_nl_vcf_record);

        auto res_view
            = res | std::views::filter([&](auto const& vcf_interval) {
                return (nl_2chroms == get_2chroms(vcf_interval.record))
                       && check_condition(validated_nl_vcf_record, vcf_interval.record);
              })
              | std::views::transform([](auto const& vcf_interval) { return vcf_interval.record; });

        std::ranges::move(res_view, std::back_inserter(overlaps_vector));

        spdlog::trace("[tra] overlaps size: {}", overlaps_vector.size());

#ifdef SV2NL_USE_CACHE
        if (overlaps_vector.size() > 1) {
          store(nl_vcf_record, overlaps_vector);
          // Do not output same nl key record
          writer_.write_trans(std::move(overlaps_vector));
        }
      }
#endif
    }
  }

  auto TraMapper::map_delegate(ThreadPool& pool) const -> void {
    auto&& nl_chroms = Sv2nlVcfRanges(nl_vcf_file_.string()).chroms();

    auto sv_tree_pointer = build_sv_tree();

    for (auto chrom : nl_chroms | std::views::filter([](auto it) {
                        return std::ranges::find(it, '_') == it.end();
                      })) {
      spdlog::debug("[tra] chrom {} is processing ", chrom);
      pool.enqueue_detach(
          [&](std::string_view chrom_, std::shared_ptr<Sv2nlVcfIntervalTree> tree) {
            map_impl(chrom_, tree);
          },
          chrom, sv_tree_pointer);
    }
  }

  bool TraMapper::check_condition(const Sv2nlVcfRecord& nl_vcf_record,
                                  const Sv2nlVcfRecord& sv_vcf_record) const {
    spdlog::debug("[tra] check condition with {}", sv_vcf_record);

    return distance_less(nl_vcf_record, sv_vcf_record, diff_);
  }

  std::shared_ptr<Sv2nlVcfIntervalTree> TraMapper::build_sv_tree() const {
    auto sv_vcf_ranges = Sv2nlVcfRanges(sv_vcf_file_.string());

    auto sv_trans_view = sv_vcf_ranges | std::views::filter([&](auto const sv_vcf_record) {
                           return sv_vcf_record.info->svtype == sv_type_;
                         });

    auto sv_interval_tree = std::make_shared<Sv2nlVcfIntervalTree>();

    sv_interval_tree->insert_node(sv_trans_view);

    return sv_interval_tree;
  }

}  // namespace sv2nl
