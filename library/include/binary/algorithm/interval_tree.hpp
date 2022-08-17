//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_

#include <spdlog/fmt/bundled/core.h>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <memory>
#include <ranges>
namespace binary::algorithm::tree {

  /** Red Black Tree

  1. Every node is either red or black
  2. The root is black
  3. Every leaf(NIL) is black
  4. If a node is red, the both its children are black
  5. For each node, all simple paths from the node to
    descendant leaves contain the same number of black nodes.

  **/

  // TODO: Add Deletion and Test
  // TODO: Implement Interval Tree with information including chrom, pos, svtype.
  // TODO: Implement Interval Tree Query with Concurrency
  // TODO: Consider to use  Move shared_ptr  and Ref shared_ptr OR std::unique_ptr
  // TODO: constexpr constructor

  // define key constraints
  template <typename T>
  concept KeyConcept = std::totally_ordered<T> && std::default_initializable<T>;

  enum class Color { Red, Black };

  template <KeyConcept Key> class BaseNode {
  protected:
    using key_type = std::remove_cv_t<Key>;
    using pointer = std::unique_ptr<BaseNode>;
    using reference_pointer = pointer &;
    using raw_pointer = BaseNode *;

  public:
    constexpr BaseNode() = default;
    BaseNode(BaseNode &&other) noexcept = default;
    auto operator=(BaseNode &&other) noexcept -> BaseNode & = default;

    explicit constexpr BaseNode(key_type value) : key{value} {}
    explicit constexpr BaseNode(Color color) : color_{color} {}
    constexpr BaseNode(key_type value, Color color) : key{value}, color_{color} {}

    virtual ~BaseNode() = default;

    void set_color(Color color) { color_ = color; }
    [[nodiscard]] auto is_black() const -> bool { return color_ == Color::Black; }
    [[nodiscard]] auto is_red() const -> bool { return color_ == Color::Red; }

    // All return raw pointers in order to make api consistent
    [[nodiscard]] auto leftr() const -> raw_pointer { return left.get(); }
    [[nodiscard]] auto rightr() const -> raw_pointer { return right.get(); }

    Key key{};
    Color color_{Color::Black};
    pointer left{nullptr};
    pointer right{nullptr};
    raw_pointer parent{nullptr};
  };

  template <typename T>
  concept NodeConcept = std::movable<T> && std::default_initializable<T>;

  // NodeType must can default initialize and copyable
  template <NodeConcept NodeType> class RbTree {
  public:
    using pointer = typename NodeType::pointer;
    using reference_pointer = typename NodeType::reference_pointer;
    using raw_pointer = typename NodeType::raw_pointer;

    constexpr RbTree() = default;

    template <std::ranges::input_range R>
    requires std::constructible_from<NodeType, std::ranges::range_value_t<R>>
    void insert_node(R &&range) {
      for (auto &&item : range) {
        insert_node(std::forward<decltype(item)>(item));
      }
    }

    [[nodiscard]] auto size(raw_pointer node) const -> size_t;
    [[nodiscard]] auto size() const -> size_t;
    [[nodiscard]] auto root() const -> raw_pointer;

    // WARNING: Return raw pointer do not delete that!
    [[nodiscard]] auto minimum(raw_pointer node) const -> raw_pointer;
    [[nodiscard]] auto maximum(raw_pointer node) const -> raw_pointer;

    [[nodiscard]] auto successor(raw_pointer node) const -> raw_pointer;
    [[nodiscard]] auto predecessor(raw_pointer node) const -> raw_pointer;

    void inorder_walk(raw_pointer node) const;

    // Ownership transfer
    void insert_node(pointer node);

    template <typename... Args>
    requires std::constructible_from<NodeType, Args...>
    void insert_node(Args &&...args) {
      insert_node(std::make_unique<NodeType>(std::forward<Args>(args)...));
    }

  private:
    // Use reference pointer as smart pointer will be reset
    [[nodiscard]] bool check_is_red(raw_pointer node) const;
    void release_reset(reference_pointer to, raw_pointer source = nullptr) const;
    void left_rotate(raw_pointer node);
    void right_rotate(raw_pointer node);
    void fix_insert(raw_pointer node);

    void insert_node_impl(raw_pointer node);
    pointer root_{nullptr};
  };

  template <NodeConcept NodeType> auto RbTree<NodeType>::size() const -> size_t {
    return size(root_.get());
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::size(raw_pointer node) const -> size_t {
    if (node == nullptr) return 0;
    return 1 + size(node->leftr()) + size(node->rightr());
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::root() const -> raw_pointer {
    return root_.get();
  }

  template <NodeConcept NodeType> bool RbTree<NodeType>::check_is_red(raw_pointer node) const {
    if (node != nullptr && node->is_red()) {
      return true;
    }
    return false;
  }

  template <NodeConcept NodeType>
  void RbTree<NodeType>::release_reset(reference_pointer to, raw_pointer source) const {
    to.release();
    to.reset(source);
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::minimum(raw_pointer node) const
      -> raw_pointer {
    while (node->left != nullptr) {
      node = node->leftr();
    }
    return node;
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::maximum(raw_pointer node) const
      -> raw_pointer {
    while (node->right != nullptr) {
      node = node->rightr();
    }
    return node;
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::successor(raw_pointer node) const
      -> raw_pointer {
    if (node->right != nullptr) {
      return minimum(node->rightr());
    }

    auto *temp = node->parent;  // return raw pointer

    while (temp != nullptr && temp->rightr() == node) {
      // use move
      node = temp;
      temp = temp->parent;
    }

    return temp;
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::predecessor(raw_pointer node) const
      -> raw_pointer {
    if (node->left != nullptr) {
      return maximum(node->leftr());
    }

    auto *temp = node->parent;

    while (temp != nullptr && temp->leftr() == node) {
      node = temp;
      temp = temp->parent;
    }

    return temp;
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::left_rotate(raw_pointer node) {
    raw_pointer y = node->right.release();  // cannot be nil
    assert(y != nullptr);
    node->right.reset(y->left.release());

    if (node->right != nullptr) {
      node->right->parent = node;
    }

    y->parent = node->parent;

    if (node->parent == nullptr) {
      // release old root
      release_reset(root_, y);
    } else if (node == node->parent->leftr()) {
      // release old pointer do not delete
      release_reset(node->parent->left, y);
    } else {
      // release old pointer do not delete
      release_reset(node->parent->right, y);
    }

    y->left.reset(node);
    node->parent = y;
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::right_rotate(raw_pointer node) {
    auto y = node->left.release();
    assert(y != nullptr);
    node->left.reset(y->right.release());

    if (node->left != nullptr) {
      node->left->parent = node;
    }

    y->parent = node->parent;
    if (node->parent == nullptr) {
      // release old root and reset new root
      release_reset(root_, y);
    } else if (node == node->parent->leftr()) {
      release_reset(node->parent->left, y);
    } else {
      release_reset(node->parent->right, y);
    }

    y->right.reset(node);
    node->parent = y;
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::fix_insert(raw_pointer node) {
    while (check_is_red(node->parent)) {
      if (node->parent == node->parent->parent->leftr()) {
        raw_pointer y = node->parent->parent->rightr();
        if (check_is_red(y)) {
          // case 1
          node->parent->set_color(Color::Black);
          y->set_color(Color::Black);
          node->parent->parent->set_color(Color::Red);
          node = node->parent->parent;
        } else {
          if (node == node->parent->rightr()) {
            // case 2
            node = node->parent;
            left_rotate(node);
          }
          // case 3
          node->parent->set_color(Color::Black);
          node->parent->parent->set_color(Color::Red);
          right_rotate(node->parent->parent);
        }
      } else {
        raw_pointer y = node->parent->parent->leftr();
        if (check_is_red(y)) {
          node->parent->set_color(Color::Black);
          y->set_color(Color::Black);
          node->parent->parent->set_color(Color::Red);
          node = node->parent->parent;
        } else {
          if (node == node->parent->leftr()) {
            node = node->parent;
            right_rotate(node);
          }
          node->parent->set_color(Color::Black);
          node->parent->parent->set_color(Color::Red);
          left_rotate(node->parent->parent);
        }
      }
    }
    root_->set_color(Color::Black);
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::insert_node_impl(raw_pointer node) {
    raw_pointer x = root_.get();
    raw_pointer y = nullptr;

    while (x != nullptr) {
      y = x;
      if (node->key < x->key) {
        x = x->leftr();
      } else {
        x = x->rightr();
      }
    }

    node->parent = y;
    if (y == nullptr) {
      root_.reset(node);
    } else if (node->key < y->key) {
      y->left.reset(node);
    } else {
      y->right.reset(node);
    }

    node->set_color(Color::Red);
    fix_insert(node);
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::insert_node(pointer node) {
    insert_node_impl(node.release());
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::inorder_walk(raw_pointer node) const {
    if (node != nullptr) {
      inorder_walk(node->leftr());
      fmt::print("{} is_black {} ", node->key, node->is_black());
      inorder_walk(node->rightr());
    }
  }

  class IntNode : public BaseNode<int> {
  public:
    using BaseNode::pointer;
    using BaseNode::raw_pointer;
    using BaseNode::reference_pointer;

    constexpr IntNode() = default;
    using BaseNode::BaseNode;

    IntNode(IntNode &&other) noexcept = default;
    IntNode &operator=(IntNode &&other) noexcept = default;

    ~IntNode() override = default;

    auto operator<=>(IntNode const &other) const { return key <=> other.key; }
    // to meet equality_comparable
    friend bool operator==(IntNode const &lhs, IntNode const &rhs) { return lhs.key == rhs.key; }
  };

  class IntervalNode : public BaseNode<std::uint32_t> {
  public:
    using BaseNode::pointer;
    using BaseNode::raw_pointer;
    using BaseNode::reference_pointer;

    IntervalNode() = default;
    IntervalNode(IntervalNode &&other) noexcept = default;
    IntervalNode &operator=(IntervalNode &&other) noexcept = default;
    IntervalNode(key_type min, key_type max) : BaseNode(min), min_{min}, max_{max} {}

  private:
    key_type min_{};
    key_type max_{};

    // add chrom and other information
    // TODO: Consider how to change these information with key
  };

}  // namespace binary::algorithm::tree

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
