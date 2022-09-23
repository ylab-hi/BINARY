//
// Created by li002252 on 8/13/22.
//

#ifndef BUILDALL_LIBRARY_INCLUDE_BINARY_CONCEPTS_HPP_
#define BUILDALL_LIBRARY_INCLUDE_BINARY_CONCEPTS_HPP_
#include <concepts>

namespace binary::concepts {

  template <typename T, typename... U>
  concept IsAnyOf = (std::same_as<T, U> || ...);

  template <typename T, typename... U>
  concept IsAllOf = (std::same_as<T, U> && ...);

  template <typename T, typename... Args>
  concept ArgsConstructible
      = std::constructible_from<T, Args...> && (!IsAnyOf<T, std::remove_cvref_t<Args>...>);

}  // namespace binary::concepts

#endif  // BUILDALL_LIBRARY_INCLUDE_BINARY_CONCEPTS_HPP_
