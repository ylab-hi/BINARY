//
// Created by li002252 on 8/3/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_TYPES_HPP_
#define BINARY_LIBRARY_INCLUDE_BINARY_TYPES_HPP_

#include <cstdint>
#include <string>

namespace binary::types {

  // VcfRecord types
  using pos_t = std::uint32_t;
  using chrom_t = std::string;

}  // namespace binary::types

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_TYPES_HPP_
