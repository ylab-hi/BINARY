#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_

#include <experimental/propagate_const>
#include <memory>
#include <string>

namespace sv2nl {

  class [[maybe_unused]] VcfReader {
  private:
    class impl;
    std::experimental::propagate_const<std::unique_ptr<impl>> pimpl;

  public:
    explicit VcfReader(std::string const& file_path);

    VcfReader(VcfReader&&) noexcept;
    VcfReader& operator=(VcfReader&&) noexcept;

    [[maybe_unused]] void open(const std::string& file_path) const;
    void close() const;
    [[nodiscard]] const std::string& get_file_path() const;

    [[nodiscard]] bool is_open() const;
    [[nodiscard]] bool is_closed() const;
  };

}  // namespace sv2nl

#endif
