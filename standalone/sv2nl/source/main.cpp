/**
 * @file main.cpp
 * Mapping structure variations to non-linear splicing
 * @author: li002252
 * @date:   8/2/22
 * @license: MIT
 *
 * @algorithm:
 *
 * 1. detect duplication
 * 2. detect inversions
 * 3. detect translocations
 *
 */

#include <spdlog/spdlog.h>

#include <binary/utils.hpp>
#include <cxxopts.hpp>
#include <iostream>
#include <string>

#include "mapper.hpp"
#include "util.hpp"

void run(std::string_view nl_, std::string_view sv_, std::string_view output_, uint32_t diff_) {
  auto files = {std::string(output_) + ".dup", std::string(output_) + ".inv",
                std::string(output_) + ".tra"};
  auto dup_mapper = sv2nl::DupMapper(sv2nl::mapper_options()
                                         .nl_file(nl_)
                                         .sv_file(sv_)
                                         .output_file(*files.begin())
                                         .nl_type("TDUP")
                                         .sv_type("DUP"));
  auto inv_mapper = sv2nl::InvMapper(sv2nl::mapper_options()
                                         .nl_file(nl_)
                                         .sv_file(sv_)
                                         .output_file(*(files.begin() + 1))
                                         .nl_type("INV")
                                         .sv_type("INV"));

  auto tra_mapper = sv2nl::TraMapper(sv2nl::mapper_options()
                                         .nl_file(nl_)
                                         .sv_file(sv_)
                                         .output_file(*(files.begin() + 2))
                                         .nl_type("TRA")
                                         .sv_type("BND"),
                                     diff_);

  dup_mapper.map();
  inv_mapper.map();
  tra_mapper.map();
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("sv2nl", "Map structural Variation to Non-Linear Transcription");
  options.show_positional_help();
  options.set_width(120);
  // clang-format off
  options.add_options()
  ("sv", "The file path of segment information from rck", cxxopts::value<std::string>())
  ("non-linear", "The file path of non-linear information from scannls", cxxopts::value<std::string>())
  ("o,output", "The file path of output", cxxopts::value<std::string>()->default_value("output.tsv"))
  ("t,threshold", "The distance threshold for trans mapper", cxxopts::value<uint32_t>()->default_value("1000000"))
  ("d,debug", "Print debug info", cxxopts::value<bool>()->default_value("false"))
  ("h,help", "Print help");
  // clang-format on

  options.positional_help("[sv non-linear]");
  options.parse_positional({"sv", "non-linear"});
  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << "\n";
    std::exit(0);
  }

  if (result.count("debug")) {
    spdlog::set_level(spdlog::level::debug);
  }

  try {
    auto segment_path = result["sv"].as<std::string>();
    auto nonlinear_path = result["non-linear"].as<std::string>();
    auto output_path = result["output"].as<std::string>();
    auto diff = result["threshold"].as<uint32_t>();
    if (!binary::utils::check_file_path({segment_path, nonlinear_path})) {
      std::exit(1);
    }

    spdlog::info("non-linear file path: {}", nonlinear_path);
    spdlog::info("struct variation file path: {}", segment_path);
    Timer timer{};
    run(nonlinear_path, segment_path, output_path, diff);
    spdlog::info("elapsed time: {:.2f}s", timer.elapsed());
    spdlog::info("result file path: {}[.dup|.inv|.tra]", output_path);

  } catch (const cxxopts::option_has_no_value_exception& err) {
    spdlog::error("error parsing options: {} ", err.what());
    std::cout << options.help() << "\n";
    std::exit(1);
  }

  return 0;
}
