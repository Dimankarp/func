#pragma once

#include "codegen/location.hh"
#include "node/ast.hpp"
#include "node/function.hpp"
#include <utility>
#include <vector>

namespace func {
class program : public ast_node_impl<program> {
    using Base = ast_node_impl<program>;
    std::vector<unique_ptr<func::function>> functions;

    public:
    program(std::vector<unique_ptr<func::function>>&& funcs, yy::location loc)
    : Base{ loc }, functions{ std::move(funcs) } {}
    const std::vector<unique_ptr<func::function>>& get_funcs() const {
        return functions;
    }
};

} // namespace func