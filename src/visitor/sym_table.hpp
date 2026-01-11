#pragma once

#include "exception.hpp"
#include <list>

namespace func {

template <typename SymInfo> class sym_table {
    std::list<SymInfo> table;

    public:
    void start_block() {
        SymInfo delim{};
        delim.is_delimeter = true;
        table.push_back(std::move(delim));
    }

    void end_block() {
        while(!table.back().is_delimeter) {
            table.pop_back();
        }
        table.pop_back();
    }

    void add(SymInfo&& sym) {

        auto iter = table.rbegin();
        while(iter != table.rend() && !iter->is_delimeter) {
            if(iter->name == sym.name)
                throw symbol_redeclaratione_exception{ sym.name, iter->declare_loc,
                                                       sym.declare_loc };
            iter++;
        }
        table.push_back(std::move(sym));
    }

    const SymInfo& find(const std::string& sym) {
        auto iter = table.rbegin();
        while(iter != table.rend()) {
            if(!iter->is_delimeter && iter->name == sym)
                return *iter;
            iter++;
        }
        throw symbol_not_found_exception({ sym, yy::location{} });
    }
};

} // namespace func
