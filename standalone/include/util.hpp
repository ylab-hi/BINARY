//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_STANDALONE_UTILS_HPP_
#define BINARY_STANDALONE_UTILS_HPP_

#include <chrono>
#include <string>

auto add_chr(std::string const& chrom) -> std::string;

class Timer {
public:
  using clock_ = std::chrono::high_resolution_clock;
  using second_ = std::chrono::duration<double>;

  Timer() : beg_(clock_::now()) {}

  void reset() { beg_ = clock_::now(); }

  [[nodiscard]] auto elapsed() const -> double;

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> beg_{};
};

#endif  // BINARY_STANDALONE_UTILS_HPP_
