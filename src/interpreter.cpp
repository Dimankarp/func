#include "driver.hpp"
#include "exception.hpp"
#include "visitor/code_visitor.hpp"
#include "visitor/print_visitor.hpp"
#include "visitor/visitor.hpp"

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
    else {
      if (drv.parse(argv[i]) != 0) {
        std::cout << "Failed to parse - exiting.\n";
        exit(1);
      };
      auto variables = std::make_shared<var_table>();
      intrp::print_visitor print_visitor{std::cout};
      intrp::code_visitor code_visitor{std::cout};

      auto tree = std::move(drv.result);
      try {
        std::cout << "Print visitor output:\n";
        tree->accept(print_visitor);
        std::cout << "Code visitor output:\n";
        tree->accept(code_visitor);
      } catch (intrp::unexpected_type_exception &e) {
        std::cout << "Syntax error: unexpected type " << e << "\n";
        exit(1);
      } catch (intrp::syntax_exception &e) {
        std::cout << "Syntax error: " << e << "\n";
        exit(1);
      }
    }
}
