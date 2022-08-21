//
// Created by li002252 on 8/13/22.
//

#ifndef BUILDALL_LIBRARY_INCLUDE_BINARY_CONCEPTS_HPP_
#define BUILDALL_LIBRARY_INCLUDE_BINARY_CONCEPTS_HPP_
#include <concepts>

namespace binary::concepts {

  template <typename T, typename... U>
  concept IsAnyOf = (std::same_as<T, U> || ...);

}  // namespace binary::concepts

#endif  // BUILDALL_LIBRARY_INCLUDE_BINARY_CONCEPTS_HPP_
