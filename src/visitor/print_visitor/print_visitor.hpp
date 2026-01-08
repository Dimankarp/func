#pragma once

#include "node/expression.hpp"
#include "node/program.hpp"
#include "node/statement.hpp"
#include "type/type.hpp"
#include "visitor/visitor.hpp"
#include <ostream>
#include <string>
namespace func {

class print_visitor : public visitor<void> {
    private:
    std::ostream& out;
    int offset = 0;

    void print_type(func::type&);

    public:
    print_visitor(std::ostream& ostream) : out{ ostream } {}

    std::string tabs() const { return std::string(offset * 3, ' '); };

    std::ostream& get_out() { return out; };

    void visit(const binop_expression&) override;
    void visit(const unarop_expression&) override;
    void visit(const literal_expression&) override;
    void visit(const identifier_expression&) override;
    void visit(const function_call&) override;
    void visit(const subscript_expression&) override;

    void visit(const subscript_assign_statement&) override;
    void visit(const program&) override;
    void visit(const block_statement&) override;
    void visit(const return_statement&) override;
    void visit(const assign_statement&) override;
    void visit(const if_statement&) override;
    void visit(const while_statement&) override;
    void visit(const function&) override;
};

} // namespace func
