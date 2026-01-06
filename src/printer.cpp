#include "printer.hpp"

namespace cmplr {

stream_proxy::stream_proxy(std::ostream& os, bool& enabled)
    : output_stream(os), enabled_ref(enabled) {}

stream_proxy& stream_proxy::operator<<(std::ostream& (*manip)(std::ostream&)) {
    if (enabled_ref) {
        output_stream << manip;
    }
    return *this;
}

printer::printer(std::ostream& os)
    : output_stream(os),
      print_code(true),
      print_debug(true),
      print_alloc(true),
      code(os, print_code),
      debug(os, print_debug),
      alloc(os, print_alloc) 
      {}

} // namespace cmplr  