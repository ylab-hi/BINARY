//
// Created by li002252 on 5/31/22.
//
#include <spdlog/spdlog.h>

#include <binary/csv.hpp>
#include <binary/parser.hpp>
#include <binary/test.hpp>
#include <iostream>

namespace binary {

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
    using namespace binary::parser;

    VcfReader reader{file_path};  // non-const
    assert(reader.is_open());
    std::cout << "file_path: " << reader.file_path() << '\n';

    for (auto& i : reader) {
      spdlog::info("Test for INT:: {}", get_info_field_int32("SVEND", i));
      spdlog::info("Test for STR:: {}", get_info_field_string("SVTYPE", i));
      spdlog::info("Chrom: {} Pos: {}", i.chrom(), i.pos());
    }

    for (auto& i : reader) {
      spdlog::info("Test again ");
      spdlog::info("Test for INT:: {}", get_info_field_int32("SVEND", i));
      spdlog::info("Test for STR:: {}", get_info_field_string("SVTYPE", i));
      spdlog::info("Chrom: {} Pos: {}", i.chrom(), i.pos());
    }
  }

  void test_vcf(std::string const& file_path) {
    using namespace binary::parser;

    auto vcf_reader = VcfReader{file_path};
    assert(vcf_reader.is_open());

    for (auto i = vcf_reader.query("chr17", 7707250, 7798250); i != vcf_reader.end();
         i = vcf_reader.iter_query_record()) {
      auto record = *i;
      auto [chrom, pos] = *record;
      spdlog::info("Chrom: {} Pos: {} SVTYPE: {} SVEND: {}", chrom, pos,
                   get_info_field_string("SVTYPE", record), get_info_field_int32("SVEND", record));
    }
  }

}  // namespace binary