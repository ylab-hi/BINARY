//
// Created by li002252 on 8/14/22.
//
//
// Created by li002252 on 6/13/22.
//
#include <binary/parser/vcf.hpp>
#include <binary/utils.hpp>

#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <algorithm>
#include <ranges>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

void test_vcf(std::string_view file_path) {
  using namespace binary::parser::vcf;

  auto vcf_reader = VcfRanges<VcfRecord>{std::string(file_path)};
  spdlog::debug("[test vcf] {}", vcf_reader.file_path());
  for (auto i = vcf_reader.query("chr17", 7707250, 7798250); i != vcf_reader.end();
       i = vcf_reader.iter_query_record()) {
    auto record = *i;
    spdlog::info("[test vcf] {}", record);
  }
}

TEST_SUITE("test vcf") {
  using namespace binary::parser::vcf;
  constexpr const char* file_path = "../../test/data/debug.vcf.gz";

  VcfRanges<VcfRecord> reader(file_path);

  TEST_CASE("testing vcf.hpp") {
    SUBCASE("test file path") { CHECK_EQ(reader.file_path(), file_path); }

    SUBCASE("test smoke vcf") { CHECK_NOTHROW(test_vcf(std::string(file_path))); }
  }

  TEST_CASE("testing read record") {
    // test case
    // chr10   93567288        1       .       <TRA>
    // .       .       CANONICAL;BOUNDARY=NEITHER;SVTYPE=TRA;
    // SR=2;OSR=1;CHR2=chr17;SVEND=7705262

    auto begin_iter = reader.begin();
    spdlog::debug("[testing read record {}", *begin_iter);
    CHECK_EQ(begin_iter->chrom, "chr10");
    CHECK_EQ(begin_iter->svtype, "TRA");
    // 0-based
    CHECK_EQ(begin_iter->pos, 93567288 - 1);
  }

  TEST_CASE("testing read all record") {
    for (auto record : reader) {
      spdlog::debug("[testing read all record {}", record);
    }
  }

  TEST_CASE("test std algorithm usage") {
    auto begin = reader.begin();
    auto end = reader.end();
    auto v = std::views::common(std::ranges::subrange{begin, end});
    std::for_each(v.begin(), v.end(),
                  [](auto record) { spdlog::debug("[test std algorithm] {}", record); });

    spdlog::debug("number of chr10 {}", std::count_if(v.begin(), v.end(), [](auto record) {
                    return record.chrom == "chr10";
                  }));
  }

  TEST_CASE("test c++20 vcf ") {
    static_assert(std::forward_iterator<VcfRanges<VcfRecord>::iterator>);

    for (auto record :
         reader | std::views::filter([](auto const& record) { return record.chrom == "chr10"; })) {
      spdlog::debug("chrom: {}", record);
    }
  }
}
