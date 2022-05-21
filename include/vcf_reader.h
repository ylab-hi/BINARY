#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_

#include <string>
#include <utility>

#include "vcf.h"
#include "spdlog/spdlog.h"

namespace sv2nl {

  class [[maybe_unused]] VcfReader {
  public:
    [[maybe_unused]] explicit VcfReader(std::string const&   file_path) : file_path_{file_path} { init(); }

    VcfReader(VcfReader const&) = delete;
    VcfReader(VcfReader&&)      = delete;
    VcfReader& operator=(VcfReader const&) = delete;
    VcfReader& operator=(VcfReader&&)      = delete;

    ~VcfReader() {
      bcf_destroy(line);
      if (fp) {
        close();
      }
    }

    [[maybe_unused]] void open(std::string file_path);
    void close();

  private:
    void init();
    std::string file_path_{};
    htsFile* fp{nullptr};
    bcf_hdr_t* hdr{nullptr};
    bcf1_t* line{nullptr};
  };

}  // namespace sv2nl

#endif
