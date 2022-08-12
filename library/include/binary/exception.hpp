//
// Created by li002252 on 6/11/22.
//

#ifndef BINARY_LIBRARY_INCLUDE_BINARY_EXCEPTION_H_
#define BINARY_LIBRARY_INCLUDE_BINARY_EXCEPTION_H_
#include <stdexcept>
#include <string>
#include <utility>

namespace binary {

  class VcfReaderError : public std::exception {
  private:
    std::string msg;

  public:
    explicit VcfReaderError(std::string msg) : msg(std::move(msg)) {}
    [[nodiscard]] auto what() const noexcept -> const char* override { return msg.c_str(); }
  };

}  // namespace binary

#endif  // BINARY_LIBRARY_INCLUDE_BINARY_EXCEPTION_H_
