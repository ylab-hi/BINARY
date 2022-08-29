//
// Created by li002252 on 8/24/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
#define BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_

#include <array>
#include <string>

#include "vcf_info.hpp"

namespace sv2nl {
  // Hardcode chromosome name for now.
  // TODO: Change to dynamic chromosome name.
  constexpr std::array<std::string_view, 22> CHROMOSOME_NAMES
      = {"chr1",  "chr2",  "chr3",  "chr4",  "chr5",  "chr6",  "chr7",  "chr8",
         "chr9",  "chr10", "chr11", "chr12", "chr13", "chr14", "chr15", "chr16",
         "chr17", "chr18", "chr19", "chr20", "chr21", "chr22"};

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
   * 4. Handle for every chromosome (Consider parallelization)
   * 5. Write to output file (Consider parallelization and synchronization cost)
   *    Or write multiple files then merge into one
   */
  auto build_tree_use_index(Sv2nlVcfRanges const&, std::string_view) -> Sv2nlVcfIntervalTree;
  auto build_tree_no_index(Sv2nlVcfRanges const& sv_vcf_ranges, std::string_view chrom)
      -> Sv2nlVcfIntervalTree;

  auto build_tree(Sv2nlVcfRanges const& sv_vcf_ranges, std::string_view chrom)
      -> Sv2nlVcfIntervalTree;

  auto find_overlaps(std::string_view chrom, const Sv2nlVcfRanges& nl_vcf_ranges,
                     const Sv2nlVcfRanges& sv_vcf_ranges) -> decltype(auto);

  void writer(std::unordered_map<Sv2nlVcfRecord, std::vector<Sv2nlVcfRecord>>&& overlaps);
  void map_duplicate(std::string_view nl_file, std::string_view sv_file,
                     std::string_view output_file);
}  // namespace sv2nl
#endif  // BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
