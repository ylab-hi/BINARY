#include <sv2nl/test.h>

#include <argparse/argparse.hpp>
#include <iostream>
#include <string>

using namespace std::string_literals;

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
  auto program = parse_args(argc, argv);
  auto segment_path = program.get<std::string>("segment");
  auto adjacent_path = program.get<std::string>("adjacent");
  auto nonlinear_path = program.get<std::string>("non-linear");
  std::cout << "segment: " << segment_path << std::endl;
  std::cout << "adjacent: " << adjacent_path << std::endl;
  std::cout << "non-linear: " << nonlinear_path << std::endl;
  //  read_tsv(segment_path);
  //  read_vcf(nonlinear_path);
  sv2nl::test();
  return 0;
}
