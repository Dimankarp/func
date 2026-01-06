#include "depends/cxxopts.hpp"
#include "driver.hpp"
#include "exception.hpp"
#include "printer.hpp"
#include "visitor/code_visitor.hpp"
#include "visitor/print_visitor.hpp"
#include "visitor/visitor.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>

int main(int argc, char *argv[]) {
  driver drv;
  bool print_ast_flag;
  bool debug_mode_flag;
  bool alloc_trace_flag;
  std::string output_file;
  std::optional<std::reference_wrapper<std::ostream>> output_stream;

  cxxopts::Options options(
      "compiler", "A compiler for a simple C-like language called FunC.");

  options.add_options()("h,help", "Print help page")(
      "p,print-ast", "Print AST to stdin")("trace-parsing", "Trace parsing")(
      "trace-scanning", "Trace scanning")("d,debug", "Include debug output")(
      "a,alloc", "Include alloc traces")("o,output", "Output file",
                                         cxxopts::value<std::string>());

  options.add_options()("source", "The .fc files to proccess",
                        cxxopts::value<std::vector<std::string>>());
  options.parse_positional({"source"});

  options.positional_help("SOURCE FILES");

  try {
    auto result{options.parse(argc, argv)};

    print_ast_flag = result["print-ast"].as<bool>();
    drv.trace_parsing = result["trace-parsing"].as<bool>();
    drv.trace_scanning = result["trace-scanning"].as<bool>();
    debug_mode_flag = result["debug"].as<bool>();
    alloc_trace_flag = result["alloc"].as<bool>();

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    if (result.count("output")) {
      output_file = result["output"].as<std::string>();
    }

    if (result.count("source")) {
      auto srcs = result["source"].as<std::vector<std::string>>();
      if (drv.parse(srcs[0]) != 0) { // Only process the first file for now.
        std::cerr << "Failed to parse - exiting.\n";
        exit(1);
      }
    } else {
      exit(0); // No input files were given.
    }
  } catch (const cxxopts::exceptions::exception &e) {
    std::cerr << "Error parsing options: " << e.what() << std::endl;
    exit(1);
  }

  func::print_visitor print_visitor{std::cout};

  auto tree = std::move(drv.result);
  try {
    if (print_ast_flag) {
      std::cout << " ### Print visitor output:\n";
      tree->accept(print_visitor);
      std::cout << "\n";
    }

    std::ofstream out;
    if (output_file.empty()) {
      std::cout << " ### Code visitor output:\n";
      output_stream = std::cout;
    } else {
      out = std::ofstream(output_file);
      output_stream = out;
    }

    func::printer printer{output_stream.value().get()};
    printer.print_code = true;
    printer.print_debug = debug_mode_flag;
    printer.print_alloc = alloc_trace_flag;

    func::code_visitor code_visitor{printer};
    tree->accept(code_visitor);

  } catch (func::unexpected_type_exception &e) {
    std::cerr << "Syntax error: unexpected type " << e << "\n";
    exit(1);
  } catch (func::symbol_not_found_exception &e) {
    std::cerr << "Syntax error: symbol not found " << e << "\n";
    exit(1);
  } catch (func::syntax_exception &e) {
    std::cerr << "Syntax error: " << e << "\n";
    exit(1);
  } catch (func::global_syntax_exception &e) {
    std::cerr << "Syntax error: " << e << "\n";
    exit(1);
  }
}
