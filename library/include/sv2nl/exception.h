//
// Created by li002252 on 6/11/22.
//

#ifndef BUILDALL_LIBRARY_INCLUDE_SV2NL_EXCEPTION_H_
#define BUILDALL_LIBRARY_INCLUDE_SV2NL_EXCEPTION_H_
#include <stdexcept>
#include <string>

namespace sv2nl {

  class VcfReaderError : public std::exception {
  private:
    std::string msg;

  public:
    VcfReaderError(std::string const &msg) : msg(msg) {}
    const char *what() const noexcept override { return msg.c_str(); }
  };

}  // namespace sv2nl

#endif  // BUILDALL_LIBRARY_INCLUDE_SV2NL_EXCEPTION_H_
