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

  TEST_CASE("test construct interval node") { IntIntervalNode int_interval_node{}; }

  TEST_CASE("test delete") {}

  TEST_CASE("test find overlaps") {}
}