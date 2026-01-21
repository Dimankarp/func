#include "visitor/llvm_visitor/llvm_visitor.hpp"
#include "exception.hpp"
#include "node/function.hpp"
#include "node/program.hpp"
#include "type/type.hpp"
#include "type/type_checker.hpp"
#include "utils.hpp"
#include "visitor/sym_table.hpp"
#include "llvm/IR/Verifier.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
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
    module.print(code_out, nullptr);
}


Type* llvm_visitor::llvm_get_type(types t) {
    switch(t) {
    case types::INT: return Type::getInt32Ty(ctx);
    case types::STRING: return PointerType::get(ctx, 0);
    case types::BOOL: return Type::getIntNTy(ctx, 1);
    case types::VOID: return Type::getVoidTy(ctx);
    case types::FUNCTION: return PointerType::get(ctx, 0);
    default: panic(1, "Invalid type in llvm_get_type: %d", t);
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
    if(Function* fm = module.getFunction(node.get_identifier())) {
        auto sym = table.find(node.get_identifier());
        if(ft != fm->getFunctionType()) {
            throw symbol_redeclaration_exception{ sym.name, node.get_loc(), sym.declare_loc };
        }
        result = fm;
        return;
    }

    // Otherwise create new
    Function* f = Function::Create(ft, Function::LinkageTypes::ExternalLinkage,
                                   node.get_identifier(), module);

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

    if(node.get_block() == nullptr) {
        result = f;
        return;
    }

    if (!f->empty()){
        throw syntax_exception{ "Multiple defenition of function '" + node.get_identifier() + "'.",
                                node.get_loc() };
    }

    BasicBlock* bb = BasicBlock::Create(ctx, "entry", f);
    builder.SetInsertPoint(bb);

    table.start_block(); // start
    IRBuilder<> temp_b(&f->getEntryBlock(), f->getEntryBlock().begin());

    int i = 0;
    for(auto& arg : f->args()) {
        const auto& param = node.get_params()[i];

        Value* alloc = temp_b.CreateAlloca(llvm_get_type(param.get_type()->get_type()),
                                           nullptr, param.get_identifier());
        builder.CreateStore(&arg, alloc);

        // Registering in sym_table
        table.add(llvm_sym_info{ param.get_identifier(), param.get_type()->clone(), alloc });

        i++;
    }

    node.get_block()->accept(*this);

    table.end_block(); // end


    // Current basicblock might change during node.get_block()->accept(*this);
    bb = builder.GetInsertBlock();

    // Insert return for a root non basic block of a function if needed
    Instruction* terminator = bb->getTerminator();
    if(terminator == nullptr && f->getReturnType()->isVoidTy()) {
        builder.SetInsertPoint(bb);
        builder.CreateRetVoid();
    }

    // Verify the function body
    std::string error_msg;
    llvm::raw_string_ostream os(error_msg);
    if(llvm::verifyFunction(*f, &os)) {
        throw unexpected_type_exception {error_msg, node.get_loc()};
    }

    fpm.run(*f, fam);

    result = f;
}

void llvm_visitor::visit(const block_statement& node) {
    table.start_block(); // start

    for(const auto& st : node.get_statements()) {
        st->accept(*this);

        if(builder.GetInsertBlock()->getTerminator() != nullptr)
            break;
    }

    table.end_block(); // end
};


FunctionType* llvm_visitor::llvm_get_function_type(const function_type& func_type) {
    Type* result_type = llvm_get_type(func_type.get_return_type()->get_type());
    std::vector<Type*> params_type;
    const auto& signature = func_type.get_signature();
    const bool has_params = signature.front()->get_type() != types::VOID;

    if(has_params) {
        for(int i = 0; i < signature.size() - 1; i++) {
            params_type.push_back(llvm_get_type(signature[i]->get_type()));
        }
    }

    return FunctionType::get(result_type, params_type, false);
}


void llvm_visitor::visit(const function_call& node) {
    auto func = node.get_func()->accept_with_result(*this);

    const function_type& func_type = std::holds_alternative<TypedFunctionPtr>(func) ?
    dynamic_cast<const function_type&>(*std::get<TypedFunctionPtr>(func).type_obj) :
    dynamic_cast<const function_type&>(*std::get<TypedValuePtr>(func).type_obj);

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
        auto arg = turn_to_typed_value_ptr(args[i]->accept_with_result(*this));
        expect_types(*func_type.get_signature()[i], *arg.type_obj, args[i]->get_loc());
        argsV.push_back(arg.ptr);
    }


    if(std::holds_alternative<TypedFunctionPtr>(func)) {
        Function* function = std::get<TypedFunctionPtr>(func).ptr;
        result = TypedValuePtr{ builder.CreateCall(function, argsV),
                                func_type.get_return_type()->clone() };
    } else {
        Value* function = std::get<TypedValuePtr>(func).ptr;
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

    // if it's a value, it must be on the stack
    result =
    TypedValuePtr{ builder.CreateLoad(llvm_get_type(sym.type_obj->get_type()),
                                      sym.value.value(), node.get_identificator()),
                   std::move(sym.type_obj) };
};

void llvm_visitor::visit(const literal_expression& lit) {
    func::lit_val val = lit.get_val();

    if(auto* v = std::get_if<int>(&val)) {
        Value* res = ConstantInt::get(ctx, APInt(32, *v, true));
        result = TypedValuePtr{ res, std::make_unique<int_type>() };
    } else if(auto* v = std::get_if<bool>(&val)) {
        int boolified_int = *v ? 1 : 0;
        Value* res = ConstantInt::get(ctx, APInt(1, boolified_int, false));
        result = TypedValuePtr{ res, std::make_unique<bool_type>() };
    } else if(auto* v = std::get_if<std::string>(&val)) {
        ArrayType* arr_ty = ArrayType::get(llvm_get_type(types::INT), v->length() + 1); // Size with null terminator
        AllocaInst* stack_str = builder.CreateAlloca(arr_ty, nullptr, "stack_str");

        for(int i = 0; i < v->length() + 1; ++i) {
            auto* idx = ConstantInt::get(llvm_get_type(types::INT), i);
            auto* gep =
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

void llvm_visitor::visit(const assign_statement& node) {
    /*
    3 Variants:
    int a;
    int a = 42;
    a = 42;

    (int-int) a = foo;
    */

    if(node.get_type() != nullptr) {
        // push on stack
        Function* f = builder.GetInsertBlock()->getParent();
        IRBuilder<> temp_b(&f->getEntryBlock(), f->getEntryBlock().begin());
        Value* alloc = temp_b.CreateAlloca(llvm_get_type(node.get_type()->get_type()),
                                           nullptr, node.get_identifier());

        // add to sym_table
        table.add(llvm_sym_info{ node.get_identifier(), node.get_type()->clone(), alloc });
    }

    // evaluate expression and save result
    if(node.get_exp() == nullptr)
        return;

    const llvm_sym_info& sym = table.find(node.get_identifier());
    if (!sym.value.has_value()) {
        throw syntax_exception{ "assignment to declared function '" + sym.name + "'.", node.get_loc() };
    }

    auto expr = turn_to_typed_value_ptr(node.get_exp()->accept_with_result(*this));
    expect_types(*sym.type_obj, *expr.type_obj, node.get_exp()->get_loc());
    builder.CreateStore(expr.ptr, sym.value.value());
};

TypedValuePtr llvm_visitor::turn_to_typed_value_ptr(llvm_result res) {
    if(std::holds_alternative<TypedValuePtr>(res)) {
        return std::move(std::get<TypedValuePtr>(res));
    }

    if(std::holds_alternative<TypedFunctionPtr>(res)) {
        auto f = std::move(std::get<TypedFunctionPtr>(res));
        Value* ptr = builder.CreateBitCast(f.ptr, PointerType::get(ctx, 0));
        return TypedValuePtr{ ptr, std::move(f.type_obj) };
    }

    throw global_syntax_exception{ "Can't turn result into typed value." };
}

void llvm_visitor::visit(const binop_expression& node) {
    auto lv = turn_to_typed_value_ptr(node.get_left()->accept_with_result(*this));
    auto rv = turn_to_typed_value_ptr(node.get_right()->accept_with_result(*this));


    if(lv.type_obj->get_type() != rv.type_obj->get_type()) {
        throw unexpected_type_exception{
            { "Mismatched types in binary operation: " +
              func::types_to_string(lv.type_obj->get_type()) + " and " +
              func::types_to_string(rv.type_obj->get_type()),
              node.get_loc() }
        };
    }

    Value* res;
    try {
        switch(node.get_op()) {
        case binop::ADD:
            expect_types(int_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateAdd(lv.ptr, rv.ptr, "addtmp");
            this->result = TypedValuePtr{ res, std::make_unique<int_type>() };
            break;
        case binop::SUB:
            expect_types(int_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateSub(lv.ptr, rv.ptr, "subtmp");
            this->result = TypedValuePtr{ res, std::make_unique<int_type>() };
            break;
        case binop::MUL:
            expect_types(int_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateMul(lv.ptr, rv.ptr, "multmp");
            this->result = TypedValuePtr{ res, std::make_unique<int_type>() };
            break;
        case binop::DIV:
            expect_types(int_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateSDiv(lv.ptr, rv.ptr, "divtmp");
            this->result = TypedValuePtr{ res, std::make_unique<int_type>() };
            break;
        case binop::MOD:
            expect_types(int_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateSRem(lv.ptr, rv.ptr, "modtmp");
            this->result = TypedValuePtr{ res, std::make_unique<int_type>() };
            break;
        case binop::LESS:
            expect_types(int_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateICmp(CmpInst::ICMP_SLT, lv.ptr, rv.ptr, "lstmp");
            this->result = TypedValuePtr{ res, std::make_unique<bool_type>() };
            break;
        case binop::GRTR:
            expect_types(int_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateICmp(CmpInst::ICMP_SGT, lv.ptr, rv.ptr, "grtmp");
            this->result = TypedValuePtr{ res, std::make_unique<bool_type>() };
            break;
        case binop::EQ:
            res = builder.CreateICmp(CmpInst::ICMP_EQ, lv.ptr, rv.ptr, "eqstmp");
            this->result = TypedValuePtr{ res, std::make_unique<bool_type>() };
            break;
        case binop::NEQ:
            res = builder.CreateICmp(CmpInst::ICMP_NE, lv.ptr, rv.ptr, "netmp");
            this->result = TypedValuePtr{ res, std::make_unique<bool_type>() };
            break;
        case binop::OR:
            expect_types(bool_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateOr(lv.ptr, rv.ptr, "ortmp");
            this->result = TypedValuePtr{ res, std::make_unique<bool_type>() };
            break;
        case binop::AND:
            expect_types(bool_type{}, *lv.type_obj, node.get_loc());
            res = builder.CreateAnd(lv.ptr, rv.ptr, "andtmp");
            this->result = TypedValuePtr{ res, std::make_unique<bool_type>() };
            break;
        }
    } catch(unexpected_type_exception& e) {
        e.loc = node.get_loc();
        throw e;
    }
};

void llvm_visitor::visit(const unarop_expression& node) {
    auto expr = node.get_exp()->accept_with_result(*this);

    if(std::holds_alternative<TypedFunctionPtr>(expr)) {
        throw syntax_exception{ "Invalid unary operation on function type", node.get_loc() };
    }

    const auto& val = std::get<TypedValuePtr>(expr);

    Value* res;
    try {
        switch(node.get_op()) {
        case unarop::MINUS: {
            expect_types(int_type{}, *val.type_obj, node.get_loc());
            Value* zero = ConstantInt::get(ctx, APInt(32, 0, true));
            res = builder.CreateSub(zero, val.ptr, "negtmp");
            this->result = TypedValuePtr{ res, std::make_unique<int_type>() };
            break;
        }
        case unarop::NOT: {
            expect_types(bool_type{}, *val.type_obj, node.get_loc());
            Value* one = ConstantInt::get(ctx, APInt(1, 1, true));
            res = builder.CreateXor(one, val.ptr, "nottmp");
            this->result = TypedValuePtr{ res, std::make_unique<bool_type>() };
            break;
        }
        }
    } catch(unexpected_type_exception& e) {
        e.loc = node.get_loc();
        throw e;
    }
};


namespace {
void CreateBrIfNotTerminated(IRBuilder<>& builder, BasicBlock* dest) {
    if(builder.GetInsertBlock()->getTerminator() == nullptr)
        builder.CreateBr(dest);
}
} // namespace

void llvm_visitor::visit(const if_statement& node) {

    TypedValuePtr cond =
    std::get<TypedValuePtr>(node.get_condition()->accept_with_result(*this));

    expect_types(bool_type{}, *cond.type_obj, node.get_loc());

    Function* function = builder.GetInsertBlock()->getParent();

    BasicBlock* thenb = BasicBlock::Create(ctx, "then", function);
    BasicBlock* elseb = BasicBlock::Create(ctx, "else");
    BasicBlock* mergeb = BasicBlock::Create(ctx, "ifend");

    builder.CreateCondBr(cond.ptr, thenb, elseb);

    builder.SetInsertPoint(thenb);
    node.get_then_block()->accept(*this);
    CreateBrIfNotTerminated(builder, mergeb);

    function->insert(function->end(), elseb);
    builder.SetInsertPoint(elseb);
    // Generating empty else block if else is omitted in original code
    if(node.get_else_block() != nullptr)
        node.get_else_block()->accept(*this);
    CreateBrIfNotTerminated(builder, mergeb);

    function->insert(function->end(), mergeb);
    builder.SetInsertPoint(mergeb);
}

void llvm_visitor::visit(const while_statement& node) {

    Function* function = builder.GetInsertBlock()->getParent();

    BasicBlock* condb = BasicBlock::Create(ctx, "while_cond", function);
    BasicBlock* loopb = BasicBlock::Create(ctx, "while_loop");
    BasicBlock* endb = BasicBlock::Create(ctx, "while_end");

    builder.CreateBr(condb);
    builder.SetInsertPoint(condb);
    TypedValuePtr cond =
    std::get<TypedValuePtr>(node.get_condition()->accept_with_result(*this));
    expect_types(bool_type{}, *cond.type_obj, node.get_loc());
    builder.CreateCondBr(cond.ptr, loopb, endb);

    function->insert(function->end(), loopb);
    builder.SetInsertPoint(loopb);
    node.get_block()->accept(*this);
    CreateBrIfNotTerminated(builder, condb);

    function->insert(function->end(), endb);
    builder.SetInsertPoint(endb);
};


void llvm_visitor::visit(const subscript_expression& node) {
    auto ptr = std::get<TypedValuePtr>(node.get_pointer()->accept_with_result(*this));
    expect_types(string_type{}, *ptr.type_obj, node.get_loc());

    auto idx = std::get<TypedValuePtr>(node.get_index()->accept_with_result(*this));
    expect_types(int_type{}, *idx.type_obj, node.get_loc());

    Value* elem_i_ptr = builder.CreateGEP(llvm_get_type(types::INT), ptr.ptr,
                                          { idx.ptr }, "array_elem_ptr");

    Value* loaded_val =
    builder.CreateLoad(llvm_get_type(types::INT), elem_i_ptr, "loaded_elem");

    this->result = TypedValuePtr{ loaded_val, std::make_unique<int_type>() };
};

void llvm_visitor::visit(const subscript_assign_statement& node) {

    auto ptr = std::get<TypedValuePtr>(node.get_pointer()->accept_with_result(*this));
    expect_types(string_type{}, *ptr.type_obj, node.get_loc());

    auto idx = std::get<TypedValuePtr>(node.get_index()->accept_with_result(*this));
    expect_types(int_type{}, *idx.type_obj, node.get_loc());

    auto expr = std::get<TypedValuePtr>(node.get_exp()->accept_with_result(*this));
    expect_types(int_type{}, *expr.type_obj, node.get_loc());

    Value* elem_i_ptr = builder.CreateGEP(llvm_get_type(types::INT), ptr.ptr,
                                          { idx.ptr }, "array_elem_ptr");

    builder.CreateStore(expr.ptr, elem_i_ptr);
};


} // namespace func
