//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

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

  // TODO: Implement Interval Tree Query with Concurrency

  /** Interval Node and Interval Tree */

  template <typename Interval>
  concept IntervalConcept = requires(Interval const &interval) {
    requires std::semiregular<Interval>;
    requires std::same_as<decltype(interval.low), decltype(interval.high)>;
    interval.low <= interval.high;
    typename Interval::key_type;
  };

  template <typename Node>
  concept IntervalNodeConcept = requires(Node &node) {
    requires NodeConcept<Node>;
    node.max;
    node.interval;
  };

  /** Interval Tree node
   *
   */
  template <IntervalConcept Interval> class IntervalNode {
  public:
    using key_type = typename Interval::key_type;
    using pointer = std::unique_ptr<IntervalNode>;
    using reference_pointer = pointer &;
    using raw_pointer = IntervalNode *;

    constexpr IntervalNode() = default;

    constexpr IntervalNode(IntervalNode const &other) = delete;
    constexpr IntervalNode &operator=(IntervalNode const &other) = delete;
    constexpr IntervalNode(IntervalNode &&other) noexcept = default;
    constexpr IntervalNode &operator=(IntervalNode &&other) noexcept = default;

    template <typename... Arg>
    requires std::constructible_from<Interval, Arg...>
    explicit constexpr IntervalNode(Arg &&...args)
        : interval{std::forward<Arg>(args)...}, max{interval.high}, key{interval.low} {}

    explicit constexpr IntervalNode(Interval const &interval_)
        : interval{interval_}, max{interval_.high}, key{interval.low} {}

    explicit constexpr IntervalNode(Interval &&interval_)
        : interval{std::move(interval_)}, max{interval.high}, key{interval.low} {}

    void copy_key(const raw_pointer other) noexcept {
      key = other->key;
      max = other->max;
      interval = other->interval;
    }

    friend std::ostream &operator<<(std::ostream &os, IntervalNode const &node) {
      os << "IntervalNode: " << node.interval << " max: " << node.max << " key: " << node.key;
      return os;
    }

    void set_color(Color color) { color_ = color; }
    [[nodiscard]] auto is_black() const -> bool { return color_ == Color::Black; }
    [[nodiscard]] auto is_red() const -> bool { return color_ == Color::Red; }

    // All return raw pointers in order to make api consistent
    [[nodiscard]] auto leftr() const -> raw_pointer { return left.get(); }
    [[nodiscard]] auto rightr() const -> raw_pointer { return right.get(); }

    Interval interval{};
    key_type max{};
    key_type key{};
    Color color_{Color::Black};
    pointer left{nullptr};
    pointer right{nullptr};
    raw_pointer parent{nullptr};
  };

  template <KeyConcept KeyType = std::uint32_t> class BaseInterval {
  public:
    using key_type = std::remove_cv_t<KeyType>;

    constexpr BaseInterval() = default;
    constexpr BaseInterval(BaseInterval const &other) = default;
    constexpr BaseInterval &operator=(BaseInterval const &other) = default;
    constexpr BaseInterval(BaseInterval &&other) noexcept = default;

    constexpr BaseInterval &operator=(BaseInterval &&other) noexcept = default;
    constexpr BaseInterval(key_type low_, key_type high_) : low{low_}, high{high_} {
      assert(low <= high);
    }

    friend std::ostream &operator<<(std::ostream &os, BaseInterval const &base_interval) {
      os << "BaseInterval: " << base_interval.low << "-" << base_interval.high;
      return os;
    }

    key_type low{};
    key_type high{};
  };

  // set template alias for interval
  using IntInterval = BaseInterval<std::int32_t>;
  using IntIntervalNode = IntervalNode<IntInterval>;
  using UIntInterval = BaseInterval<std::uint32_t>;
  using UIntIntervalNode = IntervalNode<UIntInterval>;

  class VcfInterval : public UIntInterval {
  public:
    constexpr VcfInterval() = default;

  private:
    std::string chrom{};
    std::string svtype{};
  };

  using VcfIntervalNode = IntervalNode<VcfInterval>;

  template <IntervalNodeConcept NodeType> class IntervalTree : public RbTree<NodeType> {
  public:
    using key_type = typename NodeType::key_type;
    using pointer = typename NodeType::pointer;
    using raw_pointer = typename NodeType::raw_pointer;
    using reference_pointer = typename NodeType::reference_pointer;

    void delete_node(raw_pointer node);

    void inorder_walk(raw_pointer node, int indent = 0) const override;

  private:
    void left_rotate(raw_pointer node) override;
    void right_rotate(raw_pointer node) override;
    void insert_node_impl(raw_pointer node) override;

    auto transplant(raw_pointer target, raw_pointer source) -> raw_pointer;

    auto get_max(raw_pointer node) const;
    void update_max(raw_pointer node, key_type key) const;
    void update_max(raw_pointer node) const;

    using RbTree<NodeType>::root_;
    using RbTree<NodeType>::nil_;
  };

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::left_rotate(raw_pointer node) {
    spdlog::debug("left_rotate {}", *node);
    spdlog::debug("left_rotate right child {}", *(node->right));
    RbTree<NodeType>::left_rotate(node);
    // update argument info
    update_max(node->parent, node->max);  // update node->parent->max
    update_max(node);                     // update node->max
    spdlog::debug("after left_rotate {}", *node);
    spdlog::debug("after left_rotate {}", *(node->parent));
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::right_rotate(raw_pointer node) {
    spdlog::debug("right_rotate {}", *node);
    spdlog::debug("right_rotate left child {}", *(node->left));
    RbTree<NodeType>::right_rotate(node);
    // update argument info
    update_max(node->parent, node->max);  // update node->parent->max
    update_max(node);                     // update node->max
    spdlog::debug("after right_rotate {}", *node);
    spdlog::debug("after right_rotate {}", *(node->parent));
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::insert_node_impl(raw_pointer node) {
    spdlog::debug("insert_node {}", *node);
    raw_pointer x = root_.get();
    raw_pointer y = nullptr;

    while (x != nullptr) {
      y = x;

      // update argument info
      update_max(x, node->max);

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
    RbTree<NodeType>::fix_insert(node);
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
    node->max = std::max({node->interval.high, get_max(node->leftr()), get_max(node->rightr())});
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::inorder_walk(raw_pointer node, int indent) const {
    if (node != nullptr) {
      inorder_walk(node->leftr(), indent);
      fmt::print("{:{}}-{} is_black:{} max:{} \n", node->key, indent, node->interval.high,
                 node->is_black(), node->max);
      inorder_walk(node->rightr(), indent);
    }
  }

}  // namespace binary::algorithm::tree

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
