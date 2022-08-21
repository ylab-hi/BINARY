//
// Created by li002252 on 8/20/22.
//
#include "binary/algorithm/all.hpp"
#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <algorithm>
#include <array>
#include <random>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

TEST_SUITE("algorithm-interval-tree") {
  using namespace binary::algorithm::tree;
  TEST_CASE("test construct interval") {
    IntInterval int_interval{};
    CHECK_EQ(int_interval.low, 0);
    CHECK_EQ(int_interval.high, 0);

    IntInterval int_interval2{1, 2};
    CHECK_EQ(int_interval2.low, 1);
    CHECK_EQ(int_interval2.high, 2);
  }

  TEST_CASE("test construct interval node") {
    SUBCASE("test construct for uint node from  args and left value") {
      IntIntervalNode int_interval_node{1u, 10u};
      CHECK_EQ(int_interval_node.interval.low, 1);
      CHECK_EQ(int_interval_node.interval.high, 10);
      CHECK_EQ(int_interval_node.max, 10);

      IntInterval int_interval{1u, 2u};
      IntIntervalNode int_interval_node2{int_interval};
      CHECK_EQ(int_interval_node2.interval.low, 1);
      CHECK_EQ(int_interval_node2.interval.high, 2);
      CHECK_EQ(int_interval_node2.max, 2);
    }

    SUBCASE("test construct for uint  node from rvalue node") {
      IntInterval int_interval{100u, 2000u};
      IntIntervalNode int_interval_node{std::move(int_interval)};
      CHECK_EQ(int_interval_node.interval.low, 100);
      CHECK_EQ(int_interval_node.interval.high, 2000);
      CHECK_EQ(int_interval_node.max, 2000);

      IntIntervalNode int_interval_node3{{1u, 20u}};
      CHECK_EQ(int_interval_node3.interval.low, 1);
      CHECK_EQ(int_interval_node3.interval.high, 20);
      CHECK_EQ(int_interval_node3.max, 20);
    }

    IntervalNode<BaseInterval<int>> interval_node{1, 10};
    CHECK_EQ(interval_node.interval.low, 1);
    CHECK_EQ(interval_node.interval.high, 10);
    CHECK_EQ(interval_node.max, 10);
  }

  TEST_CASE("test construct interval tree") { IntervalTree<IntIntervalNode> interval_tree{}; }

  TEST_CASE("test insert node") {}

  TEST_CASE("test delete") {}

  TEST_CASE("test find overlaps") {}
}