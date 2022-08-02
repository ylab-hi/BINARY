//
// Created by li002252 on 5/31/22.
//
#include <spdlog/spdlog.h>

#include <iostream>
#include <sv2nl/csv.hpp>
#include <sv2nl/test.hpp>
#include <sv2nl/vcf_reader.hpp>

namespace sv2nl {

  [[maybe_unused]] void read_tsv(std::string_view file_path) {
    io::CSVReader<3, io::trim_chars<' '>, io::no_quote_escape<'\t'>> in(
        std::string(file_path).data());
    in.read_header(io::ignore_extra_column, "chr", "start", "end");
    std::string chr;
    int start;
    int end;
    while (in.read_row(chr, start, end)) {
      spdlog::info("chr {} start {} ", chr, start);
    }
  }

  void read_vcf(std::string const& file_path) {
    VcfReader reader{file_path};  // non-const
    assert(reader.is_open());
    assert(!reader.is_closed());
    std::cout << "file_path: " << reader.get_file_path() << '\n';

    for (auto i = reader.begin(); i != reader.end(); ++i) {
      auto [chrom, pos] = *i;
      spdlog::info("Test for INT:: {}", get_info_field_int32("SVEND", i));
      spdlog::info("Test for STR:: {}", get_info_field_string("SVTYPE", i));
      spdlog::info("Chrom: {} Pos: {}", chrom, pos);
    }
    reader.tell();
    assert(reader.is_open());

    for (auto i = reader.begin(); i != reader.end(); ++i) {
      auto [chrom, pos] = *i;
      spdlog::info("Test for INT2:: {}", get_info_field_int32("SVEND", i));
      spdlog::info("Test for STR2:: {}", get_info_field_string("SVTYPE", i));
      spdlog::info("Chrom: {} Pos2: {}", chrom, pos);
    }
  }

  void test_vcf(std::string const& file_path) {
    auto vcf_reader = VcfReader{file_path};
    assert(vcf_reader.is_open());
    for (auto i = vcf_reader.query("chr17", 7707250, 7798250); i != vcf_reader.end();
         i = vcf_reader.iter_query_record()) {
      auto [chrom, pos] = *i;
      spdlog::info("Chrom: {} Pos: {} SVTYPE: {} SVEND: {}", chrom, pos,
                   get_info_field_string("SVTYPE", i), get_info_field_int32("SVEND", i));
    }
  }

}  // namespace sv2nl