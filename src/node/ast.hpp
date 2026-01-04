#ifndef NODE_AST_HPP_
#define NODE_AST_HPP_

#include "location.hh"
#include "visitor/visitor.hpp"
#include <memory>

namespace intrp {

using std::unique_ptr;

class visitable_base {
public:
  template <typename ReturnType>
  ReturnType &&accept_with_result(visitor<ReturnType> &visitor) {
    accept(visitor);
    return visitor.extract_result();
  }

  virtual void accept(visitor_base &v) const = 0;
  virtual ~visitable_base() = default;
};

class ast_node : public visitable_base {
private:
  yy::location loc;

public:
  virtual ~ast_node() = default;
  explicit ast_node(yy::location loc);
  yy::location get_loc() const;
};

template <typename Derived> class ast_node_impl : public ast_node {
public:
  using ast_node::ast_node;
  void accept(visitor_base &v) const override {
    return v.visit(static_cast<const Derived &>(*this));
  }
  virtual ~ast_node_impl() = default;
};

} // namespace intrp
#endif // NODE_AST_HPP_