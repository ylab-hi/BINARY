//
// Created by li002252 on 6/13/22.
//
#include <doctest/doctest.h>

#include <sv2nl/vcf_reader.hpp>

TEST_CASE("testing vcf reader") {
  using namespace sv2nl;

  auto constexpr file_path = "../../test/data/debug.vcf.gz";
  VcfReader reader(file_path);
  CHECK_EQ(reader.is_open(), true);
  CHECK_EQ(reader.get_file_path(), file_path);

  SUBCASE("testing  query") {
    CHECK_FALSE(reader.has_index());

    for (auto i = reader.query("chr17", 7707250, 7798250); i != reader.end();
         i = reader.iter_query_record()) {
      auto [chrom, pos] = *i;
      spdlog::info("Chrom: {} Pos: {} SVTYPE: {} SVEND: {}", chrom, pos,
                   get_info_field_string("SVTYPE", i), get_info_field_int32("SVEND", i));
    }
  }

  SUBCASE("testing read record") {
    // test case
    // chr10   93567288        1       .       <TRA>
    // .       .       CANONICAL;BOUNDARY=NEITHER;SVTYPE=TRA;
    // SR=2;OSR=1;CHR2=chr17;SVEND=7705262
    auto begin_record = reader.begin();
    CHECK_EQ(begin_record.get_chrom(), "chr10");
    CHECK_EQ(begin_record.get_pos(), 93567288 - 1);
    // 0-based
    CHECK_EQ(get_info_field_int32("SVEND", begin_record), 7705262);
  }

  SUBCASE("testing read all record") {
    for (auto i = reader.begin(); i != reader.end(); ++i) {
      auto [chrom, pos] = *i;
      spdlog::info("Chrom: {} Pos: {} SVTYPE: {} SVEND: {}", chrom, pos,
                   get_info_field_string("SVTYPE", i), get_info_field_int32("SVEND", i));
    }
  }
}
