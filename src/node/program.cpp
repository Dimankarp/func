#include "program.hpp"
#include "function/function.hpp"
#include "node/statement.hpp"
#include "visitor/visitor.hpp"
namespace cmplr {

program::program(std::vector<unique_ptr<cmplr::function>> &&funcs,
                 yy::location loc)
    : functions{std::move(funcs)}, statement{loc} {}
void program::accept(statement_visitor &visitor) {
  visitor.visit_program(*this);
}
} // namespace cmplr