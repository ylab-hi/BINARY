//
// Created by li002252 on 8/24/22.
//

#include "detect_mapping.hpp"

#include <spdlog/spdlog.h>

#include <ranges>
#include <unordered_map>

auto build_tree_use_index(const Sv2nlVcfRanges&, std::string_view) -> Sv2nlVcfIntervalTree {
  auto interval_tree = Sv2nlVcfIntervalTree{};
  assert(false);
  return interval_tree;
}
auto build_tree_no_index(const Sv2nlVcfRanges& sv_vcf_ranges, std::string_view chrom)
    -> Sv2nlVcfIntervalTree {
  auto interval_tree = Sv2nlVcfIntervalTree{};

  for (auto sv_vcf_record : sv_vcf_ranges | std::views::filter([&chrom](auto const& sv_vcf_record) {
                              return sv_vcf_record.chrom == chrom;
                            })) {
    interval_tree.insert_node(std::move(sv_vcf_record));
  }

  // no copy need to  move
  return interval_tree;
}
auto build_tree(const Sv2nlVcfRanges& sv_vcf_ranges, std::string_view chrom)
    -> Sv2nlVcfIntervalTree {
  return sv_vcf_ranges.has_read_index() ? build_tree_use_index(sv_vcf_ranges, chrom)
                                        : build_tree_no_index(sv_vcf_ranges, chrom);
}
auto find_overlaps(std::string_view chrom, const Sv2nlVcfRanges& nl_vcf_ranges,
                   const Sv2nlVcfRanges& sv_vcf_ranges)
    -> std::unordered_map<Sv2nlVcfRecord, std::vector<Sv2nlVcfRecord>> {
  std::unordered_map<Sv2nlVcfRecord, std::vector<Sv2nlVcfRecord>> overlaps{};

  auto interval_tree = build_tree(sv_vcf_ranges, chrom);

  auto chrom_view
      = nl_vcf_ranges | std::views::filter([&chrom](auto const& nl_vcf_record) {
          return nl_vcf_record.chrom == chrom
                 && (nl_vcf_record.info->svtype == "TDUP" || nl_vcf_record.info->svtype == "INV");
        });

  for (auto nl_vcf_record : chrom_view) {
    if (auto ret = interval_tree.find_overlaps(std::move(nl_vcf_record)); !ret.empty()) {
      std::vector<Sv2nlVcfRecord> overlaps_vector{};
      overlaps_vector.reserve(ret.size());
      for (auto const& overlap : ret) {
        overlaps_vector.push_back(overlap.record);
      }
      overlaps.try_emplace(nl_vcf_record, std::move(overlaps_vector));
    }
  }

  return overlaps;
}

void writer(std::unordered_map<Sv2nlVcfRecord, std::vector<Sv2nlVcfRecord>>&& overlaps) {
  for (auto const& [nl_vcf_record, sv_vcf_records] : overlaps) {
    for (auto const& sv_vcf_record : sv_vcf_records) {
      spdlog::info("nls {} sv {}", nl_vcf_record, sv_vcf_record);
    }
  }
}

void map_duplicate(std::string_view nl_file, std::string_view sv_file) {
  auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_file));
  auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_file));

  std::vector<Sv2nlVcfRecord> chrom_result(CHROMOSOME_NAMES.size());

  for (auto chrom : CHROMOSOME_NAMES) {
    writer(find_overlaps(chrom, nl_vcf_ranges, sv_vcf_ranges));
  }
}
