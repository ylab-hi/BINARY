//
// Created by li002252 on 5/31/22.
//
#include <sv2nl/test.h>

[[maybe_unused]] void test() { spdlog::info("This is a test"); }

[[maybe_unused]] void read_tsv(std::string_view file_path) {
  io::CSVReader<3, io::trim_chars<' '>, io::no_quote_escape<'\t'>> in(
      std::string(file_path).data());
  in.read_header(io::ignore_extra_column, "chr", "start", "end");
  std::string chr;
  int start;
  int end;
  while (in.read_row(chr, start, end)) {
    spdlog::info("chr {} start {} ", chr, start);
  }
}
[[maybe_unused]] void read_vcf(const std::string& file_path) {
  htsFile* fp = hts_open(file_path.c_str(), "r");
  if (!fp) {
    spdlog::error("Failed to open {}", file_path);
    std::exit(1);
  }
  bcf_hdr_t* hdr = bcf_hdr_read(fp);
  if (!hdr) {
    spdlog::error("Failed to init bcf_hdr_read");
    std::exit(1);
  }
  bcf1_t* line = bcf_init();
  if (!line) {
    spdlog::error("Failed to init bcf_init");
    std::exit(1);
  }
  char const* sr = nullptr;
  int32_t count = 0;
  while (bcf_read(fp, hdr, line) >= 0) {
    int ret = bcf_get_info_string(hdr, line, "SVTYPE", &sr, &count);
    spdlog::info("result code: {} count: {}", ret, count);
    if (ret >= 0) {
      spdlog::info("SVTYPE: {}", sr);
    }
  }

  bcf_hdr_destroy(hdr);
  hts_close(fp);
  bcf_destroy(line);
}
