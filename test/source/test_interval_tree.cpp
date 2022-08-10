//
// Created by li002252 on 8/10/22.
//
#include <doctest/doctest.h>

#include <sv2nl/algorithm.hpp>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <vector>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

TEST_CASE("test for red black tree") {
  using namespace sv2nl::algorithm::tree;
  RbTree<IntNode> rb_tree{};
  std::vector<int> keys{3,  7,  10, 12, 14, 15, 16, 17, 19, 20, 29,
                        21, 23, 26, 28, 30, 35, 38, 39, 41, 47};

  SUBCASE("test for inserting shareptr node") {
    rb_tree.insert_node(std::make_shared<IntNode>(1));
    rb_tree.insert_node(std::make_shared<IntNode>(2));
    rb_tree.insert_node(std::make_shared<IntNode>(4));
    CHECK_EQ(rb_tree.size(), 3);
    CHECK_EQ(rb_tree.root()->key(), 2);
  }

  SUBCASE("test for inserting arg node") {
    rb_tree.insert_node(1);
    rb_tree.insert_node(2);
    rb_tree.insert_node(4);
    CHECK_EQ(rb_tree.size(), 3);
    CHECK_EQ(rb_tree.root()->key(), 2);
  }

  SUBCASE("test for successor and predecessor") {
    rb_tree.insert_node(1);
    rb_tree.insert_node(2);
    rb_tree.insert_node(4);
    auto root = rb_tree.root();
    auto left = root->left();
    auto right = root->right();
    CHECK_EQ(rb_tree.successor(root)->key(), 4);
    CHECK_EQ(rb_tree.predecessor(root)->key(), 1);
    CHECK_EQ(left->parent(), root);
    CHECK_EQ(right->parent(), root);
  }

  // TODO: test for left rotate and right rotate

  SUBCASE("test for inserting multiple nodes") {
    for (auto key : keys) {
      rb_tree.insert_node(key);
    }
    CHECK_EQ(rb_tree.size(), keys.size());
    CHECK_EQ(rb_tree.root()->key(), 17);
  }

  SUBCASE("test for inserting multiple nodes fuzzy case") {
    int counter = 0;
    do {
      RbTree<IntNode> tree{};
      for (auto key : keys) {
        tree.insert_node(key);
      }
      ++counter;
      CHECK_EQ(tree.size(), keys.size());
    } while (std::next_permutation(keys.begin(), keys.end()) && counter < 66);
  }
}