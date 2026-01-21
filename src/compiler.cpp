#include "cxxopts.hpp"
#include "driver.hpp"
#include "exception.hpp"
#include "printer.hpp"
#include "visitor/code_visitor/code_visitor.hpp"
#include "visitor/llvm_visitor/llvm_visitor.hpp"
#include "visitor/print_visitor/print_visitor.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>

enum class Arch{
    SIM,
    X64
};

int main(int argc, char* argv[]) {
    driver drv;
    bool print_ast_flag;
    bool debug_mode_flag;
    bool alloc_trace_flag;
    std::string output_file;
    Arch target_arch;
    std::optional<std::reference_wrapper<std::ostream>> output_stream;

    cxxopts::Options options("compiler", "A compiler for a simple C-like language called FunC.");

    // clang-format off
    options.add_options()
    ("h,help", "Print help page")
    ("p,print-ast", "Print AST to stdout")
    ("trace-parsing", "Trace parsing")
    ("trace-scanning","Trace scanning")
    ("d,debug", "Include debug output")
    ("a,alloc", "Include alloc traces")
    ("o,output", "Output file",cxxopts::value<std::string>())
    ("arch", "Specify target architecture: sim | x64",
         cxxopts::value<std::string>()->default_value("x64"));
    // clang-format on
    options.add_options()("source", "The .fc files to proccess",
                          cxxopts::value<std::vector<std::string>>());
    options.parse_positional({ "source" });

    options.positional_help("SOURCE_FILES");

    try {
        auto result{ options.parse(argc, argv) };

        print_ast_flag = result["print-ast"].as<bool>();
        drv.trace_parsing = result["trace-parsing"].as<bool>();
        drv.trace_scanning = result["trace-scanning"].as<bool>();
        debug_mode_flag = result["debug"].as<bool>();
        alloc_trace_flag = result["alloc"].as<bool>();


        if(result.count("help")) {
            std::cout << options.help() << '\n';
            exit(0);
        }

        if(result.count("output")) {
            output_file = result["output"].as<std::string>();
        }


        if(result.count("source")) {
            auto srcs = result["source"].as<std::vector<std::string>>();
            if (srcs.size() > 1) {
                std::cerr << "Module can be compiled from only one source file.\n";
                exit(func::error_codes::E_PARAMS);
            }
            if(drv.parse(srcs[0]) != 0) {
                std::cerr << "Failed to parse - exiting.\n";
                exit(func::error_codes::E_SYTNTAX);
            }
        } else {
            exit(0); // No input files were given.
        }

        std::string arch_str = result["arch"].as<std::string>();
        if (arch_str == "sim") {
            target_arch = Arch::SIM;
        } else if (arch_str == "x64") {
            target_arch = Arch::X64;
        } else {
            std::cerr << "Unknown architecture: " << arch_str << "\n";
            exit(func::error_codes::E_PARAMS);
        }

    } catch(const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << '\n';
        exit(func::error_codes::E_PARAMS);
    }

    func::print_visitor print_visitor{ std::cout };

    auto tree = std::move(drv.result);
    try {
        if(print_ast_flag) {
            std::cout << " ### Print visitor output:\n";
            tree->accept(print_visitor);
            std::cout << "\n";
        }

        std::ofstream out;
        if(output_file.empty()) {
            // std::cout << " ### Code visitor output:\n";
            output_stream = std::cout;
        } else {
            out = std::ofstream(output_file);
            output_stream = out;
        }

        func::printer printer{ output_stream.value().get() };
        printer.print_code = true;
        printer.print_debug = debug_mode_flag;
        printer.print_alloc = alloc_trace_flag;

        switch (target_arch) {
            case Arch::SIM: {
                func::code_visitor code_visitor{ printer };
                tree->accept(code_visitor);
                break;
            }
            case Arch::X64: {
                func::llvm_visitor llvm_visitor{ printer };
                tree->accept(llvm_visitor);
                break;
            }
            default: {
                std::cerr << "Unsupported architecture\n";
                exit(func::error_codes::E_PARAMS);
            }
        }

    } catch(func::unexpected_type_exception& e) {
        std::cerr << "Syntax error: unexpected type " << e << "\n";
        exit(func::error_codes::E_TYPE);
    } catch(func::symbol_not_found_exception& e) {
        std::cerr << "Syntax error: symbol not found " << e << "\n";
        exit(func::error_codes::E_SYMBOL);
    } catch(func::symbol_redeclaration_exception& e) {
        std::cerr << "Syntax error: symbol redeclaration " << e << "\n";
        exit(func::error_codes::E_SYMBOL);
    } catch(func::syntax_exception& e) {
        std::cerr << "Syntax error: " << e << "\n";
        exit(func::error_codes::E_SYTNTAX);
    } catch(func::global_syntax_exception& e) {
        std::cerr << "Syntax error: " << e << "\n";
        exit(func::error_codes::E_SYTNTAX);
    }
}
