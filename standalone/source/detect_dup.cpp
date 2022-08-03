//
// Created by li002252 on 8/2/22.
//

#include "detect_dup.hpp"

#include "utils.hpp"

auto read_csv(std::string_view file_path) -> std::vector<std::tuple<std::string, pos_t, pos_t>> {
  std::vector<std::tuple<std::string, pos_t, pos_t>> ret{};

  io::CSVReader<3, io::trim_chars<' '>, io::no_quote_escape<'\t'>> in(
      std::string(file_path).data());
  in.read_header(io::ignore_extra_column, "chr", "start", "end");
  std::string chr;
  pos_t start;
  pos_t end;
  while (in.read_row(chr, start, end)) {
    ret.emplace_back(chr, start, end);
    spdlog::debug("chr {} start {} ", chr, start);
  }
  return ret;
}

void detect_dup(std::string const& vcf_path, std::string const& csv_path) {
  auto vcf_reader = sv2nl::VcfReader{vcf_path};

  auto csv_result = read_csv(csv_path);

  assert(vcf_reader.is_open());

  for (auto i = vcf_reader.begin(); i != vcf_reader.end(); ++i) {
    auto [vcf_chrom, vcf_start] = *i;
    auto vcf_end = sv2nl::get_info_field_int32("SVEND", i);
    auto vcf_type = sv2nl::get_info_field_string("SVTYPE", i);  // use enum class

    // TODO: Reconsidering copy operations of VcfReader

    if (vcf_start == 29927247) {
      spdlog::info("chrom: {} start: {}  end: {}  type: {}", vcf_chrom, vcf_start, vcf_end,
                   vcf_type);
      for (auto [csv_chrom, csv_start, csv_end] : csv_result) {
        if (enclose<pos_t>(csv_start, csv_end, vcf_start, vcf_end)) {
          spdlog::info("csv_chrom: {} csv_start: {} csv_end: {}", csv_chrom, csv_start, csv_end);
        }
      }
    }
  }
}
