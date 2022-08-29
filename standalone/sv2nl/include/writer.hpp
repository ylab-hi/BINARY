//
// Created by li002252 on 8/24/22.
//

#ifndef BUILDALL_STANDALONE_SV2NL_WRITER_HPP_
#define BUILDALL_STANDALONE_SV2NL_WRITER_HPP_
#include <fstream>
#include <ranges>
#include <string>
#include <vcf_info.hpp>

namespace sv2nl {

  class Writer {
  public:
    explicit Writer(std::string_view filename) : filename_(filename), ofs_(filename_) {
      write_header();
    }

    Writer(std::string_view filename, std::string_view header)
        : filename_(filename), ofs_(filename_), header_(header) {
      write_header();
    }

    template <std::ranges::input_range Sv2nlRecordRange>
    requires std::same_as<std::ranges::range_value_t<Sv2nlRecordRange>, Sv2nlVcfRecord>
    void write(Sv2nlRecordRange&& records) {
      if (records.size() < 2) return;
      auto key_line = get_keys(records[0]);
      for (auto&& record :
           std::ranges::subrange(std::ranges::begin(records) + 1, std::ranges::end(records))) {
        ofs_ << key_line << '\t' << get_keys(record) << '\n';
      }
    }

    void write(std::string const& line);
    static std::string get_keys(Sv2nlVcfRecord const& record);

  private:
    void write_header();

    std::string filename_{};
    std::ofstream ofs_{};
    std::string header_{"chrom\tpos\tend\tsvtype\tchrom\tpos\tend\tsvtype"};
  };

}  // namespace sv2nl

#endif  // BUILDALL_STANDALONE_SV2NL_WRITER_HPP_
