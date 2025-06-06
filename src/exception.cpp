#include "exception.hpp"
#include <ostream>
namespace intrp {

std::ostream &operator<<(std::ostream &outs, const syntax_exception &e) {
  return outs << e.reason << " at " << e.loc;
}

std::ostream &operator<<(std::ostream &outs, const global_syntax_exception &e) {
  return outs << e.reason;
}
} // namespace intrp