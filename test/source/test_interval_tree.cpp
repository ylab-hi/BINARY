//
// Created by li002252 on 8/10/22.
//
#include <doctest/doctest.h>

#include <binary/algorithm/all.hpp>

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <algorithm>
#include <array>
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
  TEST_CASE("test shared ptr for red black tree insert") {
    using namespace binary::algorithm::tree::v1_shared_ptr;
    RbTree<IntNode> rb_tree{};
    std::array<int, 21> keys{3,  7,  10, 12, 14, 15, 16, 17, 19, 20, 29,
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

  int black_height2(auto const& root) {
    if (root == nullptr) {
      return 0;
    }

    int temp = black_height2(root->leftr());
    if (temp != black_height2(root->rightr())) {
      throw std::invalid_argument("black height mismatch");
    }

    return static_cast<int>(root->is_black()) + temp;
  }

  TEST_CASE("test unique ptr black red tree for insert") {
    using namespace binary::algorithm::tree;
    IntNode int_node1{2};
    IntNode int_node2{4};
    IntNode int_node3{std::move(int_node1)};
    RbTree<IntNode> rb_tree{};
    std::array<int, 21> keys{3,  7,  10, 12, 14, 15, 16, 17, 19, 20, 29,
                             21, 23, 26, 28, 30, 35, 38, 39, 41, 47};

    SUBCASE("test for inserting one node") {
      rb_tree.insert_node(std::make_unique<IntNode>(1));
      rb_tree.insert_node(std::make_unique<IntNode>(2));
      rb_tree.insert_node(std::make_unique<IntNode>(4));
    }

    SUBCASE("test for inserting shareptr node") {
      for (auto k : {1, 2, 4}) {
        rb_tree.insert_node(std::make_unique<IntNode>(k));
      }
      CHECK_EQ(rb_tree.size(), 3);
      CHECK_EQ(rb_tree.root()->key, 2);
      CHECK_EQ(black_height2(rb_tree.root()), 1);
    }

    SUBCASE("test for inserting arg node") {
      for (auto k : {1, 2, 7, 4, 10}) {
        rb_tree.insert_node(k);
      }
      CHECK_EQ(rb_tree.size(), 5);
      CHECK_EQ(rb_tree.root()->key, 2);
      CHECK_EQ(black_height2(rb_tree.root()), 2);
    }

    SUBCASE("test for successor and predecessor") {
      for (auto k : {1, 2, 4}) {
        rb_tree.insert_node(k);
      }
      auto root = rb_tree.root();
      auto left = root->leftr();
      auto right = root->rightr();
      CHECK_EQ(rb_tree.successor(root)->key, 4);
      CHECK_EQ(rb_tree.predecessor(root)->key, 1);
      CHECK_EQ(left->parent, root);
      CHECK_EQ(right->parent, root);
    }

    SUBCASE("test for inserting multiple nodes") {
      for (auto key : keys) {
        rb_tree.insert_node(key);
      }
      CHECK_EQ(rb_tree.size(), keys.size());
      CHECK_EQ(rb_tree.root()->key, 17);
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
        CHECK_NOTHROW(black_height2(tree.root()));
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
        CHECK_NOTHROW(black_height2(tree.root()));
      } while (counter > 0);
    }

    SUBCASE("test range constructor from args") {
      std::vector<int> vkeys{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      RbTree<IntNode> tree{};
      tree.insert_node(vkeys);
    }
  }

  TEST_CASE("test unique ptr red black tree for delete") {
    using namespace binary::algorithm::tree;

    RbTree<IntNode> rb_tree{};
    std::array<int, 21> keys{3,  7,  10, 12, 14, 15, 16, 17, 19, 20, 29,
                             21, 23, 26, 28, 30, 35, 38, 39, 41, 47};

    SUBCASE("test for deleting one node from smart ptr") {
      for (auto k : {1, 2, 4}) {
        rb_tree.insert_node(k);
      }
      auto& left_child = rb_tree.root()->left;
      rb_tree.delete_node(left_child);
      CHECK_EQ(rb_tree.size(), 2);
    }

    SUBCASE("test for deleting one node from raw ptr") {
      RbTree<IntNode> tree{};

      tree.insert_node(keys);

      CHECK_EQ(tree.size(), 21);
      CHECK_NOTHROW(black_height2(tree.root()));
      CHECK_EQ(tree.root()->key, 17);
      tree.delete_node(tree.root());
      tree.delete_node(tree.root());
      CHECK_EQ(tree.size(), 19);
    }

    SUBCASE("test for deleting multi nodes from raw ptr") {
      RbTree<IntNode> tree{};
      tree.insert_node(keys);

      CHECK_EQ(tree.size(), 21);
      CHECK_NOTHROW(black_height2(tree.root()));

      for ([[maybe_unused]] auto i : keys) {
        tree.delete_node(tree.root());
      }

      CHECK(tree.empty());
    }

    SUBCASE("test for delete random key fuzzy case") {
      constexpr int kNum = 20;
      int counter = 1;
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
          std::cout << ", " << key;
        }
        std::cout << '\n';
        --counter;
        CHECK_EQ(tree.size(), random_keys.size());
        CHECK_NOTHROW(black_height2(tree.root()));
      } while (counter > 0);
    }
  }

  TEST_CASE("test for issue  use deleted memory") {
    using namespace binary::algorithm::tree;
    // commit: 61ef34c - fix twice delete issue
    std::array<int, 20> k1{54942, 75803, 49212, 64167, 14933, 44543, 10072, 90303, 45511, 70641,
                           59710, 3100,  98544, 55068, 45575, 4994,  66267, 24721, 17128, 72975};

    RbTree<IntNode> tree{};
    tree.insert_node(k1);
    CHECK_EQ(tree.size(), 20);
    CHECK_NOTHROW(black_height2(tree.root()));
    tree.inorder_walk(tree.root());
    for ([[maybe_unused]] auto i : k1) {
      tree.delete_node(tree.root());
    }
    CHECK(tree.empty());
  }
}