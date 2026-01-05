#include "driver.hpp"
#include "exception.hpp"
#include "visitor/code_visitor.hpp"
#include "visitor/print_visitor.hpp"
#include "visitor/visitor.hpp"
#include "depends/cxxopts.hpp"

#include <fstream>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
  driver drv;

  cxxopts::Options options("compiler", "A compiler for a simple C-like language called FunC.");

  options.add_options()
		("h,help", "Print help page")
		("p", "Trace parsing")
		("s", "Trace scanning")
		("d,debug", "Include debug output")
		("o,output", "Output file", cxxopts::value<std::string>())
  ;

  options.add_options()
    ("source", "The .fc files to proccess", cxxopts::value<std::vector<std::string>>())
  ;
  options.parse_positional({"source"});

  options.positional_help("SOURCE FILES");

  try {
    auto result{ options.parse(argc, argv) };
    
    drv.trace_parsing = result["p"].as<bool>();
    drv.trace_scanning = result["s"].as<bool>();
    drv.debug_mode = result["d"].as<bool>();

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    if (result.count("output")) {
      drv.output_file = result["output"].as<std::string>();
    }

    if (result.count("source")) {
      auto srcs = result["source"].as<std::vector<std::string>>();
      for (const auto& src : srcs) {
        if (drv.parse(src) != 0) {
          std::cerr << "Failed to parse - exiting.\n";
          exit(1);
        }
      }
    } else {
      exit(0); // No input files given.
    }
  }
  catch (const cxxopts::exceptions::exception& e) {
    std::cerr << "Error parsing options: " << e.what() << std::endl;
    exit(1);
  }


  cmplr::print_visitor print_visitor{std::cout};

  auto tree = std::move(drv.result);
  try {
    std::cout << "Print visitor output:\n";
    tree->accept(print_visitor);
    std::cout << "Code visitor output:\n";

    if (drv.output_file.empty()) {
      cmplr::code_visitor code_visitor{std::cout};
      tree->accept(code_visitor);
    } else {
      std::ofstream out(drv.output_file);
      cmplr::code_visitor code_visitor{out};
      tree->accept(code_visitor);
    }

  } catch (cmplr::unexpected_type_exception &e) {
    std::cerr << "Syntax error: unexpected type " << e << "\n";
    exit(1);
  } catch (cmplr::symbol_not_found_exception &e) {
    std::cerr << "Syntax error: symbol not found " << e << "\n";
    exit(1);
  } catch (cmplr::syntax_exception &e) {
    std::cerr << "Syntax error: " << e << "\n";
    exit(1);
  } catch (cmplr::global_syntax_exception &e) {
    std::cerr << "Syntax error: " << e << "\n";
    exit(1);
  }
}

