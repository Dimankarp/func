#include "driver.hpp"
#include "parser.tab.hpp"
#include <string>

driver::driver() {}

int driver::parse(const std::string &f) {
  file = f;
  location.initialize(&file);
  scan_begin();
  yy::parser parser(*this);
  parser.set_debug_level(
      static_cast<yy::parser::debug_level_type>(trace_parsing));
  int const result = parser.parse();
  scan_end();
  return result;
}
