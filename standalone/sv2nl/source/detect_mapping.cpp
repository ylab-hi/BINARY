//
// Created by li002252 on 8/24/22.
//

#include "detect_mapping.hpp"

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

  auto Mapper::build_tree_use_index(std::string_view) const -> Sv2nlVcfIntervalTree const {
    auto interval_tree = Sv2nlVcfIntervalTree{};
    assert(false);
    return interval_tree;
  }

  [[maybe_unused]] auto Mapper::build_tree_use_index(std::initializer_list<std::string_view>) const
      -> Sv2nlVcfIntervalTree const {
    assert(false);
    return sv2nl::Sv2nlVcfIntervalTree();
  }

  auto Mapper::build_tree_no_index(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges) const
      -> Sv2nlVcfIntervalTree const {
    auto interval_tree = Sv2nlVcfIntervalTree{};

    auto chrom_view
        = vcf_ranges | std::views::filter([&chrom](auto const& sv_vcf_record) {
            return sv_vcf_record.chrom == chrom
                   && (sv_vcf_record.info->svtype == "INV" || sv_vcf_record.info->svtype == "DUP");
          });

    interval_tree.insert_node(std::move(chrom_view));

    // no copy need to  move
    return interval_tree;
  }

  auto Mapper::build_tree_no_index(std::initializer_list<std::string_view> chroms,
                                   const Sv2nlVcfRanges& vcf_ranges, std::string_view svtype) const
      -> Sv2nlVcfIntervalTree const {
    auto interval_tree = Sv2nlVcfIntervalTree{};

    auto chrom_view
        = vcf_ranges | std::views::filter([&](auto const& sv_vcf_record) {
            return std::ranges::any_of(chroms, [&](auto it) { return sv_vcf_record.chrom == it; })
                   && (sv_vcf_record.info->svtype == svtype);
          });

    for (auto sv_vcf_record : chrom_view) {
      interval_tree.insert_node(std::move(sv_vcf_record));
    }

    return interval_tree;
  }

  auto Mapper::build_tree(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges) const
      -> Sv2nlVcfIntervalTree const {
    return vcf_ranges.has_index_file() ? build_tree_use_index(chrom)
                                       : build_tree_no_index(chrom, vcf_ranges);
  }

  auto Mapper::build_tree(std::initializer_list<std::string_view> chroms,
                          const Sv2nlVcfRanges& vcf_ranges) const -> Sv2nlVcfIntervalTree const {
    return vcf_ranges.has_index_file() ? build_tree_use_index(chroms)
                                       : build_tree_no_index(chroms, vcf_ranges);
  }

  auto Mapper::map_overlaps_impl(std::string_view chrom) const {
    // avoid data trace cause now vcf ranges is not thread safe
    auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_vcf_file_.string()));
    auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_vcf_file_.string()));

    auto interval_tree = build_tree(chrom, sv_vcf_ranges);

    auto chrom_view
        = nl_vcf_ranges | std::views::filter([&chrom](auto const& nl_vcf_record) {
            return nl_vcf_record.chrom == chrom
                   && (nl_vcf_record.info->svtype == "TDUP" || nl_vcf_record.info->svtype == "INV");
          });

    for (auto nl_vcf_record : chrom_view) {
      spdlog::debug("process {}", nl_vcf_record);

      auto validated_nl_vcf_record = validate_record(nl_vcf_record);
      std::vector<Sv2nlVcfRecord> overlaps_vector{std::move(nl_vcf_record)};

      if (auto ret = interval_tree.find_overlaps(validated_nl_vcf_record); !ret.empty()) {
        overlaps_vector.reserve(ret.size() + 1);
        for (auto const& overlap : ret) {
          if (check_condition(validated_nl_vcf_record, overlap.record)) {
            spdlog::debug("find overlap {}", overlap);
            overlaps_vector.push_back(std::move(overlap.record));
          }
        }
      }
      writer_.write(std::move(overlaps_vector));
    }
  }

  auto Mapper::map_trans_impl(std::string_view chrom,
                              Sv2nlVcfIntervalTree const& vcf_interval_tree) const {
    spdlog::debug("process chrom {}", chrom);

    auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_vcf_file_.string()));

    auto nl_chrom_view
        = nl_vcf_ranges | std::views::filter([&chrom](auto const& nl_vcf_record) {
            return nl_vcf_record.chrom == chrom && nl_vcf_record.info->svtype == "TRA";
          });

    for (auto nl_vcf_record : nl_chrom_view) {
      spdlog::debug("process nl vcf {}", nl_vcf_record);
      auto nl_2chroms = get_2chroms(nl_vcf_record);
      auto validated_nl_vcf_record = validate_record(nl_vcf_record);

      std::vector<Sv2nlVcfRecord> overlaps_vector{std::move(nl_vcf_record)};

      if (auto ret = vcf_interval_tree.find_overlaps(std::move(validated_nl_vcf_record));
          !ret.empty()) {
        overlaps_vector.reserve(ret.size() + 1);
        for (auto const& overlap : ret) {
          if (nl_2chroms == get_2chroms(overlap.record)) {
            spdlog::debug("find overlap {}", overlap);
            overlaps_vector.push_back(std::move(overlap.record));
          }
        }
      }

      writer_.write_trans(std::move(overlaps_vector));
    }
  }

  [[maybe_unused]] void Mapper::map_duplicate() const {
    std::vector<std::future<void>> results;
    for (auto chrom : CHROMOSOME_NAMES) {
      results.push_back(
          std::async([&](std::string_view chrom_) { map_overlaps_impl(chrom_); }, chrom));
    }
    for (auto& result : results) {
      result.wait();
    }
  }

  auto Mapper::map_trans() const {
    auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_vcf_file_.string()));
    auto sv_trans_view = sv_vcf_ranges | std::views::filter([&](auto const sv_vcf_record) {
                           return sv_vcf_record.info->svtype == "BND";
                         });

    auto sv_interval_tree = Sv2nlVcfIntervalTree{};
    sv_interval_tree.insert_node(sv_trans_view);

    spdlog::debug("SV Tree size : {}", sv_interval_tree.size());

    std::vector<std::future<void>> results;

    for (auto chrom : CHROMOSOME_NAMES) {
      results.push_back(std::async(
          [&](std::string_view chrom_, Sv2nlVcfIntervalTree const& sv_interval_tree_) {
            map_trans_impl(chrom_, sv_interval_tree_);
          },
          chrom, std::ref(sv_interval_tree)));
    }

    for (auto& res : results) res.wait();
  }

  [[maybe_unused]] void Mapper::map_duplicate_thread_pool() const {
    ThreadPool thread_pool(8);
    std::vector<std::future<void>> results;

    for (auto chrom : CHROMOSOME_NAMES) {
      results.push_back(
          thread_pool.enqueue([&](std::string_view chrom_) { map_overlaps_impl(chrom_); }, chrom));
    }
    for (auto& result : results) {
      result.wait();
    }
  }

  std::string Mapper::get_chr2(const Sv2nlVcfRecord& record) const {
    assert(record.info->svtype == "TRA" || record.info->svtype == "BND");
    return record.info->chr2;
  }

  std::pair<std::string, std::string> Mapper::get_2chroms(const Sv2nlVcfRecord& record) const {
    auto chr2 = get_chr2(record);
    return record.chrom > chr2 ? std::make_pair(chr2, record.chrom)
                               : std::make_pair(record.chrom, chr2);
  }

  void Mapper::map() const {
    map_duplicate();
    //    map_trans();
  }

  /**
   * Check if target  contains source
   * @return bool
   */
  bool is_contained(const Sv2nlVcfRecord& target, const Sv2nlVcfRecord& source) {
    assert(target.pos <= target.info->svend && source.pos <= source.info->svend);
    return target.pos <= source.pos && target.info->svend >= source.info->svend;
  }

  /**
   * Make sure vcf record's pos >= svend in order to find overlaps in interval tree
   * @param record
   */
  Sv2nlVcfRecord validate_record(const Sv2nlVcfRecord& record) noexcept {
    auto res = record;
    if (res.pos > res.info->svend) std::swap(res.pos, res.info->svend);
    return res;
  }

  bool check_inversion(const Sv2nlVcfRecord& nl_vcf_record, const Sv2nlVcfRecord& sv_vcf_record) {
    // overlap left side of sv_vcf record
    if (nl_vcf_record.pos <= sv_vcf_record.pos) {
      // check if nl vcf record is +-
      return nl_vcf_record.info->strand1 && !nl_vcf_record.info->strand2;
    }

    // overlap right side of sv_vcf record
    // check if nl vcf record is -+
    return !nl_vcf_record.info->strand1 && nl_vcf_record.info->strand2;
  }

  bool check_condition(const Sv2nlVcfRecord& nl_vcf_record, const Sv2nlVcfRecord& sv_vcf_record) {
    spdlog::debug("check condition {} {}", nl_vcf_record, sv_vcf_record);
    if (nl_vcf_record.info->svtype == "INV" && sv_vcf_record.info->svtype == "INV") {
      spdlog::debug("check condition inv {} {}", nl_vcf_record, sv_vcf_record);
      return !is_contained(sv_vcf_record, nl_vcf_record)
             && check_inversion(nl_vcf_record, sv_vcf_record);
    } else if (nl_vcf_record.info->svtype == "TDUP"
               //               && sv_vcf_record.info->svtype == "DUP"
    ) {
      spdlog::debug("check condition dup {} {}", nl_vcf_record, sv_vcf_record);
      return is_contained(sv_vcf_record, nl_vcf_record);
    }

    return false;
  }

}  // namespace sv2nl
