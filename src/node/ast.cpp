#include "ast.hpp"
#include "codegen/location.hh"
namespace func {

yy::location ast_node::get_loc() const {
    return loc;
}
ast_node::ast_node(yy::location loc) : loc(loc) {};

} // namespace func