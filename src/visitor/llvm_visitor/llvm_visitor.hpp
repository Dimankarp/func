#pragma once

#include "codegen/location.hh"
#include "printer.hpp"
#include "type/type.hpp"
#include "visitor/sym_table.hpp"
#include "visitor/visitor.hpp"
#include "llvm/IR/Value.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <optional>
#include <utility>
#include <variant>

namespace func {

using namespace llvm;

struct llvm_sym_info {
    std::string name;
    std::unique_ptr<func::type> type_obj;
    std::optional<Value*> value;
    yy::location declare_loc;
    bool is_delimeter = false;

    public:
    llvm_sym_info(std::string name,
                  const std::unique_ptr<func::type>& type_obj,
                  std::optional<Value*> value,
                  yy::location declare_loc = yy::location{},
                  bool is_delimeter = false)
    : name{ std::move(name) }, type_obj{ type_obj->clone() }, value{ value },
      declare_loc{ declare_loc }, is_delimeter{ is_delimeter } {}
    llvm_sym_info() = default;
    llvm_sym_info(const llvm_sym_info& sym)
    : name{ sym.name }, value{ sym.value }, declare_loc{ sym.declare_loc },
      is_delimeter{ sym.is_delimeter } {
        if(sym.type_obj != nullptr)
            type_obj = sym.type_obj->clone();
    }
};

struct TypedValuePtr {
    Value* ptr;
    std::unique_ptr<type> type_obj;
};

struct TypedFunctionPtr {
    Function* ptr;
    std::unique_ptr<type> type_obj;
};

using llvm_result = std::variant<TypedValuePtr, TypedFunctionPtr, Function*>;

class llvm_visitor : public visitor<llvm_result> {
    private:
    func::stream_proxy& debug_out;
    func::llvm_stream_proxy code_out;
    llvm_result result;
    sym_table<llvm_sym_info> table;
    LLVMContext ctx;
    Module module{ "func module", ctx };
    IRBuilder<> builder{ ctx };
    FunctionPassManager fpm;
    FunctionAnalysisManager fam;

    public:
    llvm_visitor(func::printer& printer)
    : debug_out{ printer.debug }, code_out{ llvm_stream_proxy{ printer.code } } {
        fpm.addPass(PromotePass());
        PassBuilder PB;
        PB.registerFunctionAnalyses(fam);
    }


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
    void visit(const declaration&) override;
    void visit(const function&) override;

    llvm_result&& extract_result() override { return std::move(result); }

    private:
    Type* llvm_get_type(types t);
    FunctionType* llvm_get_function_type(const function_type& func_type);
    TypedValuePtr turn_to_typed_value_ptr(llvm_result res);
};

} // namespace func
