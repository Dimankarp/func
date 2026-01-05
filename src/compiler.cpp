#include "driver.hpp"
#include "exception.hpp"
#include "visitor/code_visitor.hpp"
#include "visitor/print_visitor.hpp"
#include "visitor/visitor.hpp"

#include <fstream>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
  int res = 0;
  driver drv;
  for (int i = 1; i < argc; ++i)
    if (argv[i] == std::string("-p"))
      drv.trace_parsing = true;
    else if (argv[i] == std::string("-s"))
      drv.trace_scanning = true;
    else if (argv[i] == std::string("-o")) {
      if (i + 1 == argc) {
        std::cout << "Expected filename after '-f' - exiting.\n";
        exit(1);
      }
      drv.output_file = argv[i + 1];
      i++;
    } else {
      if (drv.parse(argv[i]) != 0) {
        std::cout << "Failed to parse - exiting.\n";
        exit(1);
      };

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
        std::cout << "Syntax error: unexpected type " << e << "\n";
        exit(1);
      } catch (cmplr::symbol_not_found_exception &e) {
        std::cout << "Syntax error: symbol not found " << e << "\n";
        exit(1);
      } catch (cmplr::syntax_exception &e) {
        std::cout << "Syntax error: " << e << "\n";
        exit(1);
      } catch (cmplr::global_syntax_exception &e) {
        std::cout << "Syntax error: " << e << "\n";
        exit(1);
      }
    }
}
