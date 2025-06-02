#include "program.hpp"
#include "function/function.hpp"
#include "node/statement.hpp"
#include "visitor/visitor.hpp"
namespace intrp {

program::program(std::vector<unique_ptr<intrp::function>> &&funcs,
                 yy::location loc)
    : functions{std::move(funcs)}, statement{loc} {}
void program::accept(statement_visitor &visitor) {
  visitor.visit_program(*this);
}
} // namespace intrp