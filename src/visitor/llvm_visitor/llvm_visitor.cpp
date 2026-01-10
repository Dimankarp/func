#include "visitor/llvm_visitor/llvm_visitor.hpp"
#include "node/function.hpp"
#include "node/program.hpp"
#include "type/type.hpp"
#include "type/type_checker.hpp"
#include "visitor/sym_table.hpp"
#include "llvm/IR/Verifier.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <optional>
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


void llvm_visitor::visit(const declaration& node) {
    // construct llvm function type
    Type* result_type = llvm_get_type(node.get_result_type()->get_type());
    std::vector<Type*> params_type;
    for(const auto& p : node.get_params()) {
        params_type.push_back(llvm_get_type(p.get_type()->get_type()));
    }
    FunctionType* ft = FunctionType::get(result_type, params_type, false);

    // Check if its already exists
    if (Function* fm = module.getFunction(node.get_identifier())){
        auto sym = table.find(node.get_identifier());
        if (ft != fm->getFunctionType()){
            throw symbol_redeclaratione_exception{ sym.name, node.get_loc(), sym.declare_loc };
        }
        result = fm;
        return;
    }

    // Otherwise create new and register it 
    Function* f = Function::Create(ft, Function::LinkageTypes::ExternalLinkage, node.get_identifier(), module);
    
    // register it in sym_table
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
                   std::make_unique<func::function_type>(std::move(signature)),
                   std::nullopt };
    table.add(std::move(info));

    result = f;
}

void llvm_visitor::visit(const function& node) {
    Function* f = std::get<Function*>(node.get_declaration()->accept_with_result(*this));

    if (node.get_block() == nullptr){
        result = f;
        return;
    }

    table.start_block(); // start
    auto* f_arg_vals = f->args().begin();
    for(const auto& param : node.get_params()) {
        // Registering parameters
        table.add(llvm_sym_info{ param.get_identifier(), param.get_type()->clone(),
                                 std::make_optional(&(*f_arg_vals)) });
        f_arg_vals++;
    }

    BasicBlock* bb = BasicBlock::Create(ctx, "entry", f);
    builder.SetInsertPoint(bb);

    node.get_block()->accept(*this);

    table.end_block(); // end
    
    verifyFunction(*f);

    // TODO: why don't we set insert block back?

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
        result = TypedValuePtr{ builder.CreateCall(function, argsV),
                                func_type.get_return_type()->clone() };
    } else {
        Value* function = std::get<TypedValuePtr>(result).ptr;
        result =
        TypedValuePtr{ builder.CreateCall(llvm_get_function_type(func_type), function, argsV),
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

void llvm_visitor::visit(const literal_expression& lit) {
    func::lit_val val = lit.get_val();

    if(auto* v = std::get_if<int>(&val)) {
        Value* res = ConstantInt::get(ctx, APInt(32, *v, true));
        result = TypedValuePtr{ res, std::make_unique<int_type>() };
    } else if(auto* v = std::get_if<bool>(&val)) {
        int boolified_int = v ? 1 : 0;
        Value* res = ConstantInt::get(ctx, APInt(1, boolified_int, false));
        result = TypedValuePtr{ res, std::make_unique<bool_type>() };
    } else if(auto* v = std::get_if<std::string>(&val)) {
        ArrayType* arr_ty = ArrayType::get(llvm_get_type(types::INT), v->length() + 1); // Size with null terminator
        AllocaInst* stack_str = builder.CreateAlloca(arr_ty, nullptr, "stack_str");

        for(int i = 0; i < v->length() + 1; ++i) {
            auto idx = ConstantInt::get(llvm_get_type(types::INT), i);
            auto gep =
            builder.CreateGEP(arr_ty, stack_str,
                              { ConstantInt::get(llvm_get_type(types::INT), 0), idx },
                              "str_" + std::to_string(i));

            char symbol = '\0';
            if(i != v->length())
                symbol = (*v)[i];
            builder.CreateStore(ConstantInt::get(llvm_get_type(types::INT), symbol), gep);
        }

        Value* res = builder.CreateBitCast(stack_str, llvm_get_type(types::STRING));
        result = TypedValuePtr{ res, std::make_unique<string_type>() };
    }
};

void llvm_visitor::visit(const return_statement& node) {
    if(node.get_exp() == nullptr) {
        Value* res = builder.CreateRetVoid();
        result = TypedValuePtr{ res, std::make_unique<void_type>() };
        return;
    }

    auto ret = std::get<TypedValuePtr>(node.get_exp()->accept_with_result(*this));
    Value* res = builder.CreateRet(ret.ptr);
    result = TypedValuePtr{ res, ret.type_obj->clone() };
};


void llvm_visitor::visit(const binop_expression&) {};
void llvm_visitor::visit(const unarop_expression&) {};
void llvm_visitor::visit(const subscript_expression&) {};

void llvm_visitor::visit(const subscript_assign_statement&) {};
void llvm_visitor::visit(const assign_statement&) {};
void llvm_visitor::visit(const if_statement&) {};
void llvm_visitor::visit(const while_statement&) {};


} // namespace func
