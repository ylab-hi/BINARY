//
// Created by li002252 on 8/24/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
#define BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_

#define SV2NL_USE_CACHE ON

#include <array>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "helper.hpp"
#include "threadsafe_map.hpp"
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

  constexpr const char HEADER[] = "chrom\tpos\tend\tsvtype\tchrom\tpos\tend\tsvtype";

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
    Mapper(std::string_view non_linear_file, std::string_view sv_file, std::string_view output_file,
           std::string_view nl_type, std::string_view sv_type)
        : nl_vcf_file_(non_linear_file),
          sv_vcf_file_(sv_file),
          writer_(output_file, HEADER),
          nl_type_(nl_type),
          sv_type_(sv_type) {}

    virtual ~Mapper() = default;
    virtual void map() = 0;

    static auto build_tree(std::string_view chrom, const Sv2nlVcfRanges& vcf_ranges,
                           std::string_view svtype) -> Sv2nlVcfIntervalTree const;

    static auto build_tree(std::initializer_list<std::string_view> chroms,
                           const Sv2nlVcfRanges& vcf_ranges, std::string_view svtype)
        -> Sv2nlVcfIntervalTree const;

    [[nodiscard]] bool find(std::string const&, std::vector<Sv2nlVcfRecord>&) const;
    [[nodiscard]] bool find(Sv2nlVcfRecord const&, std::vector<Sv2nlVcfRecord>&) const;

    void store(std::string const&, std::vector<Sv2nlVcfRecord>&) const;
    void store(Sv2nlVcfRecord const&, std::vector<Sv2nlVcfRecord>&) const;

  protected:
    fs::path nl_vcf_file_;
    fs::path sv_vcf_file_;
    mutable Writer writer_;
    std::string nl_type_;
    std::string sv_type_;
    mutable ThreadSafeMap<std::string, std::vector<Sv2nlVcfRecord>> cache_{};
  };

  class DupMapper : public Mapper {
  public:
    using Mapper::Mapper;

    ~DupMapper() override = default;

    void map() override;

  private:
    void map_impl(std::string_view chrom) const;

    bool check_condition(Sv2nlVcfRecord const& nl_vcf_record,
                         Sv2nlVcfRecord const& sv_vcf_record) const;
  };

  class InvMapper : public Mapper {
  public:
    using Mapper::Mapper;

    ~InvMapper() override = default;

    void map() override;

  private:
    void map_impl(std::string_view chrom) const;
    bool check_condition(Sv2nlVcfRecord const& nl_vcf_record,
                         Sv2nlVcfRecord const& sv_vcf_record) const;
  };

  class TraMapper : public Mapper {
  public:
    using Mapper::Mapper;
    TraMapper(std::string_view non_linear_file, std::string_view sv_file,
              std::string_view output_file, std::string_view nl_type, std::string_view sv_type,
              uint32_t diff)
        : Mapper(non_linear_file, sv_file, output_file, nl_type, sv_type), diff_(diff) {}

    ~TraMapper() override = default;

    void map() override;
    [[maybe_unused]] void set_diff(uint32_t diff) { diff_ = diff; }

  private:
    uint32_t diff_{0};
    void map_impl(std::string_view chrom, Sv2nlVcfIntervalTree const& vcf_interval_tree) const;
    bool check_condition(Sv2nlVcfRecord const& nl_vcf_record,
                         Sv2nlVcfRecord const& sv_vcf_record) const;
  };

}  // namespace sv2nl
#endif  // BUILDALL_STANDALONE_SV2NL_DETECT_MAPPING_HPP_
