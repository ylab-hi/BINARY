#include <argparse/argparse.hpp>
#include <iostream>
#include <string>

#include "csv.h"
#include "spdlog/spdlog.h"
#include "vcf.h"

using namespace std::string_literals;

void read_tsv(std::string_view file_path) {
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

void read_vcf(std::string const& file_path) {
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

argparse::ArgumentParser parse_args(int argc, char* argv[]) {
  argparse::ArgumentParser program("sv2nl");
  program.add_description("Map structural Variation to Non-Linear Transcription");

  program.add_argument("segment").help("The file path of segment information from rck");
  program.add_argument("adjacent").help("The file path of adjacent information from rck");
  program.add_argument("non-linear").help("The file path of non-linear information from scannls");

  program.add_epilog("Yangyang Li 2022 Northwestern University");
  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
  return program;
}

int main(int argc, char* argv[]) {
  spdlog::set_level(spdlog::level::debug);
  auto program = parse_args(argc, argv);
  auto segment_path = program.get<std::string>("segment");
  auto adjacent_path = program.get<std::string>("adjacent");
  auto nonlinear_path = program.get<std::string>("non-linear");
  spdlog::info("segment path: {}", segment_path);
  spdlog::info("adjacent path: {}", adjacent_path);
  spdlog::info("non-linear path: {}", nonlinear_path);
  read_tsv(segment_path);
  read_vcf(nonlinear_path);
  return 0;
}
