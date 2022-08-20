//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_

#include <binary/algorithm/rb_tree.hpp>

namespace binary::algorithm::tree {

  /** Interval Tree Based on Red Black Tree

  1. Every node is either red or black
  2. The root is black
  3. Every leaf(NIL) is black
  4. If a node is red, the both its children are black
  5. For each node, all simple paths from the node to
    descendant leaves contain the same number of black nodes.
  **/

  // TODO: Implement Interval Tree with information including chrom, pos, svtype.
  // TODO: Implement Interval Tree Query with Concurrency

  /** Interval Node and Interval Tree */

  template <typename Interval>
  concept IntervalConcept = requires(Interval const &interval) {
    requires std::semiregular<Interval>;
    requires std::same_as<decltype(interval.low), decltype(interval.high)>;
    interval.low <= interval.high;
  };

  template <IntervalConcept Interval, KeyConcept KeyType = std::uint32_t> class IntervalNode
      : public BaseNode<KeyType> {
  public:
    using raw_pointer = IntervalNode *;
    using pointer = std::unique_ptr<IntervalNode>;
    using reference_pointer = std::unique_ptr<IntervalNode> &;
    using key_type = KeyType;

    IntervalNode() = default;

    IntervalNode(IntervalNode const &other) = default;
    IntervalNode &operator=(IntervalNode const &other) = default;
    IntervalNode(IntervalNode &&other) noexcept = default;
    IntervalNode &operator=(IntervalNode &&other) noexcept = default;

    explicit IntervalNode(Interval const &interval_)
        : BaseNode<KeyType>(interval_.low), interval{interval_}, max{interval_.high} {}

    explicit IntervalNode(Interval &&interval_)
        : BaseNode<KeyType>(interval_.low), max{interval_.high}, interval{std::move(interval_)} {}

    ~IntervalNode() override = default;

    void copy_key(const raw_pointer other) noexcept {
      interval = other->interval;
      max = other->max;
    }

    KeyType max{};
    Interval interval{};
  };

  template <typename Node>
  concept IntervalNodeConcept = requires(Node &node) {
    requires NodeConcept<Node>;
    node.max;
    node.interval;
  };

  template <IntervalNodeConcept NodeType> class IntervalTree : public RbTree<NodeType> {
  public:
    using typename RbTree<NodeType>::raw_pointer;
    using typename RbTree<NodeType>::pointer;
    using typename RbTree<NodeType>::reference_pointer;
    using typename NodeType::key_type;

    void delete_node(raw_pointer node);

  private:
    void left_rotate(raw_pointer node);
    void right_rotate(raw_pointer node);
    auto transplant(raw_pointer target, raw_pointer source) -> raw_pointer;
    void insert_node_impl(raw_pointer node);

    auto get_max(raw_pointer node) const;
    void update_max(raw_pointer node, key_type key) const;
    void update_max(raw_pointer node) const;

    using RbTree<NodeType>::root_;
    using RbTree<NodeType>::nil_;
  };

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::left_rotate(raw_pointer node) {
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

    // update argument info
    update_max(y, node->max);
    update_max(node);
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::right_rotate(raw_pointer node) {
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

    // update argument info
    update_max(y, node->max);
    update_max(node);
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::insert_node_impl(raw_pointer node) {
    raw_pointer x = root_.get();
    raw_pointer y = nullptr;

    while (x != nullptr) {
      y = x;

      // update argument info
      update_max(x, node->key);

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

  template <IntervalNodeConcept NodeType>
  auto IntervalTree<NodeType>::transplant(raw_pointer target, raw_pointer source) -> raw_pointer {
    auto ret = target->parent;
    if (source != nullptr) {
      source->parent = ret;
    }
    if (target->parent == nullptr) {
      root_.reset(source);  // delete target and reset root
    } else if (target == target->parent->leftr()) {
      target->parent->left.reset(source);  // delete target and reset left
    } else {
      target->parent->right.reset(source);  // delete target and reset right
    }

    return ret;
  }

  template <IntervalNodeConcept NodeType>
  auto IntervalTree<NodeType>::get_max(raw_pointer node) const {
    if (node == nullptr) {
      return std::numeric_limits<key_type>::lowest();
    }
    return node->max;
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::update_max(raw_pointer node, key_type key) const {
    node->max = std::max(node->max, key);
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::update_max(raw_pointer node) const {
    node->max = std::max(node->interval.high, get_max(node->leftr()), get_max(node->rightr()));
  }

  class VcfInvterval {
  public:
    std::uint32_t low;
    std::uint32_t high;

    // TODO: Add vcfrecord or enough info to the class
  };

}  // namespace binary::algorithm::tree

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
