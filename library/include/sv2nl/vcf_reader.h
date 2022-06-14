#ifndef SV2NL_SRC_VCF_READER_H_
#define SV2NL_SRC_VCF_READER_H_

#include <memory>
#include <string>

namespace sv2nl {

  class VcfReader {
  private:
    struct impl;
    std::unique_ptr<impl> pimpl;

    [[maybe_unused]] void open(const std::string& file_path);

  public:
    VcfReader() = delete;
    explicit VcfReader(std::string const& file_path);
    ~VcfReader() noexcept;

    VcfReader(VcfReader&&) noexcept;
    VcfReader& operator=(VcfReader&&) noexcept;

    [[nodiscard]] const std::string& get_file_path() const;

    [[nodiscard]] bool is_open() const;
    [[nodiscard]] bool is_closed() const;
    [[nodiscard]] bool has_index() const;
    void check_record() const;

    [[nodiscard]] std::string get_chrom() const;
    [[nodiscard]] int64_t get_pos() const;
    [[nodiscard]] int64_t get_rlen() const;
    [[nodiscard]] int32_t get_info_int(const std::string& key) const;
    [[nodiscard]] std::string get_info_string(const std::string& key) const;

    int next_record();
    void print_record() const;

    void query(const std::string& chrom, int64_t start, int64_t end);
  };

}  // namespace sv2nl

#endif
