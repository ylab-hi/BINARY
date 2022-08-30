//
// Created by li002252 on 8/24/22.
//

#include "detect_mapping.hpp"

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <binary/utils.hpp>
#include <future>
#include <ranges>
#include <unordered_map>

#include "thread_pool.hpp"
#include "writer.hpp"

namespace sv2nl {

  auto build_tree_use_index(const Sv2nlVcfRanges&, std::string_view) -> Sv2nlVcfIntervalTree {
    auto interval_tree = Sv2nlVcfIntervalTree{};
    assert(false);
    return interval_tree;
  }

  auto build_tree_no_index(const Sv2nlVcfRanges& sv_vcf_ranges, std::string_view chrom)
      -> Sv2nlVcfIntervalTree {
    auto interval_tree = Sv2nlVcfIntervalTree{};

    for (auto sv_vcf_record :
         sv_vcf_ranges | std::views::filter([&chrom](auto const& sv_vcf_record) {
           return sv_vcf_record.chrom == chrom
                  && (sv_vcf_record.info->svtype == "INV" || sv_vcf_record.info->svtype == "DUP");
         })) {
      interval_tree.insert_node(std::move(sv_vcf_record));
    }

    // no copy need to  move
    return interval_tree;
  }

  auto build_tree(Sv2nlVcfRanges const& sv_vcf_ranges, std::string_view chrom)
      -> Sv2nlVcfIntervalTree {
    return sv_vcf_ranges.has_index_file() ? build_tree_use_index(sv_vcf_ranges, chrom)
                                          : build_tree_no_index(sv_vcf_ranges, chrom);
  }

  auto find_overlaps(std::string_view chrom, const Sv2nlVcfRanges& nl_vcf_ranges,
                     const Sv2nlVcfRanges& sv_vcf_ranges) -> decltype(auto) {
    std::vector<std::vector<Sv2nlVcfRecord>> overlaps{};
    auto interval_tree = build_tree(sv_vcf_ranges, chrom);

    auto chrom_view
        = nl_vcf_ranges | std::views::filter([&chrom](auto const& nl_vcf_record) {
            return nl_vcf_record.chrom == chrom
                   && (nl_vcf_record.info->svtype == "TDUP" || nl_vcf_record.info->svtype == "INV");
          });

    for (auto nl_vcf_record : chrom_view) {
      spdlog::debug("handle {}", nl_vcf_record);
      std::vector<Sv2nlVcfRecord> overlaps_vector{nl_vcf_record};
      if (auto ret = interval_tree.find_overlaps(std::move(nl_vcf_record)); !ret.empty()) {
        overlaps_vector.reserve(ret.size() + 1);
        for (auto const& overlap : ret) {
          spdlog::debug("find overlap {}", overlap);
          overlaps_vector.push_back(std::move(overlap.record));
        }
      }
      overlaps.push_back(std::move(overlaps_vector));
    }

    return overlaps;
  }

  void writer(std::vector<std::vector<Sv2nlVcfRecord>>&& overlaps) {
    for (auto const& record_vector : overlaps) {
      for (auto const& record : record_vector) {
        spdlog::debug("writer {}", record);
      }
    }
  }

  [[maybe_unused]] void map_duplicate_sync(std::string_view nl_file, std::string_view sv_file,
                                           std::string_view output_file) {
    auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_file));
    auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_file));

    auto result_writer = Writer(output_file, "chrom\tpos\tend\tsvtype\tchrom\tpos\tend\tsvtype");

    for (auto chrom : CHROMOSOME_NAMES) {
      spdlog::debug("process chrom {}", chrom);
      for (auto const& record_vector : find_overlaps(chrom, nl_vcf_ranges, sv_vcf_ranges)) {
        result_writer.write(record_vector);
      }
    }
  }

  auto map_duplicate_async_impl(std::string_view nl_file, std::string_view sv_file,
                                std::string_view output_file, std::string_view chrom) {
    auto nl_vcf_ranges = Sv2nlVcfRanges(std::string(nl_file));
    auto sv_vcf_ranges = Sv2nlVcfRanges(std::string(sv_file));
    auto result_name = std::string(output_file) + "." + chrom.data();
    auto result_writer = Writer(result_name);

    for (auto const& record_vector : find_overlaps(chrom, nl_vcf_ranges, sv_vcf_ranges)) {
      result_writer.write(record_vector);
    }
    return result_name;
  }

  [[maybe_unused]] void map_duplicate_async(std::string_view nl_file, std::string_view sv_file,
                                            std::string_view output_file) {
    std::vector<std::future<std::string>> results;
    std::vector<std::string> result_names;

    for (auto chrom : CHROMOSOME_NAMES) {
      results.push_back(std::async(map_duplicate_async_impl, nl_file, sv_file, output_file, chrom));
    }

    for (auto& result : results) {
      result_names.push_back(result.get());
    }

    binary::utils::merge_files(result_names, output_file,
                               "chrom\tpos\tend\tsvtype\tchrom\tpos\tend\tsvtype");
  }

  void map_duplicate_thread_pool(std::string_view nl_file, std::string_view sv_file,
                                 std::string_view output_file) {
    ThreadPool thread_pool(8);

    std::vector<std::string> result_names;
    std::vector<std::future<std::string>> results;

    for (auto chrom : CHROMOSOME_NAMES) {
      results.push_back(
          thread_pool.enqueue(map_duplicate_async_impl, nl_file, sv_file, output_file, chrom));
    }

    for (auto& result : results) {
      result_names.push_back(result.get());
    }

    binary::utils::merge_files(result_names, output_file,
                               "chrom\tpos\tend\tsvtype\tchrom\tpos\tend\tsvtype");
  }

}  // namespace sv2nl
