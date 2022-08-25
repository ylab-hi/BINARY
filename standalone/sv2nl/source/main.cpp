/**
 * Mapping structure variations to non-linear splicing
 * author: li002252
 * date:   8/2/22
 * license: MIT
 *
 * algorithm:
 *
 * 1. detect duplications
 * 2. detect inversions
 * 3. detect translocations
 *
 */

#include <spdlog/spdlog.h>

#include <binary/utils.hpp>
#include <cxxopts.hpp>
#include <iostream>
#include <string>

#include "util.hpp"

auto main(int argc, char* argv[]) -> int {
  cxxopts::Options options("sv2nl", "Map structural Variation to Non-Linear Transcription");
  options.show_positional_help();
  options.set_width(120);
  // clang-format off
  options.add_options()
  ("segment", "The file path of segment information from rck", cxxopts::value<std::string>())
  ("non-linear", "The file path of non-linear information from scannls", cxxopts::value<std::string>())
  ("d,debug", "Print debug info", cxxopts::value<bool>()->default_value("false"))
  ("h,help", "Print help");
  // clang-format o

  options.positional_help("[segment non-linear]");
  options.parse_positional({"segment", "non-linear"});
  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << "\n";
    std::exit(0);
  }

  if (result.count("debug")) {
    spdlog::set_level(spdlog::level::debug);
  }

  try {
    auto segment_path = result["segment"].as<std::string>();
    auto nonlinear_path = result["non-linear"].as<std::string>();
    if (!binary::utils::check_file_path({segment_path, nonlinear_path})) {
      std::exit(1);
    }
    spdlog::debug("segment file path: {}", segment_path);
    spdlog::debug("non-linear file path: {}", nonlinear_path);

  } catch (const cxxopts::option_has_no_value_exception& err) {
    spdlog::error("error parsing options: {} ", err.what());
    std::cout << options.help() << "\n";
    std::exit(1);
  }

  return 0;
}
