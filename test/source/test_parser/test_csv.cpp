//
// Created by li002252 on 8/22/22.
//
#include <spdlog/spdlog.h>

#include "binary/parser/csv.hpp"
#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <string>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

void read_tsv(std::string_view file_path) {
  io::CSVReader<3, io::trim_chars<' '>, io::no_quote_escape<'\t'>> in(
      std::string(file_path).data());
  in.read_header(io::ignore_extra_column, "chr", "start", "end");
  std::string chr;
  int start;
  int end;
  while (in.read_row(chr, start, end)) {
    spdlog::debug("chr {} start {} ", chr, start);
  }
}

TEST_CASE("smoke test for tsv") {
  constexpr const char* tsv_path = "../../test/data/rck.scnt.tsv";
  CHECK_NOTHROW(read_tsv(std::string(tsv_path)));
}