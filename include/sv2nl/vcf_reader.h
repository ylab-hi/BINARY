#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_

#include <memory>
#include <string>
#include <utility>

namespace sv2nl {

  class [[maybe_unused]] VcfReader {
  private:
    struct impl;
    std::unique_ptr<impl> pimpl;

  public:
    explicit VcfReader(std::string file_path);

    VcfReader(VcfReader const&) = delete;
    VcfReader& operator=(VcfReader const&) = delete;
    VcfReader(VcfReader&&) = default;
    VcfReader& operator=(VcfReader&&) = default;

    ~VcfReader();

    [[maybe_unused]] void open(std::string file_path);
    void close();
  };

}  // namespace sv2nl

#endif
