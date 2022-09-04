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

#include "thread_pool.hpp"

namespace sv2nl {

  bool Mapper::find(std::string const& key, std::vector<Sv2nlVcfRecord>& value) const {
    if (cache_.find(key, value)) {
      spdlog::debug("[find] cache hit {}", key);
      return true;
    }

    return false;
  }

  void Mapper::store(std::string const& key, std::vector<Sv2nlVcfRecord>& value) const {
    cache_.insert(key, value);
  }

  auto Mapper::build_tree(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges,
                          std::string_view svtype) -> Sv2nlVcfIntervalTree {
    auto interval_tree = Sv2nlVcfIntervalTree{};

    auto sorted_ = [](auto const& res) {
      spdlog::debug("[build tree] {}", res);
      return validate_record(res);
    };

    auto chrom_view = vcf_ranges | std::views::filter([&chrom, &svtype](auto const& sv_vcf_record) {
                        return sv_vcf_record.chrom == chrom && sv_vcf_record.info->svtype == svtype;
                      })
                      | std::views::transform(sorted_);

    interval_tree.insert_node(chrom_view);

    // no copy need to  move
    return interval_tree;
  }

  [[maybe_unused]] auto Mapper::build_tree(std::initializer_list<std::string_view> chroms,
                                           const Sv2nlVcfRanges& vcf_ranges,
                                           std::string_view svtype) -> Sv2nlVcfIntervalTree {
    auto interval_tree = Sv2nlVcfIntervalTree{};

    auto chrom_view
        = vcf_ranges | std::views::filter([&](auto const& sv_vcf_record) {
            return std::ranges::any_of(chroms, [&](auto it) { return sv_vcf_record.chrom == it; })
                   && (sv_vcf_record.info->svtype == svtype);
          });
    interval_tree.insert_node(chrom_view);

    return interval_tree;
  }

  bool Mapper::find(const Sv2nlVcfRecord& vcf_record, std::vector<Sv2nlVcfRecord>& value) const {
    auto key = format_map_key(vcf_record);
    return find(key, value);
  }
  void Mapper::store(const Sv2nlVcfRecord& vcf_record, std::vector<Sv2nlVcfRecord>& value) const {
    auto key = format_map_key(vcf_record);
    store(key, value);
  }

  void Mapper::map_impl(std::string_view chrom) const {
    // avoid data trace cause now vcf ranges is not thread safe
    auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_vcf_file_.string()));
    auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_vcf_file_.string()));

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
                return check_condition(validated_nl_vcf_record, vcf_interval.record);
              })
              | std::views::transform([](auto const& vcf_interval) { return vcf_interval.record; });

        std::ranges::move(res_view, std::back_inserter(overlaps_vector));

#ifdef SV2NL_USE_CACHE
        store(nl_vcf_record, overlaps_vector);
      }
#endif
      writer_.write(std::move(overlaps_vector));
    }
  }

  void Mapper::map() {
    std::vector<std::future<void>> results;
    for (auto chrom : CHROMOSOME_NAMES) {
      results.push_back(std::async([&](std::string_view chrom_) { map_impl(chrom_); }, chrom));
    }
    for (auto& result : results) {
      result.wait();
    }
  }

  bool DupMapper::check_condition(const Sv2nlVcfRecord& nl_vcf_record,
                                  const Sv2nlVcfRecord& sv_vcf_record) const {
    spdlog::debug("check condition with {}", sv_vcf_record);
    return is_contained(sv_vcf_record, nl_vcf_record);
  }

  bool InvMapper::check_condition(const Sv2nlVcfRecord& nl_vcf_record,
                                  const Sv2nlVcfRecord& sv_vcf_record) const {
    if (is_contained(sv_vcf_record, nl_vcf_record) || is_contained(nl_vcf_record, sv_vcf_record))
      return false;

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
                           Sv2nlVcfIntervalTree const& vcf_interval_tree) const {
    // avoid data trace cause now vcf ranges is not thread safe

    auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_vcf_file_.string()));

    auto nl_chrom_view
        = nl_vcf_ranges | std::views::filter([&](auto const& nl_vcf_record) {
            return nl_vcf_record.chrom == chrom && nl_vcf_record.info->svtype == nl_type_;
          });

    for (auto nl_vcf_record : nl_chrom_view) {
      spdlog::debug("process nl vcf {}", nl_vcf_record);
      auto nl_2chroms = get_2chroms(nl_vcf_record);
      std::vector<Sv2nlVcfRecord> overlaps_vector{};

      // check if the result of the record is  cached
#ifdef SV2NL_USE_CACHE
      if (!find(nl_vcf_record, overlaps_vector)) {
#endif
        overlaps_vector.push_back(nl_vcf_record);
        auto validated_nl_vcf_record = validate_record(nl_vcf_record);

        auto&& res = vcf_interval_tree.find_overlaps(validated_nl_vcf_record);
        auto res_view
            = res | std::views::filter([&](auto const& vcf_interval) {
                return (nl_2chroms == get_2chroms(vcf_interval.record))
                       && check_condition(validated_nl_vcf_record, vcf_interval.record);
              })
              | std::views::transform([](auto const& vcf_interval) { return vcf_interval.record; });

        std::ranges::move(res_view, std::back_inserter(overlaps_vector));

#ifdef SV2NL_USE_CACHE
        store(nl_vcf_record, overlaps_vector);
      }
#endif
      writer_.write_trans(std::move(overlaps_vector));
    }
  }

  void TraMapper::map() {
    auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_vcf_file_.string()));
    auto sv_trans_view = sv_vcf_ranges | std::views::filter([&](auto const sv_vcf_record) {
                           return sv_vcf_record.info->svtype == sv_type_;
                         });

    auto sv_interval_tree = Sv2nlVcfIntervalTree{};
    sv_interval_tree.insert_node(sv_trans_view);
    std::vector<std::future<void>> results;

    for (auto chrom : CHROMOSOME_NAMES) {
      results.push_back(std::async(
          [&](std::string_view chrom_, Sv2nlVcfIntervalTree const& sv_interval_tree_) {
            map_impl(chrom_, sv_interval_tree_);
          },
          chrom, std::ref(sv_interval_tree)));
    }
    for (auto& res : results) res.wait();
  }

  bool TraMapper::check_condition(const Sv2nlVcfRecord& nl_vcf_record,
                                  const Sv2nlVcfRecord& sv_vcf_record) const {
    auto diff1 = nl_vcf_record.pos >= sv_vcf_record.pos ? nl_vcf_record.pos - sv_vcf_record.pos
                                                        : sv_vcf_record.pos - nl_vcf_record.pos;

    auto diff2 = nl_vcf_record.info->svend >= sv_vcf_record.info->svend
                     ? nl_vcf_record.info->svend - sv_vcf_record.info->svend
                     : sv_vcf_record.info->svend - nl_vcf_record.info->svend;

    return diff1 <= diff_ && diff2 <= diff_;
  }

}  // namespace sv2nl
