//
// Created by li002252 on 8/20/22.
//

#ifndef BUILDALL_LIBRARY_INCLUDE_BINARY_ALGORITHM_RB_TREE_HPP_
#define BUILDALL_LIBRARY_INCLUDE_BINARY_ALGORITHM_RB_TREE_HPP_
#include <spdlog/fmt/bundled/core.h>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>

namespace binary::algorithm::tree {

  // define key constraints
  template <typename T>
  concept KeyConcept = std::totally_ordered<T> && std::default_initializable<T>;

  template <typename Node>
  concept NodeConcept = requires(Node &node) {
    requires std::movable<Node> && std::default_initializable<Node>;
    typename Node::key_type;
    typename Node::pointer;
    typename Node::reference_pointer;
    typename Node::raw_pointer;
    node.key;
    node.left;
    node.right;
    node.parent;
    node.color_;
    node.is_black();
    node.is_red();
  };

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

    virtual void copy_key(const raw_pointer other) noexcept { key = other->key; }

    Key key{};
    Color color_{Color::Black};
    pointer left{nullptr};
    pointer right{nullptr};
    raw_pointer parent{nullptr};
  };

  class IntNode : public BaseNode<int> {
  public:
    using BaseNode::key_type;
    using BaseNode::pointer;
    using BaseNode::raw_pointer;
    using BaseNode::reference_pointer;

    constexpr IntNode() = default;
    using BaseNode::BaseNode;

    IntNode(IntNode &&other) noexcept = default;
    IntNode &operator=(IntNode &&other) noexcept = default;

    ~IntNode() override = default;

    auto operator<=>(IntNode const &other) const { return key <=> other.key; };
    // to meet equality_comparable
    friend bool operator==(IntNode const &lhs, IntNode const &rhs) { return lhs.key == rhs.key; }
  };

  // NodeType must can default initialize and  movable
  template <NodeConcept NodeType> class RbTree {
  public:
    using pointer = typename NodeType::pointer;
    using reference_pointer = typename NodeType::reference_pointer;
    using raw_pointer = typename NodeType::raw_pointer;

    constexpr RbTree() = default;
    constexpr RbTree(RbTree &&other) noexcept = default;
    auto operator=(RbTree &&other) noexcept -> RbTree & = default;
    constexpr RbTree(const RbTree &other) = delete;
    auto operator=(const RbTree &other) -> RbTree & = delete;

    virtual ~RbTree() = default;

    template <std::ranges::input_range R>
    requires std::constructible_from<NodeType, std::ranges::range_value_t<R>>
    void insert_node(R &&range) {
      for (auto &&item : range) {
        insert_node(std::forward<decltype(item)>(item));
      }
    }

    [[maybe_unused]] auto search(const typename NodeType::key_type &key) const -> raw_pointer;
    [[maybe_unused]] auto search(const typename NodeType::key_type &key) -> raw_pointer;

    /**
     * @brief return if the tree is empty
     * @return
     */
    [[nodiscard]] auto empty() const -> bool;
    [[nodiscard]] auto size(raw_pointer node) const -> size_t;
    [[nodiscard]] auto size() const -> size_t;
    [[nodiscard]] auto root() const -> raw_pointer;

    // WARNING: Return raw pointer do not delete that!
    [[nodiscard]] auto minimum(raw_pointer node) const -> raw_pointer;
    [[nodiscard]] auto maximum(raw_pointer node) const -> raw_pointer;

    [[maybe_unused]] [[nodiscard]] auto successor(raw_pointer node) const -> raw_pointer;
    [[maybe_unused]] [[nodiscard]] auto predecessor(raw_pointer node) const -> raw_pointer;

    virtual void inorder_walk(raw_pointer node, int indent) const;

    void to_dot(std::string_view filename) const;

    // Ownership transfer
    [[maybe_unused]] void insert_node(pointer node);

    template <typename... Args>
    requires std::constructible_from<NodeType, Args...>
    void insert_node(Args &&...args) {
      insert_node(std::make_unique<NodeType>(std::forward<Args>(args)...));
    }

    void delete_node(reference_pointer node);
    void delete_node(raw_pointer node);

  protected:
    virtual void to_dot_impl(std::ofstream &output, raw_pointer node) const;
    virtual void left_rotate(raw_pointer node);
    virtual void right_rotate(raw_pointer node);
    virtual void insert_node_impl(raw_pointer node);
    void fix_insert(raw_pointer node);
    auto transplant(raw_pointer target, raw_pointer source) -> raw_pointer;
    void fix_delete(raw_pointer node, bool is_left_child);
    auto check_is_left_child_when_delete(raw_pointer node, bool is_left_child) const -> bool;

    // Use reference pointer as smart pointer will be reset
    [[nodiscard]] auto check_is_red(raw_pointer node) const -> bool;
    [[nodiscard]] auto check_is_black(raw_pointer node) const -> bool;
    void release_reset(reference_pointer target, raw_pointer source = nullptr) const;

    pointer root_{nullptr};
    pointer nil_{std::make_unique<NodeType>()};  // only for delete and tree is always owner
  };

  template <NodeConcept NodeType> auto RbTree<NodeType>::size() const -> size_t {
    return size(root_.get());
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::size(raw_pointer node) const -> size_t {
    if (node == nullptr) return 0;
    return 1 + size(node->leftr()) + size(node->rightr());
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::empty() const -> bool {
    return size() == 0;
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::root() const -> raw_pointer {
    return root_.get();
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::check_is_red(raw_pointer node) const
      -> bool {
    return node != nullptr && node->is_red();
  }

  template <NodeConcept NodeType> auto RbTree<NodeType>::check_is_black(raw_pointer node) const
      -> bool {
    return !check_is_red(node);
  }

  template <NodeConcept NodeType>
  void RbTree<NodeType>::release_reset(reference_pointer target, raw_pointer source) const {
    target.release();
    target.reset(source);
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

  template <NodeConcept NodeType>
  [[maybe_unused]] auto RbTree<NodeType>::successor(raw_pointer node) const -> raw_pointer {
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

  template <NodeConcept NodeType>
  [[maybe_unused]] auto RbTree<NodeType>::predecessor(raw_pointer node) const -> raw_pointer {
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

  template <NodeConcept NodeType>
  [[maybe_unused]] void RbTree<NodeType>::insert_node(pointer node) {
    insert_node_impl(node.release());
  }

  template <NodeConcept NodeType>
  void RbTree<NodeType>::inorder_walk(raw_pointer node, int indent) const {
    if (node != nullptr) {
      inorder_walk(node->leftr(), indent);
      fmt::print("{:{}} is_black {} \n", node->key, indent + 4, node->is_black());
      inorder_walk(node->rightr(), indent);
    }
  }

  template <NodeConcept NodeType>
  void RbTree<NodeType>::to_dot_impl(std::ofstream &output, raw_pointer node) const {
    if (node != nullptr) {
      output << fmt::format("{} [label=\"{}\", color={}, style=bold];\n", node->key, node->key,
                            node->is_black() ? "black" : "red");
      to_dot_impl(output, node->leftr());
      to_dot_impl(output, node->rightr());
      if (node->left != nullptr) {
        output << fmt::format("{} -- {} [style=bold];\n", node->key, node->leftr()->key);
      }
      if (node->right != nullptr) {
        output << fmt::format("{} -- {} [style=bold];\n", node->key, node->rightr()->key);
      }
    }
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::to_dot(std::string_view filename) const {
    std::ofstream output(filename.data());
    output << "graph G {\n";
    output << "fontname=\"Helvetica,Arial,sans-serif\"\n"
           << "node [fontname=\"Helvetica,Arial,sans-serif\"]\n"
           << "edge [fontname=\"Helvetica,Arial,sans-serif\"]\n";
    to_dot_impl(output, root_.get());
    output << "}\n";
    output.close();
  }

  template <NodeConcept NodeType>
  auto RbTree<NodeType>::transplant(raw_pointer target, raw_pointer source) -> raw_pointer {
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

  template <NodeConcept NodeType>
  void RbTree<NodeType>::fix_delete(raw_pointer node, bool is_left_child) {
    while (node != root() && check_is_black(node)) {
      if (check_is_left_child_when_delete(node, is_left_child)) {
        raw_pointer w = node->parent->rightr();
        if (w->is_red()) {
          // case 1
          w->set_color(Color::Black);
          node->parent->set_color(Color::Red);
          left_rotate(node->parent);
          w = node->parent->rightr();
        }
        if (check_is_black(w->leftr()) && check_is_black(w->rightr())) {
          // case 2
          w->set_color(Color::Red);
          node = node->parent;
        } else {
          if (check_is_black(w->rightr())) {
            // case 3
            assert(w->leftr() != nullptr);
            w->left->set_color(Color::Black);  // w->left may be nullptr
            w->set_color(Color::Red);
            right_rotate(w);
            w = node->parent->rightr();
          }
          // case 4
          w->set_color(node->parent->color_);
          node->parent->set_color(Color::Black);
          w->right->set_color(Color::Black);
          left_rotate(node->parent);
          node = root_.get();
        }
      } else {
        raw_pointer w = node->parent->leftr();
        if (w->is_red()) {
          // case 1
          w->set_color(Color::Black);
          node->parent->set_color(Color::Red);
          right_rotate(node->parent);
          w = node->parent->leftr();
        }
        if (check_is_black(w->leftr()) && check_is_black(w->rightr())) {
          // case 2
          w->set_color(Color::Red);
          node = node->parent;
        } else {
          if (check_is_black(w->leftr())) {
            // case 3
            assert(w->rightr() != nullptr);
            w->right->set_color(Color::Black);
            w->set_color(Color::Red);
            left_rotate(w);
            w = node->parent->leftr();
          }
          // case 4
          w->set_color(node->parent->color_);
          node->parent->set_color(Color::Black);
          w->left->set_color(Color::Black);
          right_rotate(node->parent);
          node = root_.get();
        }
      }
    }

    node->set_color(Color::Black);
  }

  template <NodeConcept NodeType>
  auto RbTree<NodeType>::check_is_left_child_when_delete(raw_pointer node, bool is_left_child) const
      -> bool {
    if (node == nil_.get()) {
      return is_left_child;
    }
    return node == node->parent->leftr();
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::delete_node(raw_pointer node) {
    if (node == nullptr) return;

    raw_pointer y = node;
    raw_pointer x{nullptr};
    raw_pointer x_parent{nullptr};
    bool x_is_left = false;

    auto y_origin_is_black = y->is_black();
    // node has one child
    if (node->left == nullptr) {
      x = node->right.release();
      x_parent = transplant(y, x);  // y and node is not valid after this
    } else if (node->right == nullptr) {
      x = node->left.release();
      x_is_left = true;
      x_parent = transplant(y, x);  // y and node is not valid after this
    } else {
      // node has two children
      y = minimum(node->rightr());
      y_origin_is_black = y->is_black();
      x = y->right.release();

      if (y != node->rightr()) {
        node->copy_key(y);
        x_parent = transplant(y, x);  // y is not valid after this
        x_is_left = true;
      } else {
        raw_pointer node_left = node->left.release();
        node->right.release();  // release y
        y->set_color(node->color_);
        transplant(node, y);  //  release node raw pointer
        y->left.reset(node_left);
        node_left->parent = y;  // add parent to node_left
        y->right.reset(x);
        x_parent = y;
      }
    }

    //  node may be nullptr
    if (y_origin_is_black && x_parent != nullptr) {
      if (x == nullptr) {
        x = nil_.get();
        x->parent = x_parent;
      }
      fix_delete(x, x_is_left);
    }
  }

  template <NodeConcept NodeType> void RbTree<NodeType>::delete_node(reference_pointer node) {
    delete_node(node.get());
  }

  template <NodeConcept NodeType>
  [[maybe_unused]] auto RbTree<NodeType>::search(const typename NodeType::key_type &key) const
      -> raw_pointer {
    raw_pointer node = root();
    while (node != nullptr) {
      if (key == node->key) {
        return node;
      } else if (key < node->key) {
        node = node->leftr();
      } else {
        node = node->rightr();
      }
    }
    return nullptr;
  }

  template <NodeConcept NodeType>
  [[maybe_unused]] auto RbTree<NodeType>::search(const typename NodeType::key_type &key)
      -> raw_pointer {
    raw_pointer node = root();
    while (node != nullptr) {
      if (key == node->key) {
        return node;
      } else if (key < node->key) {
        node = node->leftr();
      } else {
        node = node->rightr();
      }
    }
    return nullptr;
  }

}  // namespace binary::algorithm::tree

#endif  // BUILDALL_LIBRARY_INCLUDE_BINARY_ALGORITHM_RB_TREE_HPP_
