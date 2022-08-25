//
// Created by li002252 on 8/24/22.
//

#include "detect_mapping.hpp"

#include <ranges>
#include <unordered_map>

auto build_tree_use_index(Sv2nlVcfRanges const&, std::string_view) -> Sv2nlVcfIntervalTree {
  auto interval_tree = Sv2nlVcfIntervalTree{};
  assert(false);
  return interval_tree;
}

auto build_tree_no_index(Sv2nlVcfRanges const& sv_vcf_ranges, std::string_view chrom)
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

auto build_tree(Sv2nlVcfRanges const& sv_vcf_ranges, std::string_view chrom)
    -> Sv2nlVcfIntervalTree {
  return sv_vcf_ranges.has_read_index() ? build_tree_use_index(sv_vcf_ranges, chrom)
                                        : build_tree_no_index(sv_vcf_ranges, chrom);
}

auto find_overlaps(std::string_view chrom, Sv2nlVcfRanges const& nl_vcf_ranges,
                   Sv2nlVcfRanges const& sv_vcf_ranges)
    -> std::unordered_map<Sv2nlVcfRecord, std::vector<Sv2nlVcfInterval>> {
  std::unordered_map<Sv2nlVcfRecord, std::vector<Sv2nlVcfInterval>> overlaps{};

  auto interval_tree = build_tree(sv_vcf_ranges, chrom);

  for (auto nl_vcf_record :
       nl_vcf_ranges | std::views::filter([&chrom](auto const& nl_vcf_record) {
         return nl_vcf_record.chrom == chrom
                && (nl_vcf_record.info->svtype == "TDUP" || nl_vcf_record.info->svtype == "INV");
       })) {
    if (auto ret = interval_tree.find_overlaps(std::move(nl_vcf_record)); !ret.empty()) {
      overlaps.try_emplace(nl_vcf_record, std::move(ret));
    }
  }

  return overlaps;
}

auto writer(std::unordered_map<Sv2nlVcfRecord, std::vector<Sv2nlVcfInterval>>&& overlaps) {
  for (auto const& [nl_vcf_record, sv_vcf_intervals] : overlaps) {
    for (auto const& sv_vcf_interval : sv_vcf_intervals) {
      std::cout << nl_vcf_record << '\t' << sv_vcf_interval.record << '\n';
    }
  }
}

void map_duplicate(std::string_view nl_file, std::string_view sv_file) {
  auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_file));
  auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_file));

  for (auto chrom : CHROMOSOME_NAMES) {
    writer(find_overlaps(chrom, nl_vcf_ranges, sv_vcf_ranges));
  }
}
