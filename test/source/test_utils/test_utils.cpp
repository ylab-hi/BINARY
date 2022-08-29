//
// Created by li002252 on 8/29/22.
//

#include "binary/utils.hpp"
#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

void create_file(std::string_view filename, std::string_view content) {
  std::ofstream ofs(filename.data());
  ofs << content;
  ofs.close();
}

TEST_SUITE("test binary utils") {
  TEST_CASE("test merge files") {
    namespace fs = std::filesystem;
    constexpr std::array files = {"test1.txt", "test2.text"};
    constexpr const char* output_file = "test_merge.txt";

    for (auto const& file : files) {
      create_file(file, "test content");
      CHECK(fs::exists(file));
    }

    SUBCASE("test delete original files") {
      binary::utils::merge_files(files, output_file);
      CHECK(fs::exists(output_file));

      for (auto const& file : files) {
        CHECK_FALSE(fs::exists(file));
      }
    }

    SUBCASE("test not delete original files") {
      binary::utils::merge_files(files, output_file, false);
      CHECK(fs::exists(output_file));
      for (auto const& file : files) {
        CHECK(fs::exists(file));
      }

      for (auto const& file : files) {
        fs::remove(file);
      }
    }
  }

  TEST_CASE("test index ranges") {
    std::vector v1{1, 2, 3, 4};
    std::array a1{1, 2, 3, 4};
    std::vector sv1{"I", "am", "an", "test"};

    CHECK_EQ(binary::utils::index(v1, 1), 0);
    CHECK_EQ(binary::utils::index(v1, 2), 1);
    CHECK_EQ(binary::utils::index(a1, 3), 2);
    CHECK_EQ(binary::utils::index(a1, 4), 3);

    CHECK_EQ(binary::utils::index(sv1, "test"), 3);
    CHECK_EQ(binary::utils::index(sv1, "am"), 1);
  }
}