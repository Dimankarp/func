#include "driver.hpp"
#include "parser.tab.hpp"

driver::driver() : trace_parsing(false), trace_scanning(false) {}

int driver::parse(const std::string &f) {
  file = f;
  location.initialize(&file);
  scan_begin();
  yy::parser parser(*this);
  parser.set_debug_level(
      static_cast<yy::parser::debug_level_type>(trace_parsing));
  int result = parser.parse();
  scan_end();
  return result;
}
