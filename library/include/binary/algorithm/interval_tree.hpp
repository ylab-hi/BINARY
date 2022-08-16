//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_

#include <cassert>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <memory>
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
  // TODO: Use concepts
  // TODO: Consider to use  Move shared_ptr  and Ref shared_ptr OR std::unique_ptr
  // TODO: constexpr constructor

  inline namespace v1 {
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

    template <std::regular NodeType> auto RbTree<NodeType>::size(pointer_type node) const
        -> size_t {
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

  }  // namespace v1

  // WARNING: this is not a complete implementation of a red-black tree.

  namespace v2 {
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

      Key key_{};
      Color color_{Color::Black};

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

      // All return raw pointers in order to make api consistent
      [[nodiscard]] auto leftr() const -> raw_pointer { return left.get(); }
      [[nodiscard]] auto rightr() const -> raw_pointer { return right.get(); }
      [[nodiscard]] auto partentr() const -> raw_pointer { return parent; }

      pointer left{nullptr};
      pointer right{nullptr};
      raw_pointer parent{nullptr};
    };

    // NodeType must can default initialize and copyable
    template <std::regular NodeType> class RbTree {
    public:
      using node_type = NodeType;
      using pointer = typename NodeType::pointer;
      using reference_pointer = typename NodeType::reference_pointer;
      using raw_pointer = typename NodeType::raw_pointer;

      RbTree() = default;

      [[nodiscard]] auto size(raw_pointer node) const -> size_t;
      [[nodiscard]] auto size() const -> size_t;

      // WARNING: Return raw pointer do not delete that!
      [[nodiscard]] auto minimum(raw_pointer node) const -> raw_pointer;
      [[nodiscard]] auto maximum(raw_pointer node) const -> raw_pointer;

      [[nodiscard]] auto successor(raw_pointer node) const -> raw_pointer;
      [[nodiscard]] auto predecessor(raw_pointer node) const -> raw_pointer;

      void inorder_walk(raw_pointer node) const;

      // inorder walk from root_
      void inorder_walk() const;

      // Ownership transfer
      void insert_node(pointer node);

      template <typename... Args>
      requires std::constructible_from<node_type, Args...>
      void insert_node(Args &&...args) {
        insert_node(std::make_unique<node_type>(std::forward<Args>(args)...));
      }

    private:
      // Use reference pointer as smart pointer will be reset
      void left_rotate(reference_pointer node);
      void right_rotate(reference_pointer node);
      void fix_insert(reference_pointer node);

      void insert_node_impl(pointer node);

    public:
      pointer root{nullptr};
    };

    template <std::regular NodeType> auto RbTree<NodeType>::size(raw_pointer node) const -> size_t {
      if (node == nullptr) return 0;
      return 1 + size(node->leftr()) + size(node->rightr());
    }

    template <std::regular NodeType> auto RbTree<NodeType>::minimum(raw_pointer node) const
        -> raw_pointer {
      while (node->left != nullptr) {
        node = node->leftr();
      }
      return node;
    }

    template <std::regular NodeType> auto RbTree<NodeType>::maximum(raw_pointer node) const
        -> raw_pointer {
      while (node->right != nullptr) {
        node = node->rightr();
      }
      return node;
    }

    template <std::regular NodeType> auto RbTree<NodeType>::successor(raw_pointer node) const
        -> raw_pointer {
      if (node->right != nullptr) {
        return minimum(node->rightr());
      }

      auto *temp = node->parentr();  // return raw pointer

      while (temp != nullptr && temp->rightr() == node) {
        // use move
        node = temp;
        temp = temp->parentr();
      }

      return temp;
    }

    template <std::regular NodeType> auto RbTree<NodeType>::predecessor(raw_pointer node) const
        -> raw_pointer {
      if (node->left != nullptr) {
        return maximum(node->leftr());
      }

      auto *temp = node->parentr();

      while (temp != nullptr && temp->leftr() == node) {
        node = temp;
        temp = temp->parentr();
      }

      return temp;
    }

    //    template <std::regular NodeType> void RbTree<NodeType>::left_rotate(reference_pointer
    //    node) {
    //      reference_pointer y = node->right;  // cannot be nil
    //      assert(y != nullptr);
    //      node->set_right(y->left());
    //      if (y->left() != nullptr) {
    //        y->left()->set_parent(node);
    //      }
    //      y->set_parent(node->parent());
    //      if (node->parent() == nullptr) {
    //        root = y;
    //      } else if (node == node->parent()->left()) {
    //        node->parent()->set_left(y);
    //      } else {
    //        node->parent()->set_right(y);
    //      }
    //      y->set_left(node);
    //      node->set_parent(y);
    //    }

    template <std::regular NodeType> void RbTree<NodeType>::left_rotate(reference_pointer node) {
      raw_pointer y = node->right.release();
      raw_pointer x = node.release();
      assert(y != nullptr);
      x->right.reset(y->left.release());
      if (x->right != nullptr) {
        x->right->parent = x;
      }
      node.reset(y);  // do not need to set parent again
      node->left.reset(x);
    }

    template <std::regular NodeType> void RbTree<NodeType>::right_rotate(reference_pointer node) {
      raw_pointer y = node->left.release();
      raw_pointer x = node.release();
      assert(y != nullptr);
      x->left.reset(y->right.release());
      if (x->left != nullptr) {
        x->left->parent = x;
      }
      node.reset(y);
      node->right.reset(x);
    }

    template <std::regular NodeType> void RbTree<NodeType>::fix_insert(reference_pointer node) {
      while (node->parent->is_red()) {
        if (node->parentr() == node->parentr()->parentr()->leftr()) {
          raw_pointer y = node->parentr()->parentr()->rightr();
          if (y->is_red()) {
            // case 1
            node->parentr()->set_color(Color::Black);
            y->set_color(Color::Black);
            node->parentr()->parentr()->set_color(Color::Red);
            node = node->parentr()->parentr();
          } else {
            if (node == node->parentr()->rightr()) {
              // case 2
              node = node->parentr();
              left_rotate(node);
            }
            // case 3
            node->parent()->set_color(Color::Black);
            node->parent()->parent()->set_color(Color::Red);
            right_rotate(node->parent()->parent());
          }
        } else {
          pointer y = node->parent()->parent()->left();
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
      root->set_color(Color::Black);
    }

    template <std::regular NodeType> void RbTree<NodeType>::insert_node_impl(pointer node) {
      pointer x = root;
      pointer y = nullptr;
      while (x != nullptr) {
        y = x;
        if (node->key() < x->key()) {
          x = x->left();
        } else {
          x = x->right();
        }
      }
      node->set_parent(y);
      if (y == nullptr) {
        root = node;
      } else if (node->key() < y->key()) {
        y->set_left(node);
      } else {
        y->set_right(node);
      }
      node->set_left(nullptr);
      node->set_right(nullptr);
      node->set_color(Color::Red);
      fix_insert(node);
    }

    template <std::regular NodeType> void RbTree<NodeType>::insert_node(pointer node) {
      insert_node_impl(std::move(node));
    }

    template <std::regular NodeType> void RbTree<NodeType>::inorder_walk(raw_pointer node) const {
      if (node != nullptr) {
        inorder_walk(node->left());
        std::cout << node->key() << ' ' << node->is_black() << '\n';
        inorder_walk(node->right());
      }
    }
    template <std::regular NodeType> auto RbTree<NodeType>::size() const -> size_t {
      return size(root);
    }

  }  // namespace v2

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

}  // namespace binary::algorithm::tree

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
