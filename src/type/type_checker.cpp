#include "type/type_checker.hpp"
#include "codegen/location.hh"
#include "exception.hpp"
#include "type/type.hpp"

namespace func {

void expect_types(const type& expected, const type& checked, yy::location loc) {
    if(!expected.equals(checked))
        throw unexpected_type_exception{ "expected " + func::type_to_string(expected) +
                                         " but received " + func::type_to_string(checked),
                                         loc };
}

} // namespace func