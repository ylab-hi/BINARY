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

void test_vcf_query(std::string_view file_path) {
  using namespace binary::parser::vcf;

  auto vcf_ranges = VcfRanges<VcfRecord>{std::string(file_path)};

  spdlog::debug("[test vcf query] {}", vcf_ranges.file_path());
  CHECK_EQ(vcf_ranges.has_read_index(), false);
  for (auto i = vcf_ranges.query("chr17", 7707250, 7798250); i != vcf_ranges.end();
       i = vcf_ranges.iter_query_record()) {
    auto record = *i;
    spdlog::debug("[test vcf] {}", record);
  }

  for (auto i = vcf_ranges.query("chr10"); i != vcf_ranges.end();
       i = vcf_ranges.iter_query_record()) {
    spdlog::debug("[test vcf query only chrom] {}", *i);
  }
  CHECK_EQ(vcf_ranges.has_read_index(), true);
}

void test_vcf_iter(std::string_view file_path) {
  using namespace binary::parser::vcf;
  auto vcf_reader = VcfRanges<VcfRecord>{std::string(file_path)};
  spdlog::debug("[test vcf iter] {}", vcf_reader.file_path());
  for (auto record : vcf_reader) {
    spdlog::debug("[test vcf iter] {}", record);
  }
}

TEST_SUITE("parser-vcf") {
  using namespace binary::parser::vcf;
  constexpr const char* file_path = "../../test/data/debug.vcf.gz";
  constexpr const char* uncompressed_file_path = "../../test/data/debug_uncom.vcf";

  VcfRanges<VcfRecord> vcf_ranges(file_path);

  TEST_CASE("test get chroms") {
    auto chroms = vcf_ranges.chroms();

    for (auto const& chrom : chroms | std::views::filter([](auto const& chrom) {
                               return std::ranges::find(chrom, '_') == chrom.end();
                             })) {
      spdlog::debug("[test get chroms] {}", chrom);
    }
  }

  TEST_CASE("test copy and move constructors") {
    VcfRanges<VcfRecord> vcf_ranges2(file_path);
    VcfRanges<VcfRecord> vcf_ranges_copy(vcf_ranges2);
    VcfRanges<VcfRecord> vcf_ranges_move(std::move(vcf_ranges2));
    CHECK_EQ(vcf_ranges_copy.file_path(), vcf_ranges_move.file_path());
  }

  TEST_CASE("test vcf query") { CHECK_NOTHROW(test_vcf_query(file_path)); }

  TEST_CASE("testing vcf.hpp") {
    SUBCASE("test file path") {
      CHECK_EQ(vcf_ranges.file_path(), file_path);
      CHECK(vcf_ranges.has_index_file());
    }
    SUBCASE("test iter vcf without index") {
      VcfRanges<VcfRecord> vcf_ranges3(uncompressed_file_path);
      CHECK_EQ(vcf_ranges3.has_index_file(), false);
      CHECK_NOTHROW(test_vcf_iter(std::string(uncompressed_file_path)));
    }
    SUBCASE("test query vcf without index") {
      CHECK_THROWS(test_vcf_query(std::string(uncompressed_file_path)));
    }
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
    spdlog::set_level(spdlog::level::debug);
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

    for (auto const& record : vcf_ranges | std::views::filter([](auto const& record) {
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
