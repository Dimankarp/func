#include "program.hpp"
#include "function/function.hpp"
#include "node/statement.hpp"
#include "visitor/visitor.hpp"
namespace func {

program::program(std::vector<unique_ptr<func::function>> &&funcs,
                 yy::location loc)
    : functions{std::move(funcs)}, statement{loc} {}
void program::accept(statement_visitor &visitor) {
  visitor.visit_program(*this);
}
} // namespace func