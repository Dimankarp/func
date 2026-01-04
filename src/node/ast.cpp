#include "ast.hpp"
namespace func {

yy::location ast_node::get_loc() const { return loc; }
ast_node::ast_node(yy::location loc) : loc(loc) {};

} // namespace func