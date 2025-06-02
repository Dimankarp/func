#include "type.hpp"

namespace intrp {
types int_type::get_type() { return types::INT; }

types string_type::get_type() { return types::STRING; }

types bool_type::get_type() { return types::BOOL; }

types void_type::get_type() { return types::VOID; }


types function_type::get_type() { return types::FUNCTION; }

function_type::function_type(std::vector<std::unique_ptr<type>> &&sign)
    : signature{std::move(sign)} {}

} // namespace intrp
