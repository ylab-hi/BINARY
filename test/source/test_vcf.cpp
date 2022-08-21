//
// Created by li002252 on 8/14/22.
//
//
// Created by li002252 on 6/13/22.
//
#include <doctest/doctest.h>

#include <binary/parser/all.hpp>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <algorithm>
#include <ranges>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

TEST_CASE("testing vcf.hpp") {
  using namespace binary::parser;

  constexpr const char* file_path = "../../test/data/debug.vcf.gz";
  VcfReader reader(file_path);
  CHECK_EQ(reader.is_open(), true);
  CHECK_EQ(reader.file_path(), file_path);
  SUBCASE("testing read record") {
    // test case
    // chr10   93567288        1       .       <TRA>
    // .       .       CANONICAL;BOUNDARY=NEITHER;SVTYPE=TRA;
    // SR=2;OSR=1;CHR2=chr17;SVEND=7705262

    auto begin_record_iterator = *reader.begin();

    spdlog::debug("{} {}", begin_record_iterator.chrom(), begin_record_iterator.pos());
    CHECK_EQ(begin_record_iterator.chrom(), "chr10");
    CHECK_EQ(begin_record_iterator.pos(), 93567288 - 1);
    // 0-based
    CHECK_EQ(get_info_field_int32("SVEND", begin_record_iterator), 7705262);
  }

  SUBCASE("testing read all record") {
    for (auto& record : reader) {
      auto chrom = record.chrom();
      auto pos = record.pos();
      spdlog::debug("Chrom: {} Pos: {} SVTYPE: {} SVEND: {}", chrom, pos,
                    get_info_field_string("SVTYPE", record), get_info_field_int32("SVEND", record));
    }
  }

  SUBCASE("testing  query") {
    CHECK_FALSE(reader.has_index());

    for (auto iterator = reader.query("chr17", 7707250, 7798250); iterator != reader.end();
         iterator = reader.iter_query_record()) {
      auto& record = *iterator;
      auto chrom = record.chrom();
      auto pos = record.pos();
      spdlog::debug("Chrom: {} Pos: {} SVTYPE: {} SVEND: {}", chrom, pos,
                    get_info_field_string("SVTYPE", *iterator),
                    get_info_field_int32("SVEND", *iterator));
    }
  }

  SUBCASE("test std algorithm usage") {
    std::for_each(reader.begin(), reader.end(),
                  [](auto& record) { spdlog::debug("chrom: {}", record.chrom()); });

    spdlog::debug("number of chr10 {}",
                  std::count_if(reader.begin(), reader.end(),
                                [](auto& record) { return record.chrom() == "chr10"; }));
  }
}

TEST_CASE("test c++20 vcf ") {
  using namespace binary::parser::experimental;
  constexpr const char* file_path = "../../test/data/debug.vcf.gz";
  VcfRanges const reader(file_path);
  CHECK_EQ(reader.is_open(), true);
  CHECK_EQ(reader.file_path(), file_path);

  SUBCASE("test standard algorithm") {
    std::for_each(reader.begin(), reader.end(),
                  [](auto const& record) { spdlog::debug("chrom: {}", record.chrom()); });

    spdlog::debug("number of chr10 {}",
                  std::count_if(reader.begin(), reader.end(),
                                [](auto const& record) { return record.chrom() == "chr10"; }));
  }

  SUBCASE("test range and view") {
    static_assert(std::forward_iterator<VcfRanges<binary::parser::VcfRecord>::iterator>);

    for (auto record : reader | std::views::filter([](auto const& record) {
                         return record.chrom() == "chr10";
                       })) {
      spdlog::debug("chrom: {}", record.chrom());
    }
  }
}
