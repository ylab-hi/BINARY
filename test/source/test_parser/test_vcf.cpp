//
// Created by li002252 on 8/14/22.
//
//
// Created by li002252 on 6/13/22.
//
#include <binary/algorithm/interval_tree.hpp>
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
  CHECK_EQ(vcf_reader.has_index(), false);
  for (auto i = vcf_reader.query("chr17", 7707250, 7798250); i != vcf_reader.end();
       i = vcf_reader.iter_query_record()) {
    auto record = *i;
    spdlog::info("[test vcf] {}", record);
  }
  CHECK_EQ(vcf_reader.has_index(), true);
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

    VcfRanges<VcfRecord> reader(file_path);
    auto begin_iter = reader.begin();
    spdlog::debug("[testing read record {}", *begin_iter);
    CHECK_EQ(begin_iter->chrom, "chr10");
    CHECK_EQ(begin_iter->info_data->svtype, "TRA");

    // 0-based
    CHECK_EQ(begin_iter->pos, 93567288 - 1);
  }

  TEST_CASE("testing read all record") {
    VcfRanges<VcfRecord> reader(file_path);
    for (auto record : reader) {
      spdlog::debug("[testing read all record {}", record);
    }
  }

  TEST_CASE("test std algorithm usage") {
    VcfRanges<VcfRecord> vcf_reader(file_path);
    auto begin = vcf_reader.begin();
    auto end = vcf_reader.end();
    auto v = std::views::common(std::ranges::subrange{begin, end});
    std::for_each(v.begin(), v.end(),
                  [](auto record) { spdlog::debug("[test std algorithm] {}", record); });

    spdlog::debug("number of chr10 {}", std::count_if(v.begin(), v.end(), [](auto record) {
                    return record.chrom == "chr10";
                  }));
  }

  TEST_CASE("test c++20 vcf ") {
    VcfRanges<VcfRecord> vcf_reader(file_path);
    static_assert(std::forward_iterator<VcfRanges<VcfRecord>::iterator>);

    for (auto record : vcf_reader | std::views::filter([](auto const& record) {
                         return record.chrom == "chr10";
                       })) {
      spdlog::debug("chrom: {}", record);
    }
  }

  TEST_CASE("test info factory") {
    details::InfoFieldFactory<char, pos_t> info_field1("SVTYPE", "SVEND");
  }

  TEST_CASE("test construct vcf interval node from vcf record") {
    VcfRanges<VcfRecord> vcf_reader(file_path);
    auto begin = vcf_reader.begin();
    auto vcf_interval_node = VcfIntervalNode{*begin};
    CHECK_EQ(vcf_interval_node.interval.record.chrom, "chr10");
    CHECK_EQ(vcf_interval_node.interval.record.pos, 93567288 - 1);
    CHECK_EQ(vcf_interval_node.interval.record.info_data->svtype, "TRA");
  }

  TEST_CASE("test construct vcf interval node from low high record") {
    VcfRanges<VcfRecord> vcf_reader(file_path);
    auto begin = vcf_reader.begin();
    auto start = begin->pos;
    auto end = begin->info_data->svend;
    auto vcf_interval_node = VcfIntervalNode{end, start, *begin};
    CHECK_EQ(vcf_interval_node.interval.record.chrom, "chr10");
    CHECK_EQ(vcf_interval_node.interval.record.pos, 93567288 - 1);
    CHECK_EQ(vcf_interval_node.interval.record.info_data->svtype, "TRA");
  }

  TEST_CASE("test construct vcf interval tree from vcf record") {
    using namespace binary::algorithm::tree;
    binary::utils::set_debug();
    auto interval_tree = IntervalTree<VcfIntervalNode>{};
  }
}
