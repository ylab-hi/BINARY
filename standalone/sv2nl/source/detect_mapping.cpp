//
// Created by li002252 on 8/24/22.
//

#include "detect_mapping.hpp"

void map_duplicate(std::string_view nl_file, std::string_view sv_file) {
  auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_file));
  auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_file));

  auto interval_tree = Sv2nlVcfIntervalTree{};
  interval_tree.insert_node(sv_vcf_ranges);

  for (auto nl_vcf_record : nl_vcf_ranges) {
    interval_tree.find_overlaps(std::move(nl_vcf_record));
  }
}
