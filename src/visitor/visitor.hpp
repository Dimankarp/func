#pragma once

namespace func {

class binop_expression;
class unarop_expression;
class literal_expression;
class identifier_expression;
class function_call;
class subscript_expression;
class function;

class program;
class block_statement;
class return_statement;
class assign_statement;
class if_statement;
class while_statement;
class subscript_assign_statement;

class visitor_base {
    public:
    virtual void visit(const binop_expression&) = 0;
    virtual void visit(const unarop_expression&) = 0;
    virtual void visit(const literal_expression&) = 0;
    virtual void visit(const identifier_expression&) = 0;
    virtual void visit(const function_call&) = 0;
    virtual void visit(const subscript_expression&) = 0;
    virtual void visit(const program&) = 0;
    virtual void visit(const block_statement&) = 0;
    virtual void visit(const return_statement&) = 0;
    virtual void visit(const assign_statement&) = 0;
    virtual void visit(const if_statement&) = 0;
    virtual void visit(const while_statement&) = 0;
    virtual void visit(const function&) = 0;
    virtual void visit(const subscript_assign_statement&) = 0;
    virtual ~visitor_base() = default;
};

template <typename ReturnType> class visitor : public visitor_base {
    public:
    virtual ReturnType&& extract_result() = 0;
};
template <> class visitor<void> : public visitor_base {
    // empty
};

} // namespace func