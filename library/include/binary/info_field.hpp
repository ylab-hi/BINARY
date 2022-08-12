//
// Created by li002252 on 8/1/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_INFO_FIELD_H_
#define BINARY_LIBRARY_INCLUDE_BINARY_INFO_FIELD_H_
#include <binary/types.hpp>
#include <map>
#include <string>

// TODO: Overload get_info_field to support many types

namespace binary {
  using namespace types;

  /**
#define BCF_HT_FLAG 0  header type
#define BCF_HT_INT  1
#define BCF_HT_REAL 2
#define BCF_HT_STR  3
#define BCF_HT_LONG (BCF_HT_INT | 0x100)  BCF_HT_INT, but for int64_t values; VCF only!
**/

  constexpr int BINARY_BCF_HT_DEFAULT = -1;
  constexpr int BINARY_BCF_HT_FLAG = 0;
  constexpr int BINARY_BCF_HT_INT = 1;
  constexpr int BINARY_BCF_HT_REAL = 2;
  constexpr int BINARY_BCF_HT_STR = 3;
  constexpr int BINARY_BCF_HT_LONG = (BINARY_BCF_HT_INT | 0x100);

  template <typename Datatype> struct InfoField {
    using result_type = Datatype;
    const int data_id = BINARY_BCF_HT_DEFAULT;
    auto get_result(Datatype const* data, int32_t /**unused**/) const -> result_type {
      return *data;
    }
  };

  template <> struct InfoField<pos_t> {
    using result_type = pos_t;
    const int data_id = BINARY_BCF_HT_INT;
    auto get_result(pos_t const* data, int32_t /**unused**/) const -> result_type { return *data; }
  };

  template <> struct InfoField<int64_t> {
    using result_type = int64_t;
    const int data_id = BINARY_BCF_HT_LONG;
    auto get_result(int64_t const* data, int32_t /**unused**/) -> result_type { return *data; }
  };

  template <> struct InfoField<float> {
    using result_type = float;
    const int data_id = BINARY_BCF_HT_REAL;
    auto get_result(float const* data, int32_t /**unused**/) const -> result_type { return *data; }
  };

  template <> struct InfoField<char> {
    using result_type = std::string;
    const int data_id = BINARY_BCF_HT_STR;
    auto get_result(char const* data, int32_t count) const -> result_type {
      return {data, data + count - 1};  // -1 to exclude the null terminator
    }
  };

}  // namespace binary
#endif  // BINARY_LIBRARY_INCLUDE_BINARY_INFO_FIELD_H_
