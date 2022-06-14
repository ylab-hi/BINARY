//
// Created by li002252 on 5/31/22.
//
#include <spdlog/spdlog.h>
#include <sv2nl/csv.h>
#include <sv2nl/test.h>
#include <sv2nl/vcf_reader.h>

#include <iostream>

#include "tbx.h"
#include "vcf.h"

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
    VcfReader reader{file_path};
    assert(reader.is_open());
    assert(!reader.is_closed());
    std::cout << "file_path: " << reader.get_file_path() << '\n';

    while (reader.next_record() >= 0) {
      spdlog::info("Chrom: {} Pos: {} SVTYPE: {} SVEND: {}", reader.get_chrom(), reader.get_pos(),
                   reader.get_info_int("SVEND"), reader.get_info_string("SVTYPE"));
    }
  }

  void test_vcf(const std::string& file_path) {
    auto vcf_reader = VcfReader{file_path};
    assert(vcf_reader.is_open());
    vcf_reader.query("chr17", 7707250, 7798250);
  }

}  // namespace sv2nl