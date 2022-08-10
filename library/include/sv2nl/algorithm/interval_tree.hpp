//
// Created by li002252 on 8/3/22.
//

#ifndef SV2NL_LIBRARY_INCLUDE_SV2NL_INTERVAL_TREE_HPP_
#define SV2NL_LIBRARY_INCLUDE_SV2NL_INTERVAL_TREE_HPP_

#include <cassert>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <memory>
namespace sv2nl::algorithm::tree {

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
  // TODO: Use concepts
  // TODO: Conciser to use  Move shared_ptr  and Ref shared_ptr

  // define key constraints
  template <typename T>
  concept key_concepts = std::totally_ordered<T> && std::default_initializable<T>;

  enum class Color { Red, Black };

  template <key_concepts Key> class BaseNode {
  protected:
    using key_type = Key;
    using pointer_type = std::shared_ptr<BaseNode>;

    Key key_{};
    Color color_{Color::Black};
    pointer_type left_{nullptr};
    pointer_type right_{nullptr};
    std::weak_ptr<BaseNode> parent_{};

  public:
    BaseNode() = default;
    explicit BaseNode(key_type key) : key_{key} {}
    explicit BaseNode(Color color) : color_{color} {}
    BaseNode(key_type key, Color color) : key_{key}, color_{color} {}

    virtual ~BaseNode() = default;

    void set_color(Color color) { color_ = color; }
    [[nodiscard]] auto is_black() const -> bool { return color_ == Color::Black; }
    [[nodiscard]] auto is_red() const -> bool { return color_ == Color::Red; }

    [[nodiscard]] auto key() const -> key_type const & { return key_; }
    [[nodiscard]] auto left() const -> pointer_type { return left_; }
    [[nodiscard]] auto right() const -> pointer_type { return right_; }
    [[nodiscard]] auto parent() const -> pointer_type {
      assert(!parent_.expired());
      return parent_.lock();
    }

    void set_left(pointer_type left) { left_ = left; }
    void set_right(pointer_type right) { right_ = right; }
    void set_parent(pointer_type parent) { parent_ = parent; }
  };

  // NodeType must can default initialize and copyable
  template <std::regular NodeType> class RbTree {
  public:
    using node_type = NodeType;
    using pointer_type = typename NodeType::pointer_type;

    RbTree() = default;

    [[nodiscard]] auto nil() const -> pointer_type { return nil_; }
    [[nodiscard]] auto root() const -> pointer_type { return root_; }
    [[nodiscard]] auto size(pointer_type node) const -> size_t;
    [[nodiscard]] auto size() const -> size_t;

    [[nodiscard]] auto minimum(pointer_type node) const -> pointer_type;
    [[nodiscard]] auto maximum(pointer_type node) const -> pointer_type;

    [[nodiscard]] auto successor(pointer_type node) const -> pointer_type;
    [[nodiscard]] auto predecessor(pointer_type node) const -> pointer_type;

    void inorder_walk(pointer_type node) const;

    void insert_node(pointer_type node);

    template <typename... Args>
    requires std::constructible_from<node_type, Args...>
    void insert_node(Args &&...args) {
      insert_node(std::make_shared<node_type>(std::forward<Args>(args)...));
    }

  private:
    void left_rotate(pointer_type node);
    void right_rotate(pointer_type node);
    void fix_insert(pointer_type node);
    void insert_node_impl(pointer_type node);

    pointer_type nil_{std::make_shared<NodeType>()};  // only one nil to save space
    pointer_type root_{nil_};
  };

  template <std::regular NodeType> auto RbTree<NodeType>::size(pointer_type node) const -> size_t {
    if (node == nil_) return 0;
    return 1 + size(node->left()) + size(node->right());
  }

  template <std::regular NodeType> auto RbTree<NodeType>::minimum(pointer_type node) const
      -> pointer_type {
    while (node->left() != nil_) {
      node = node->left();
    }
    return node;
  }

  template <std::regular NodeType> auto RbTree<NodeType>::maximum(pointer_type node) const
      -> pointer_type {
    while (node->right() != nil_) {
      node = node->right();
    }
    return node;
  }

  template <std::regular NodeType> auto RbTree<NodeType>::successor(pointer_type node) const
      -> pointer_type {
    if (node->right() != nil_) {
      return minimum(node->right());
    }

    pointer_type temp = node->parent();

    while (temp != nil_ && temp->right() == node) {
      // use move
      node = std::move(temp);
      temp = node->parent();
    }
    return temp;
  }

  template <std::regular NodeType> auto RbTree<NodeType>::predecessor(pointer_type node) const
      -> pointer_type {
    if (node->left() != nil_) {
      return maximum(node->left());
    }

    pointer_type temp = node->parent();

    while (temp != nil_ && temp->left() == node) {
      node = std::move(temp);
      temp = node->parent();
    }

    return temp;
  }

  template <std::regular NodeType> void RbTree<NodeType>::left_rotate(pointer_type node) {
    pointer_type y = node->right();  // cannot be nil
    assert(y != nil_);
    node->set_right(y->left());
    if (y->left() != nil_) {
      y->left()->set_parent(node);
    }
    y->set_parent(node->parent());
    if (node->parent() == nil_) {
      root_ = y;
    } else if (node == node->parent()->left()) {
      node->parent()->set_left(y);
    } else {
      node->parent()->set_right(y);
    }
    y->set_left(node);
    node->set_parent(y);
  }

  template <std::regular NodeType> void RbTree<NodeType>::right_rotate(pointer_type node) {
    pointer_type y = node->left();  // cannot be nil
    assert(y != nil_);
    node->set_left(y->right());
    if (y->right() != nil_) {
      y->right()->set_parent(node);
    }
    y->set_parent(node->parent());
    if (node->parent() == nil_) {
      root_ = y;
    } else if (node == node->parent()->left()) {
      node->parent()->set_left(y);
    } else {
      node->parent()->set_right(y);
    }
    y->set_right(node);
    node->set_parent(y);
  }

  template <std::regular NodeType> void RbTree<NodeType>::fix_insert(pointer_type node) {
    while (node->parent()->is_red()) {
      if (node->parent() == node->parent()->parent()->left()) {
        pointer_type y = node->parent()->parent()->right();
        if (y->is_red()) {
          // case 1
          node->parent()->set_color(Color::Black);
          y->set_color(Color::Black);
          node->parent()->parent()->set_color(Color::Red);
          node = node->parent()->parent();
        } else {
          if (node == node->parent()->right()) {
            // case 2
            node = node->parent();
            left_rotate(node);
          }
          // case 3
          node->parent()->set_color(Color::Black);
          node->parent()->parent()->set_color(Color::Red);
          right_rotate(node->parent()->parent());
        }
      } else {
        pointer_type y = node->parent()->parent()->left();
        if (y->is_red()) {
          node->parent()->set_color(Color::Black);
          y->set_color(Color::Black);
          node->parent()->parent()->set_color(Color::Red);
          node = node->parent()->parent();
        } else {
          if (node == node->parent()->left()) {
            node = node->parent();
            right_rotate(node);
          }
          node->parent()->set_color(Color::Black);
          node->parent()->parent()->set_color(Color::Red);
          left_rotate(node->parent()->parent());
        }
      }
    }
    root_->set_color(Color::Black);
  }

  template <std::regular NodeType> void RbTree<NodeType>::insert_node_impl(pointer_type node) {
    pointer_type x = root_;
    pointer_type y = nil_;
    while (x != nil_) {
      y = x;
      if (node->key() < x->key()) {
        x = x->left();
      } else {
        x = x->right();
      }
    }
    node->set_parent(y);
    if (y == nil_) {
      root_ = node;
    } else if (node->key() < y->key()) {
      y->set_left(node);
    } else {
      y->set_right(node);
    }
    node->set_left(nil_);
    node->set_right(nil_);
    node->set_color(Color::Red);
    fix_insert(node);
  }

  template <std::regular NodeType> void RbTree<NodeType>::insert_node(pointer_type node) {
    insert_node_impl(std::move(node));
  }

  template <std::regular NodeType> void RbTree<NodeType>::inorder_walk(pointer_type node) const {
    if (node != nil_) {
      inorder_walk(node->left());
      std::cout << node->key() << ' ' << node->is_black() << '\n';
      inorder_walk(node->right());
    }
  }
  template <std::regular NodeType> auto RbTree<NodeType>::size() const -> size_t {
    return size(root_);
  }

  class IntNode : public BaseNode<int> {
  public:
    using BaseNode::pointer_type;

    IntNode() = default;
    using BaseNode::BaseNode;

    ~IntNode() override = default;

    auto operator<=>(IntNode const &other) const { return key_ <=> other.key_; }
    // to meet equality_comparable
    friend bool operator==(IntNode const &lhs, IntNode const &rhs) { return lhs.key_ == rhs.key_; }
  };

  class IntervalNode : public BaseNode<std::uint32_t> {
  public:
    using BaseNode::pointer_type;

    IntervalNode() = default;
    IntervalNode(key_type min, key_type max) : BaseNode(min), min_{min}, max_{max} {}

  private:
    [[maybe_unused]] key_type min_{};
    [[maybe_unused]] key_type max_{};

    // add chrom and other information
    // TODO: Consider how to change these information with key
  };
}  // namespace sv2nl::algorithm::tree

#endif  // SV2NL_LIBRARY_INCLUDE_SV2NL_INTERVAL_TREE_HPP_
