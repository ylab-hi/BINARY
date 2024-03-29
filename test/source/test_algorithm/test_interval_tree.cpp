//
// Created by li002252 on 8/20/22.
//

#include <binary/algorithm/all.hpp>
#include <binary/utils.hpp>

#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <algorithm>
#include <array>
#include <filesystem>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

TEST_SUITE("algorithm-interval-tree") {
  using namespace binary::algorithm::tree;
  int check_black_height(auto const& root) {
    if (root == nullptr) {
      return 0;
    }

    int temp = check_black_height(root->leftr());
    if (temp != check_black_height(root->rightr())) {
      throw std::invalid_argument("black height mismatch");
    }

    return static_cast<int>(root->is_black()) + temp;
  }

  TEST_CASE("test construct interval") {
    UIntInterval int_interval{};
    CHECK_EQ(int_interval.low, 0);
    CHECK_EQ(int_interval.high, 0);

    UIntInterval int_interval2{1, 2};
    CHECK_EQ(int_interval2.low, 1);
    CHECK_EQ(int_interval2.high, 2);
  }

  TEST_CASE("test construct interval node") {
    SUBCASE("test construct for uint node from  args and left value") {
      UIntIntervalNode int_interval_node{1u, 10u};
      CHECK_EQ(int_interval_node.interval.low, 1);
      CHECK_EQ(int_interval_node.interval.high, 10);
      CHECK_EQ(int_interval_node.max, 10);

      UIntInterval int_interval{1u, 2u};
      UIntIntervalNode int_interval_node2{int_interval};
      CHECK_EQ(int_interval_node2.interval.low, 1);
      CHECK_EQ(int_interval_node2.interval.high, 2);
      CHECK_EQ(int_interval_node2.max, 2);
    }

    SUBCASE("test construct for uint  node from rvalue node") {
      UIntInterval int_interval{100u, 2000u};
      UIntIntervalNode int_interval_node{int_interval};
      CHECK_EQ(int_interval_node.interval.low, 100);
      CHECK_EQ(int_interval_node.interval.high, 2000);
      CHECK_EQ(int_interval_node.max, 2000);

      UIntIntervalNode int_interval_node3{{1u, 20u}};
      CHECK_EQ(int_interval_node3.interval.low, 1);
      CHECK_EQ(int_interval_node3.interval.high, 20);
      CHECK_EQ(int_interval_node3.max, 20);
    }

    IntervalNode<IntInterval> interval_node{1, 10};
    CHECK_EQ(interval_node.interval.low, 1);
    CHECK_EQ(interval_node.interval.high, 10);
    CHECK_EQ(interval_node.max, 10);
  }

  TEST_CASE("test construct interval tree") {
    SUBCASE("test insert node from args") {
      IntervalTree<UIntIntervalNode> interval_tree{};
      interval_tree.insert_node(16u, 21u);
      CHECK_EQ(interval_tree.size(), 1);
    }
    SUBCASE("test insert node from args  int node") {
      IntervalTree<IntIntervalNode> interval_tree{};
      interval_tree.insert_node(16, 21);
      CHECK_EQ(interval_tree.size(), 1);
    }
  }

  TEST_CASE("test insert multiple nodes") {
    std::array<UIntInterval, 10> nodes{UIntInterval(16u, 21u), UIntInterval(8u, 9u),
                                       UIntInterval(5u, 8u),   UIntInterval(0u, 3u),
                                       UIntInterval(6u, 10u),  UIntInterval(15u, 23u),
                                       UIntInterval(25u, 30u), UIntInterval(17u, 19u),
                                       UIntInterval(19u, 20u), UIntInterval(26u, 26u)};

    IntervalTree<UIntIntervalNode> interval_tree{};
    interval_tree.insert_node(nodes);
    CHECK_EQ(interval_tree.size(), nodes.size());
    CHECK_EQ(interval_tree.root()->key, 16);
    CHECK_NOTHROW(check_black_height(interval_tree.root()));
  }

  TEST_CASE("test insert nodes fuzzy test") {
    IntervalTree<IntIntervalNode> interval_tree{};

    for (int i = 0; i < 1000; i += 2) {
      interval_tree.insert_node(i, i + 3);
    }
    CHECK_EQ(interval_tree.size(), 500);
    CHECK_NOTHROW(check_black_height(interval_tree.root()));
  }

  TEST_CASE("test find overlap") {
    std::array<UIntInterval, 10> nodes{UIntInterval(16u, 21u), UIntInterval(8u, 9u),
                                       UIntInterval(5u, 8u),   UIntInterval(0u, 3u),
                                       UIntInterval(6u, 10u),  UIntInterval(15u, 23u),
                                       UIntInterval(25u, 30u), UIntInterval(17u, 19u),
                                       UIntInterval(19u, 20u), UIntInterval(26u, 26u)};
    IntervalTree<UIntIntervalNode> interval_tree{};
    interval_tree.insert_node(nodes);

    SUBCASE("test find single overlap") {
      auto intervals = interval_tree.find_overlap(22u, 25u);

      CHECK(intervals.has_value());
      CHECK_EQ(intervals->low, 15u);
      CHECK_EQ(intervals->high, 23u);

      auto non_found = interval_tree.find_overlap(UIntInterval{100u, 111u});
      CHECK_FALSE(non_found.has_value());
    }

    SUBCASE("test find multiple overlaps case 1") {
      auto intervals1 = interval_tree.find_overlaps(UIntInterval{7u, 25u});
      CHECK_EQ(intervals1.size(), 8);

      auto intervals2 = interval_tree.find_overlaps(15u, 25u);
      CHECK_EQ(intervals2.size(), 5);
    }

    SUBCASE("test to dot") {
      CHECK_NOTHROW(interval_tree.to_dot("interval_tree.dot"));
      CHECK(std::filesystem::exists("interval_tree.dot"));
      CHECK_NOTHROW(std::filesystem::remove("interval_tree.dot"));
    }
  }

  TEST_CASE("test same interval value") {
    IntervalTree<UIntIntervalNode> interval_tree{};
    interval_tree.insert_node(1u, 4u);
    interval_tree.insert_node(1u, 4u);
    interval_tree.insert_node(1u, 4u);
    interval_tree.insert_node(1u, 4u);
    CHECK_EQ(interval_tree.size(), 4);
    auto res = interval_tree.find_overlaps(2u, 5u);
    CHECK_EQ(res.size(), 4);
  }

  TEST_CASE("test delete") {}
}