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
    spdlog::debug("[test vcf] {}", record);
  }
  CHECK_EQ(vcf_reader.has_index(), true);
}

TEST_SUITE("test vcf") {
  using namespace binary::parser::vcf;
  constexpr const char* file_path = "../../test/data/debug.vcf.gz";

  VcfRanges<VcfRecord> vcf_ranges(file_path);

  TEST_CASE("testing vcf.hpp") {
    SUBCASE("test file path") { CHECK_EQ(vcf_ranges.file_path(), file_path); }

    SUBCASE("test smoke vcf") { CHECK_NOTHROW(test_vcf(std::string(file_path))); }
  }

  TEST_CASE("testing read record") {
    // test case
    // chr10   93567288        1       .       <TRA>
    // .       .       CANONICAL;BOUNDARY=NEITHER;SVTYPE=TRA;
    // SR=2;OSR=1;CHR2=chr17;SVEND=7705262

    auto begin_iter = vcf_ranges.begin();
    spdlog::debug("[testing read record {}", *begin_iter);
    CHECK_EQ(begin_iter->chrom, "chr10");
    CHECK_EQ(begin_iter->info->svtype, "TRA");

    // 0-based
    CHECK_EQ(begin_iter->pos, 93567288 - 1);
  }

  TEST_CASE("testing read all record") {
    for (auto record : vcf_ranges) {
      spdlog::debug("[testing read all record] {}", record);
    }
  }

  TEST_CASE("test std algorithm usage") {
    std::ranges::for_each(vcf_ranges,
                          [](auto record) { spdlog::debug("[test std algorithm] {}", record); });

    spdlog::debug(
        "[test std algorithm] number of chr10 {}",
        std::ranges::count_if(vcf_ranges, [](auto record) { return record.chrom == "chr10"; }));
  }

  TEST_CASE("test c++20 vcf ") {
    static_assert(std::forward_iterator<VcfRanges<VcfRecord>::iterator>);

    for (auto record : vcf_ranges | std::views::filter([](auto const& record) {
                         return record.info->svtype == "TDUP";
                       })) {
      spdlog::debug("[test c++ 20 views] chrom: {}", record);
    }
  }

  TEST_CASE("test info factory") { InfoFieldFactory<char, pos_t> info_field1("SVTYPE", "SVEND"); }

  TEST_CASE("test construct vcf interval node from vcf record") {
    auto begin = vcf_ranges.begin();
    auto vcf_interval_node = VcfIntervalNode{*begin};
    CHECK_EQ(vcf_interval_node.interval.record.chrom, "chr10");
    CHECK_EQ(vcf_interval_node.interval.record.pos, 93567288 - 1);
    CHECK_EQ(vcf_interval_node.interval.record.info->svtype, "TRA");
  }

  TEST_CASE("test construct vcf interval node from low high record") {
    auto begin = vcf_ranges.begin();
    auto start = begin->pos;
    auto end = begin->info->svend;
    auto vcf_interval_node = VcfIntervalNode{end, start, *begin};
    CHECK_EQ(vcf_interval_node.interval.record.chrom, "chr10");
    CHECK_EQ(vcf_interval_node.interval.record.pos, 93567288 - 1);
    CHECK_EQ(vcf_interval_node.interval.record.info->svtype, "TRA");
  }

  TEST_CASE("test construct vcf interval tree from vcf record") {
    using namespace binary::algorithm::tree;

    auto interval_tree = IntervalTree<VcfIntervalNode>{};
    auto begin = vcf_ranges.begin();
    interval_tree.insert_node(*begin);
    CHECK_EQ(interval_tree.size(), 1);
  }

  TEST_CASE("test construct vcf interval tree from vcf record ranges") {
    using namespace binary::algorithm::tree;

    auto interval_tree = IntervalTree<VcfIntervalNode>{};
    interval_tree.insert_node(vcf_ranges);
    CHECK_EQ(interval_tree.size(), 6);
  }

  TEST_CASE("test construct vcf interval tree from vcf record ranges view") {
    using namespace binary::algorithm::tree;

    auto interval_tree = IntervalTree<VcfIntervalNode>{};
    auto v = vcf_ranges
             | std::views::filter([](auto const& record) { return record.info->svtype == "TRA"; });
    interval_tree.insert_node(v);
    CHECK_EQ(interval_tree.size(), 2);
  }

  TEST_CASE("test construct vcf interval tree from vcf record range for loop") {
    using namespace binary::algorithm::tree;

    auto interval_tree = IntervalTree<VcfIntervalNode>{};

    for (auto r : vcf_ranges) {
      interval_tree.insert_node(std::move(r));
    }

    CHECK_EQ(interval_tree.size(), 6);
  }
}
