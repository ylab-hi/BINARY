//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <binary/algorithm/rb_tree.hpp>
#include <binary/concepts.hpp>
#include <optional>
#include <ranges>
namespace binary::algorithm::tree {

  /** Interval Tree Based on Red Black Tree

  1. Every node is either red or black
  2. The root is black
  3. Every leaf(NIL) is black
  4. If a node is red, the both its children are black
  5. For each node, all simple paths from the node to
    descendant leaves contain the same number of black nodes.
  **/

  // TODO: implement delete operation in interval tree
  /** Interval Node and Interval Tree */

  template <typename Interval>
  concept IntervalConcept = requires(Interval const &interval) {
    requires std::semiregular<Interval>;
    requires std::movable<Interval>;
    requires std::same_as<decltype(interval.low), decltype(interval.high)>;
    interval.low <= interval.high;
    typename Interval::key_type;
  };

  template <typename Node>
  concept IntervalNodeConcept = requires(Node &node) {
    requires NodeConcept<Node>;
    typename Node::interval_type;
    node.max;
    node.interval;
  };

  /** Interval Tree node
   *
   */
  template <IntervalConcept Interval> class IntervalNode {
  public:
    using interval_type = Interval;
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

    [[nodiscard]] virtual auto is_overlap(BaseInterval const &other) const -> bool {
      return low <= other.high && other.low <= high;
    }

    virtual ~BaseInterval() = default;

    friend std::ostream &operator<<(std::ostream &os, BaseInterval const &base_interval) {
      os << "BaseInterval: " << base_interval.low << "-" << base_interval.high;
      return os;
    }

    key_type low{};
    key_type high{};
  };

  // set template alias for interval
  using IntInterval = BaseInterval<std::int32_t>;
  using UIntInterval = BaseInterval<std::uint32_t>;
  using IntIntervalNode = IntervalNode<IntInterval>;
  using UIntIntervalNode = IntervalNode<UIntInterval>;

  template <IntervalNodeConcept NodeType> class IntervalTree : public RbTree<NodeType> {
  public:
    using key_type = typename NodeType::key_type;
    using pointer = typename NodeType::pointer;
    using raw_pointer = typename NodeType::raw_pointer;
    using reference_pointer = typename NodeType::reference_pointer;
    using interval_type = typename NodeType::interval_type;

    void delete_node(raw_pointer node);

    void inorder_walk(raw_pointer node, int indent) const override;

    [[nodiscard]] auto find_overlap(interval_type const &interval) const
        -> std::optional<interval_type>;

    template <typename... Args>
    requires binary::concepts::ArgsConstructible<interval_type, Args...>
    [[nodiscard]] auto find_overlap(Args &&...args) const -> std::optional<interval_type> {
      return find_overlap(interval_type{std::forward<Args>(args)...});
    }

    [[nodiscard]] auto find_overlaps(std::same_as<interval_type> auto &&interval) const
        -> std::vector<interval_type>;

    template <typename... Args>
    requires binary::concepts::ArgsConstructible<interval_type, Args...>
    [[nodiscard]] auto find_overlaps(Args &&...args) const -> std::vector<interval_type> {
      return find_overlaps(interval_type{std::forward<Args>(args)...});
    }

  private:
    auto find_overlaps_impl(interval_type const &interval, raw_pointer node) const
        -> std::vector<interval_type>;

    void to_dot_impl(std::ofstream &output, raw_pointer node) const override;
    void left_rotate(raw_pointer node) override;
    void right_rotate(raw_pointer node) override;
    void insert_node_impl(raw_pointer node) override;

    auto transplant(raw_pointer target, raw_pointer source) -> raw_pointer;

    auto get_max(raw_pointer node) const -> key_type;
    void update_max(raw_pointer node, key_type key) const;
    void update_max(raw_pointer node) const;

    using RbTree<NodeType>::root_;
    using RbTree<NodeType>::nil_;
  };

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::to_dot_impl(std::ofstream &output, raw_pointer node) const {
    if (node != nullptr) {
      output << fmt::format("{} [label=\"{}-{}-{}\", color={}, style=bold];\n", node->key,
                            node->key, node->interval.high, node->max,
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

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::left_rotate(raw_pointer node) {
    spdlog::trace("left_rotate {}", *node);
    spdlog::trace("left_rotate right child {}", *(node->right));
    RbTree<NodeType>::left_rotate(node);
    // update argument info
    update_max(node->parent, node->max);  // update node->parent->max
    update_max(node);                     // update node->max
    spdlog::trace("after left_rotate {}", *node);
    spdlog::trace("after left_rotate {}", *(node->parent));
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::right_rotate(raw_pointer node) {
    spdlog::trace("right_rotate {}", *node);
    spdlog::trace("right_rotate left child {}", *(node->left));
    RbTree<NodeType>::right_rotate(node);
    // update argument info
    update_max(node->parent, node->max);  // update node->parent->max
    update_max(node);                     // update node->max
    spdlog::trace("after right_rotate {}", *node);
    spdlog::trace("after right_rotate {}", *(node->parent));
  }

  template <IntervalNodeConcept NodeType>
  void IntervalTree<NodeType>::insert_node_impl(raw_pointer node) {
    spdlog::trace("insert_node {}", *node);
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
  auto IntervalTree<NodeType>::get_max(raw_pointer node) const -> key_type {
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

  template <IntervalNodeConcept NodeType>
  auto IntervalTree<NodeType>::find_overlap(interval_type const &interval) const
      -> std::optional<interval_type> {
    raw_pointer x = root_.get();
    while (x != nullptr) {
      if (interval.is_overlap(x->interval)) {
        return x->interval;
      } else if (interval.low <= get_max(x->leftr())) {
        x = x->leftr();
      } else {
        x = x->rightr();
      }
    }
    return {};
  }

  template <IntervalNodeConcept NodeType>
  auto IntervalTree<NodeType>::find_overlaps_impl(interval_type const &interval,
                                                  raw_pointer node) const
      -> std::vector<interval_type> {
    std::vector<interval_type> ret{};
    if (node == nullptr) {
      return ret;
    }

    spdlog::debug("find_overlaps_impl {} {}", get_max(node), interval);

    if (interval.is_overlap(node->interval)) {
      spdlog::debug("push {}", node->interval);
      ret.push_back(node->interval);
    }

    if (interval.low <= get_max(node->leftr())) {
      spdlog::debug("find_overlaps_impl left child {} {}", get_max(node->leftr()), interval.low);

      std::ranges::move(find_overlaps_impl(interval, node->leftr()), std::back_inserter(ret));
    } else {
      spdlog::debug("find_overlaps_impl right child {} {}", get_max(node->rightr()), interval.low);
      std::ranges::move(find_overlaps_impl(interval, node->rightr()), std::back_inserter(ret));
    }

    return ret;
  }

  template <IntervalNodeConcept NodeType>
  auto IntervalTree<NodeType>::find_overlaps(std::same_as<interval_type> auto &&interval) const
      -> std::vector<interval_type> {
    return find_overlaps_impl(std::forward<interval_type>(interval), root_.get());
  }

}  // namespace binary::algorithm::tree

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_INTERVAL_TREE_HPP_
