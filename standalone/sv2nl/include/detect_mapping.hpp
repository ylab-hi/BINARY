//
// Created by li002252 on 8/24/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
#define BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_

#include <array>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "vcf_info.hpp"
#include "writer.hpp"

namespace sv2nl {

  namespace fs = std::filesystem;

  // Hardcode chromosome name for now.
  // TODO: Change to dynamic chromosome name.
  constexpr std::array<std::string_view, 22> CHROMOSOME_NAMES
      = {"chr1",  "chr2",  "chr3",  "chr4",  "chr5",  "chr6",  "chr7",  "chr8",
         "chr9",  "chr10", "chr11", "chr12", "chr13", "chr14", "chr15", "chr16",
         "chr17", "chr18", "chr19", "chr20", "chr21", "chr22"};

  constexpr char HEADER[] = "chrom\tpos\tend\tsvtype\tchrom\tpos\tend\tsvtype";

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

  class Mapper {
  public:
    Mapper(std::string_view non_linear_file, std::string_view sv_file, std::string_view output_file)
        : nl_vcf_file_(non_linear_file), sv_vcf_file_(sv_file), writer_(output_file, HEADER) {}

    void map() const;

    [[maybe_unused]] void map_duplicate() const;

    [[maybe_unused]] void map_duplicate_thread_pool() const;

  private:
    auto map_overlaps_impl(std::string_view chrom) const;
    auto map_trans() const;

    auto map_trans_impl(std::string_view chrom,
                        Sv2nlVcfIntervalTree const& vcf_interval_tree) const;

    auto build_tree(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges) const
        -> Sv2nlVcfIntervalTree const;

    auto build_tree(std::initializer_list<std::string_view> chroms,
                    const Sv2nlVcfRanges& vcf_ranges) const -> Sv2nlVcfIntervalTree const;

    std::pair<std::string, std::string> get_2chroms(Sv2nlVcfRecord const& record) const;

    std::string get_chr2(Sv2nlVcfRecord const& record) const;

    auto build_tree_use_index(std::string_view) const -> Sv2nlVcfIntervalTree const;

    [[maybe_unused]] auto build_tree_use_index(std::initializer_list<std::string_view>) const
        -> Sv2nlVcfIntervalTree const;

    auto build_tree_no_index(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges) const
        -> Sv2nlVcfIntervalTree const;

    auto build_tree_no_index(std::initializer_list<std::string_view> chroms,
                             const Sv2nlVcfRanges& vcf_ranges,
                             std::string_view svtype = "BND") const -> Sv2nlVcfIntervalTree const;

    fs::path nl_vcf_file_;
    fs::path sv_vcf_file_;
    mutable Writer writer_;
  };

  [[nodiscard]] bool is_contained(const Sv2nlVcfRecord& target, const Sv2nlVcfRecord& source);
  [[nodiscard]] bool check_condition(const Sv2nlVcfRecord& nl_vcf_record,
                                     const Sv2nlVcfRecord& sv_vcf_record);

  [[nodiscard]] bool check_inversion(const Sv2nlVcfRecord& nl_vcf_record,
                                     const Sv2nlVcfRecord& sv_vcf_record);

  Sv2nlVcfRecord validate_record(const Sv2nlVcfRecord& record) noexcept;

}  // namespace sv2nl
#endif  // BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
