//
// Created by li002252 on 8/10/22.
//
#include <doctest/doctest.h>

#include <sv2nl/algorithm.hpp>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <random>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

int black_height(auto const& root, auto const& nil) {
  if (root == nil) {
    return 0;
  }

  int temp = black_height(root->left(), nil);
  if (temp != black_height(root->right(), nil)) {
    throw std::invalid_argument("black height mismatch");
  }

  return static_cast<int>(root->is_black()) + temp;
}

TEST_SUITE("algorithm") {
  TEST_CASE("test for red black tree") {
    using namespace sv2nl::algorithm::tree;
    RbTree<IntNode> rb_tree{};
    std::array keys{3,  7,  10, 12, 14, 15, 16, 17, 19, 20, 29,
                    21, 23, 26, 28, 30, 35, 38, 39, 41, 47};

    SUBCASE("test for inserting shareptr node") {
      for (auto k : {1, 2, 4}) {
        rb_tree.insert_node(std::make_shared<IntNode>(k));
      }
      CHECK_EQ(rb_tree.size(), 3);
      CHECK_EQ(rb_tree.root()->key(), 2);
      CHECK_EQ(black_height(rb_tree.root(), rb_tree.nil()), 1);
    }

    SUBCASE("test for inserting arg node") {
      for (auto k : {1, 2, 7, 4, 10}) {
        rb_tree.insert_node(k);
      }
      CHECK_EQ(rb_tree.size(), 5);
      CHECK_EQ(rb_tree.root()->key(), 2);
      CHECK_EQ(black_height(rb_tree.root(), rb_tree.nil()), 2);
    }

    SUBCASE("test for successor and predecessor") {
      for (auto k : {1, 2, 4}) {
        rb_tree.insert_node(k);
      }
      auto root = rb_tree.root();
      auto left = root->left();
      auto right = root->right();
      CHECK_EQ(rb_tree.successor(root)->key(), 4);
      CHECK_EQ(rb_tree.predecessor(root)->key(), 1);
      CHECK_EQ(left->parent(), root);
      CHECK_EQ(right->parent(), root);
    }

    SUBCASE("test for inserting multiple nodes") {
      for (auto key : keys) {
        rb_tree.insert_node(key);
      }
      CHECK_EQ(rb_tree.size(), keys.size());
      CHECK_EQ(rb_tree.root()->key(), 17);
    }

    SUBCASE("test for inserting multiple nodes fuzzy case") {
      int counter = 42;
      do {
        RbTree<IntNode> tree{};
        for (auto key : keys) {
          tree.insert_node(key);
        }
        --counter;
        CHECK_EQ(tree.size(), keys.size());
        CHECK_NOTHROW(black_height(tree.root(), tree.nil()));
      } while (std::next_permutation(keys.begin(), keys.end()) && counter > 0);
    }

    SUBCASE("test for inserting random key fuzzy case") {
      constexpr int kNum = 5000;
      int counter = 10;
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(1, 100000);
      std::vector<int> random_keys(kNum);

      do {
        std::generate(random_keys.begin(), random_keys.end(), [&]() { return dis(gen); });
        random_keys.erase(std::unique(random_keys.begin(), random_keys.end()), random_keys.end());
        RbTree<IntNode> tree{};
        for (auto key : random_keys) {
          tree.insert_node(key);
        }
        --counter;
        CHECK_EQ(tree.size(), random_keys.size());
        CHECK_NOTHROW(black_height(tree.root(), tree.nil()));
      } while (counter > 0);
    }
  }
}