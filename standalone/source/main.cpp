#include <spdlog/spdlog.h>
#include <sv2nl/test.h>

// #include <argparse/argparse.hpp>
#include <cxxopts.hpp>
#include <iostream>
#include <string>

auto main(int argc, char* argv[]) -> int {
  cxxopts::Options options("sv2nl", "Map structural Variation to Non-Linear Transcription");
  options.show_positional_help();
  options.set_width(120);
  // clang-format off
  options.add_options()
  ("segment", "The file path of segment information from rck", cxxopts::value<std::string>())
  ("adjacent", "The file path of adjacent information from rck", cxxopts::value<std::string>())
  ("non-linear", "The file path of non-linear information from scannls", cxxopts::value<std::string>())
  ("d,debug", "Print debug info", cxxopts::value<bool>()->default_value("false"))
  ("h,help", "Print help");
  // clang-format on

  options.positional_help("[segment adjacent non-linear]");
  options.parse_positional({"segment", "adjacent", "non-linear"});
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
    auto adjacent_path = result["adjacent"].as<std::string>();
    auto nonlinear_path = result["non-linear"].as<std::string>();
    spdlog::debug("segment file path: {}", segment_path);
    spdlog::debug("adjacent file path: {}", adjacent_path);
    spdlog::debug("non-linear file path: {}", nonlinear_path);
    sv2nl::read_tsv(segment_path);
    sv2nl::test_vcf(nonlinear_path);
    sv2nl::read_vcf(nonlinear_path);
  } catch (const cxxopts::option_has_no_value_exception& err) {
    spdlog::error("error parsing options: {} ", err.what());
    std::cout << options.help() << "\n";
    return 1;
  }

  return 0;
}
