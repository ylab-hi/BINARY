//
// Created by li002252 on 8/1/22.
//

#ifndef SV2NL_LIBRARY_INCLUDE_SV2NL_INFO_FIELD_H_
#define SV2NL_LIBRARY_INCLUDE_SV2NL_INFO_FIELD_H_
#include <map>
#include <string>

namespace sv2nl {

  /**
#define BCF_HT_FLAG 0  header type
#define BCF_HT_INT  1
#define BCF_HT_REAL 2
#define BCF_HT_STR  3
#define BCF_HT_LONG (BCF_HT_INT | 0x100)  BCF_HT_INT, but for int64_t values; VCF only!
**/

  constexpr int SV2NL_BCF_HT_DEFAULT = -1;
  constexpr int SV2NL_BCF_HT_FLAG = 0;
  constexpr int SV2NL_BCF_HT_INT = 1;
  constexpr int SV2NL_BCF_HT_REAL = 2;
  constexpr int SV2NL_BCF_HT_STR = 3;
  constexpr int SV2NL_BCF_HT_LONG = (SV2NL_BCF_HT_INT | 0x100);

  template <typename Datatype> struct InfoField {
    using result_type = Datatype;
    const int data_id = SV2NL_BCF_HT_DEFAULT;
    auto get_result(Datatype const* data, int32_t /**unused**/) const -> result_type {
      return *data;
    }
  };

  template <> struct InfoField<int32_t> {
    using result_type = int32_t;
    const int data_id = SV2NL_BCF_HT_INT;
    auto get_result(int32_t const* data, int32_t /**unused**/) const -> result_type {
      return *data;
    }
  };

  template <> struct InfoField<int64_t> {
    using result_type = int64_t;
    const int data_id = SV2NL_BCF_HT_LONG;
    auto get_result(int64_t const* data, int32_t /**unused**/) -> result_type { return *data; }
  };

  template <> struct InfoField<float> {
    using result_type = float;
    const int data_id = SV2NL_BCF_HT_REAL;
    auto get_result(float const* data, int32_t /**unused**/) const -> result_type { return *data; }
  };

  template <> struct InfoField<char> {
    using result_type = std::string;
    const int data_id = SV2NL_BCF_HT_STR;
    auto get_result(char const* data, int32_t count) const -> result_type {
      return {data, data + count};
    }
  };

}  // namespace sv2nl
#endif  // SV2NL_LIBRARY_INCLUDE_SV2NL_INFO_FIELD_H_
