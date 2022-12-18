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

#include <binary/version.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <binary/utils.hpp>
#include <cxxopts.hpp>
#include <iostream>
#include <string>

#include "mapper.hpp"
#include "util.hpp"

constexpr int NUM_THREADS = 4;

inline std::vector<std::string> creat_files_name(std::string_view output) {
  return {std::string(output) + ".dup", std::string(output) + ".inv", std::string(output) + ".tra"};
}

void submit_task(sv2nl::DupMapper const& dup, sv2nl::InvMapper const& inv,
                 sv2nl::TraMapper const& tra, int num_threads) {
  auto thread_pool = dp::thread_pool(num_threads);

  spdlog::debug("dup use strand {}", dup.use_strand());
  spdlog::debug("inv use strand {}", inv.use_strand());
  spdlog::debug("tra use strand {}", tra.use_strand());
  dup.map(thread_pool);
  inv.map(thread_pool);
  tra.map(thread_pool);
}

void run(std::string_view nl_, std::string_view sv_, std::string_view output_, uint32_t diff_,
         int num_threads, bool use_strand) {
  auto files = creat_files_name(output_);

  auto nl_vcf = sv2nl::Sv2nlVcfRanges{std::string(nl_), "nls"};

  auto dup_mapper = sv2nl::DupMapper(sv2nl::mapper_options()
                                         .nl_file(nl_)
                                         .sv_file(sv_)
                                         .output_file(*files.begin())
                                         .nl_type("TDUP")
                                         .sv_type("DUP")
                                         .diff(diff_));

  auto inv_mapper = sv2nl::InvMapper(sv2nl::mapper_options()
                                         .nl_file(nl_)
                                         .sv_file(sv_)
                                         .output_file(*(files.begin() + 1))
                                         .nl_type("INV")
                                         .sv_type("INV")
                                         .diff(diff_)
                                         .use_strand(use_strand));

  auto tra_mapper = sv2nl::TraMapper(sv2nl::mapper_options()
                                         .nl_file(nl_)
                                         .sv_file(sv_)
                                         .output_file(*(files.begin() + 2))
                                         .nl_type("TRA")
                                         .sv_type("BND")
                                         .diff(diff_));

  submit_task(dup_mapper, inv_mapper, tra_mapper, num_threads);
  tra_mapper.close_writer();
  inv_mapper.close_writer();
  dup_mapper.close_writer();
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("sv2nl", "Map structural Variation to Non-Linear Transcription");
  options.show_positional_help();
  options.set_width(120).set_tab_expansion();
  // clang-format off
  options.add_options()
  ("sv", "The file path of segment information from delly", cxxopts::value<std::string>())
  ("non-linear", "The file path of non-linear information from scannls", cxxopts::value<std::string>())
  ("dis", "The distance threshold for trans mapper", cxxopts::value<uint32_t>()->default_value("1000000"))
  ("o,output", "The file path of output", cxxopts::value<std::string>()->default_value("output.tsv"))
  ("t,thread", "The number of thread program use", cxxopts::value<int32_t>()->default_value(std::to_string(NUM_THREADS)))
  ("s,short", "If running in short read and do not use strand", cxxopts::value<bool>()->default_value("false"))
  ("m,merge", "If provided only merge outputs into one file", cxxopts::value<bool>()->default_value("false"))
  ("d,debug", "Print debug info", cxxopts::value<bool>()->default_value("false"))
  ("h,help", "Print help")
  ("v,version", "Print the current version number");
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

  if (result["version"].as<bool>()) {
    spdlog::info("version {}", BINARY_VERSION);
    return 0;
  }

  try {
    auto segment_path = result["sv"].as<std::string>();
    auto nonlinear_path = result["non-linear"].as<std::string>();
    auto diff = result["dis"].as<uint32_t>();
    auto output_path = result["output"].as<std::string>();
    auto is_merged = result["merge"].as<bool>();
    auto num_threads = result["thread"].as<int32_t>();
    auto use_strand = !result["short"].as<bool>();

    if (!binary::utils::check_file_path({segment_path, nonlinear_path})) {
      std::exit(1);
    }

    if (num_threads < 0
        || num_threads > static_cast<int32_t>(std::thread::hardware_concurrency())) {
      spdlog::warn("The number of threads {} is invalid, default value {} will be used",
                   num_threads, NUM_THREADS);
      num_threads = NUM_THREADS;
    }

    spdlog::info("non-linear file path: {}", nonlinear_path);
    spdlog::info("struct variation file path: {}", segment_path);
    spdlog::info("distance threshold: {} bp", diff);
    spdlog::info("the number of threads: {}", num_threads);
    spdlog::info("use strand: {} ", use_strand);

    Timer timer{};
    run(nonlinear_path, segment_path, output_path, diff, num_threads, use_strand);

    if (is_merged) {
      binary::utils::merge_files(creat_files_name(output_path), output_path, sv2nl::HEADER);
    }

    spdlog::info("elapsed time: {:.2f}s", timer.elapsed());
    if (is_merged)
      spdlog::info("result file path: {}", output_path);
    else
      spdlog::info("result file path: {}[.dup|.inv|.tra]", output_path);

  } catch (const cxxopts::option_has_no_value_exception& err) {
    spdlog::error("error parsing options: {} ", err.what());
    std::cout << options.help() << "\n";
    std::exit(1);
  }

  return 0;
}
