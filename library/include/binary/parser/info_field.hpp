//
// Created by li002252 on 8/1/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_INFO_FIELD_H_
#define BINARY_LIBRARY_INCLUDE_BINARY_INFO_FIELD_H_
#include <binary/concepts.hpp>
#include <binary/types.hpp>
#include <string>

namespace binary {

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

  template <typename Datatype>
  requires binary::concepts::IsAnyOf<Datatype, int, float, char, types::pos_t, int64_t>
  struct InfoField {
    Datatype* data{nullptr};
    int32_t count{};
    int data_id{BINARY_BCF_HT_DEFAULT};

    constexpr InfoField() {
      if constexpr (std::same_as<Datatype, types::pos_t>) {
        data_id = BINARY_BCF_HT_INT;
      } else if constexpr (std::same_as<Datatype, float>) {
        data_id = BINARY_BCF_HT_REAL;
      } else if constexpr (std::same_as<Datatype, char>) {
        data_id = BINARY_BCF_HT_STR;
      } else if constexpr (std::same_as<Datatype, int64_t>) {
        data_id = BINARY_BCF_HT_LONG;
      } else {
        data_id = BINARY_BCF_HT_DEFAULT;
      }
    };

    InfoField(InfoField const&) = delete;
    InfoField(InfoField&&) = delete;
    InfoField& operator=(InfoField const&) = delete;
    InfoField& operator=(InfoField&&) = delete;

    ~InfoField() { free(data); }

    auto result() {
      if constexpr (std::same_as<Datatype, char>) {
        return std::string(data, count - 1);
      } else {
        return *data;
      }
    }
  };

}  // namespace binary
#endif  // BINARY_LIBRARY_INCLUDE_BINARY_INFO_FIELD_H_
