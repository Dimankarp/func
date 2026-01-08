#pragma once

#include "location.hh"
#include "node/ast.hpp"
#include <memory>
#include <string>
#include <utility>
#include <variant>
namespace func {

using std::unique_ptr;
using lit_val = std::variant<std::string, int, bool>;

enum class binop : char {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LESS,
    GRTR,
    EQ,
    NEQ,
    OR,
    AND
};

class binop_expression : public ast_node_impl<binop_expression> {
    using Base = ast_node_impl<binop_expression>;

    private:
    binop op;
    unique_ptr<ast_node> left;
    unique_ptr<ast_node> right;

    public:
    binop_expression(binop op, unique_ptr<ast_node> left, unique_ptr<ast_node> right, yy::location loc)
    : op(op), left(std::move(left)), right(std::move(right)), Base(loc) {}
    binop get_op() const { return op; }
    const unique_ptr<ast_node>& get_left() const { return left; }
    const unique_ptr<ast_node>& get_right() const { return right; }
};

enum class unarop : char { MINUS, NOT };

class unarop_expression : public ast_node_impl<unarop_expression> {
    using Base = ast_node_impl<unarop_expression>;

    private:
    unarop op;
    unique_ptr<ast_node> exp;

    public:
    unarop_expression(unarop op, unique_ptr<ast_node> exp, yy::location loc)
    : op(op), exp(std::move(exp)), Base(loc) {};
    unarop get_op() const { return op; }
    const unique_ptr<ast_node>& get_exp() const { return exp; }
};

class literal_expression : public ast_node_impl<literal_expression> {
    using Base = ast_node_impl<literal_expression>;

    private:
    lit_val val;

    public:
    literal_expression(lit_val val, yy::location loc)
    : val(std::move(val)), Base(loc) {};
    lit_val get_val() const { return val; }
};

class identifier_expression : public ast_node_impl<identifier_expression> {

    using Base = ast_node_impl<identifier_expression>;

    private:
    std::string identificator;

    public:
    identifier_expression(std::string identificator, yy::location loc)
    : identificator(std::move(identificator)), Base(loc) {}
    const std::string& get_identificator() const { return identificator; }
};

class subscript_expression : public ast_node_impl<subscript_expression> {
    using Base = ast_node_impl<subscript_expression>;

    private:
    unique_ptr<ast_node> pointer;
    unique_ptr<ast_node> index;

    public:
    subscript_expression(unique_ptr<ast_node> pointer, unique_ptr<ast_node> index, yy::location loc)
    : pointer{ std::move(pointer) }, index{ std::move(index) }, Base{ loc } {}

    const unique_ptr<ast_node>& get_pointer() const { return pointer; }
    const unique_ptr<ast_node>& get_index() const { return index; }
};

} // namespace func