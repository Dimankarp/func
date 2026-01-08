#pragma once

#include "printer.hpp"
#include "visitor/visitor.hpp"
#include "llvm/IR/Value.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <variant>

namespace func {

using namespace llvm;

using llvm_result = std::variant<Value*, Function*>;

class code_visitor : public visitor<llvm_result> {
    private:
    func::stream_proxy& debug_out;
    llvm_result result;
    //   sym_table<sym_info> table;
    LLVMContext ctx;
    Module module;
    IRBuilder<> builder;

    public:
    code_visitor(func::printer& printer);

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

    llvm_result&& extract_result() override { return std::move(result); }

    private:
};

} // namespace func
