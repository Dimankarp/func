#include "visitor/llvm_visitor/llvm_visitor.hpp"
#include "node/program.hpp"
#include "type/type.hpp"
#include "type/type_checker.hpp"
#include "llvm/IR/Verifier.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <variant>

namespace func {

void llvm_visitor::visit(const program& node) {
    for(const auto& func : node.get_funcs()) {
        func->accept(*this);
    }
    module.print(outs(), nullptr);
}


Type* llvm_visitor::llvm_get_type(types t) {
    switch(t) {
    case types::INT: return Type::getInt32Ty(ctx);
    case types::STRING: return PointerType::get(ctx, 0);
    case types::BOOL: return Type::getIntNTy(ctx, 1);
    case types::VOID: return Type::getVoidTy(ctx);
    case types::FUNCTION: exit(10); // TODO
    default: exit(10);
    }
}


void llvm_visitor::visit(const function& node) {
    Type* result_type = llvm_get_type(node.get_result_type()->get_type());
    std::vector<Type*> params_type;
    for(const auto& p : node.get_params()) {
        params_type.push_back(llvm_get_type(p.get_type()->get_type()));
    }
    FunctionType* ft = FunctionType::get(result_type, params_type, false);
    Function* f = Function::Create(ft, Function::LinkageTypes::InternalLinkage,
                                   node.get_identifier(), module);
    int i = 0;
    const auto& params = node.get_params();
    for(auto& arg : f->args())
        arg.setName(params[i++].get_identifier());


    auto signature = std::vector<unique_ptr<type>>();
    if(node.get_params().empty())
        signature.push_back(std::make_unique<void_type>());
    else
        for(const auto& p : node.get_params()) {
            signature.push_back(p.get_type()->clone());
        }
    signature.push_back(node.get_result_type()->clone());

    auto info =
    llvm_sym_info{ node.get_identifier(),
                   std::make_unique<func::function_type>(std::move(signature)) };
    table.add(std::move(info));


    table.start_block(); // start
    for(int i = 0; i < node.get_params().size(); i++) {
        const auto& param = node.get_params()[i];
        // Registering parameters
        table.add(llvm_sym_info{
        param.get_identifier(),
        param.get_type()->clone(),
        });
    }

    BasicBlock* bb = BasicBlock::Create(ctx, "entry", f);
    builder.SetInsertPoint(bb);

    node.get_block()->accept(*this);

    table.end_block(); // end

    verifyFunction(*f);

    result = f;
}

void llvm_visitor::visit(const block_statement& node) {
    table.start_block(); // start

    for(const auto& st : node.get_statements()) {
        st->accept(*this);
    }

    table.end_block(); // end
};


FunctionType* llvm_visitor::llvm_get_function_type(const function_type& func_type) {
    Type* result_type = llvm_get_type(func_type.get_return_type()->get_type());
    std::vector<Type*> params_type;
    const auto& signature = func_type.get_signature();
    for(int i = 0; i < signature.size() - 1; i++) {
        params_type.push_back(llvm_get_type(signature[i]->get_type()));
    }

    return FunctionType::get(result_type, params_type, false);
}


void llvm_visitor::visit(const function_call& node) {
    auto result = node.get_func()->accept_with_result(*this);

    const function_type& func_type = std::holds_alternative<TypedFunctionPtr>(result) ?
    dynamic_cast<const function_type&>(*std::get<TypedFunctionPtr>(result).type_obj) :
    dynamic_cast<const function_type&>(*std::get<TypedValuePtr>(result).type_obj);

    const auto& args = node.get_arg_list();
    auto args_sz = args.size();
    auto params_sz = func_type.get_signature().front()->get_type() == types::VOID ?
    0 :
    func_type.get_signature().size() - 1;

    if(args_sz != params_sz)
        throw syntax_exception{ "wrong number of arguments, expected: " + std::to_string(params_sz) +
                                ", but got " + std::to_string(args_sz),
                                node.get_loc() };

    std::vector<Value*> argsV;
    for(int i = 0; i < node.get_arg_list().size(); i++) {
        auto a = std::get<TypedValuePtr>(args[i]->accept_with_result(*this));

        expect_types(*func_type.get_signature()[i], *a.type_obj, args[i]->get_loc());

        argsV.push_back(a.ptr);
    }


    if(std::holds_alternative<TypedFunctionPtr>(result)) {
        Function* function = std::get<TypedFunctionPtr>(result).ptr;
        result = TypedValuePtr{ builder.CreateCall(function, argsV, "calltmp"),
                                func_type.get_return_type()->clone() };
    } else {
        Value* function = std::get<TypedValuePtr>(result).ptr;
        result = TypedValuePtr{ builder.CreateCall(llvm_get_function_type(func_type),
                                                   function, argsV, "calltmp"),
                                func_type.get_return_type()->clone() };
    }
}

void llvm_visitor::visit(const identifier_expression& node) {
    auto sym = table.find(node.get_identificator());

    if(sym.type_obj->get_type() == types::FUNCTION && !sym.value.has_value()) {
        auto* f = module.getFunction(sym.name);
        assert(f);
        result = TypedFunctionPtr{ f, std::move(sym.type_obj) };
        return;
    }

    result = TypedValuePtr{ sym.value.value(), std::move(sym.type_obj) };
};


void llvm_visitor::visit(const binop_expression&) {};
void llvm_visitor::visit(const unarop_expression&) {};
void llvm_visitor::visit(const literal_expression&) {};
void llvm_visitor::visit(const subscript_expression&) {};

void llvm_visitor::visit(const subscript_assign_statement&) {};
void llvm_visitor::visit(const return_statement&) {};
void llvm_visitor::visit(const assign_statement&) {};
void llvm_visitor::visit(const if_statement&) {};
void llvm_visitor::visit(const while_statement&) {};


} // namespace func
