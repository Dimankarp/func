#include "visitor/type_checker.hpp"
#include "exception.hpp"
#include "type/type.hpp"

namespace cmplr {

void expect_types(const type &expected, const type &checked, yy::location loc) {
  if (!expected.equals(checked))
    throw unexpected_type_exception{
        "expected " + cmplr::type_to_string(expected) + " but received " +
            cmplr::type_to_string(checked),
        loc};
}

} // namespace cmplr