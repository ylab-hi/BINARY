#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_

#include <experimental/propagate_const>
#include <memory>
#include <string>
#include <utility>

namespace sv2nl {

  class [[maybe_unused]] VcfReader {
  private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;

  public:
    VcfReader();
    explicit VcfReader(std::string file_path);

    VcfReader(VcfReader const&) = delete;
    VcfReader& operator=(VcfReader const&) = delete;
    VcfReader(VcfReader&&) noexcept;
    VcfReader& operator=(VcfReader&&) noexcept;

    ~VcfReader();

    [[maybe_unused]] void open(std::string file_path);
    void close();
    [[nodiscard]] const std::string& get_file_path() const;

    [[nodiscard]] bool is_open() const;
    [[nodiscard]] bool is_closed() const;
  };

}  // namespace sv2nl

#endif