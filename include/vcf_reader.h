#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_

#include <string>
#include <utility>

#include "vcf.h"
#include "spdlog/spdlog.h"

namespace sv2nl {

  class VcfReader {
  public:
    [[maybe_unused]] explicit VcfReader(const std::string&  file_path) : file_path_{file_path} { init(); }

    VcfReader(const VcfReader& vcfreader) = default;
    VcfReader& operator=(const VcfReader& vcfreader) = default;
    VcfReader(VcfReader&& vcfreader) = default;
    VcfReader& operator=(VcfReader&& vcfreader) = default;

    ~VcfReader() {
      bcf_destroy(line);
      if (fp) {
        close();
      }
    }

    void init();
    [[maybe_unused]] void open(std::string file_path);
    void close();

  private:
    std::string file_path_{};
    htsFile* fp{nullptr};
    bcf_hdr_t* hdr{nullptr};
    bcf1_t* line{nullptr};
  };

}  // namespace sv2nl

#endif
