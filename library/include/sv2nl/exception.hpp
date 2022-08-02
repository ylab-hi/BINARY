//
// Created by li002252 on 6/11/22.
//

#ifndef SV2NL_LIBRARY_INCLUDE_SV2NL_EXCEPTION_H_
#define SV2NL_LIBRARY_INCLUDE_SV2NL_EXCEPTION_H_
#include <stdexcept>
#include <string>
#include <utility>

namespace sv2nl {

  class VcfReaderError : public std::exception {
  private:
    std::string msg;

  public:
    explicit VcfReaderError(std::string msg) : msg(std::move(msg)) {}
    [[nodiscard]] auto what() const noexcept -> const char* override { return msg.c_str(); }
  };

}  // namespace sv2nl

#endif  // SV2NL_LIBRARY_INCLUDE_SV2NL_EXCEPTION_H_
