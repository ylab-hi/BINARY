//
// Created by li002252 on 6/13/22.
//
#include <doctest/doctest.h>
#include <sv2nl/vcf_reader.h>

TEST_CASE("testing vcf reader") {
  using namespace sv2nl;

  auto constexpr file_path = "../../test/data/debug.vcf.gz";
  VcfReader reader(file_path);
  CHECK_EQ(reader.is_open(), true);
  CHECK_EQ(reader.get_file_path(), file_path);

  SUBCASE("testing  query") {
    CHECK_FALSE(reader.has_index());
    reader.query("chr10", 93547288, 93867288);
  }

  SUBCASE("testing read record") {
    // test case
    // chr10   93567288        1       .       <TRA>
    // .       .       CANONICAL;BOUNDARY=NEITHER;SVTYPE=TRA;
    // SR=2;OSR=1;CHR2=chr17;SVEND=7705262
    reader.next_record();
    CHECK_EQ(reader.get_chrom(), "chr10");
    CHECK_EQ(reader.get_pos(), 93567288 - 1);
    // 0-based
    CHECK_EQ(reader.get_info_int("SVEND"), 7705262);
    CHECK_EQ(reader.get_info_string("SVTYPE"), "TRA");
    CHECK_EQ(get_info_field<int32_t>("SVEND", reader), 7705262);
  }

  SUBCASE("testing read all record") {
    while (reader.next_record() >= 0) {
      reader.check_record();
    }
  }
}
