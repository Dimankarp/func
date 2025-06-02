#pragma once

#include "location.hh"
#include <memory>
#include <string>
#include <variant>
namespace intrp {

class expression_visitor;
using std::unique_ptr;
using lit_val = std::variant<std::string, int, bool>;

class expression {
private:
  yy::location loc;

public:
  virtual ~expression() = default;
  virtual void accept(expression_visitor &) = 0;
  expression(yy::location loc);
  yy::location get_loc() const;
};

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

class binop_expression : public expression {
private:
  binop op;
  unique_ptr<expression> left;
  unique_ptr<expression> right;

public:
  binop_expression(binop op, unique_ptr<expression> left,
                   unique_ptr<expression> right, yy::location loc);
  void accept(expression_visitor &visitor) override;
  binop get_op() const;
  const unique_ptr<expression> &get_left() const;
  const unique_ptr<expression> &get_right() const;
};

enum class unarop : char { MINUS, NOT };

class unarop_expression : public expression {
private:
  unarop op;
  unique_ptr<expression> exp;

public:
  unarop_expression(unarop op, unique_ptr<expression> exp, yy::location loc);
  void accept(expression_visitor &visitor) override;
  unarop get_op() const;
  const unique_ptr<expression> &get_exp() const;
};

class literal_expression : public expression {
private:
  lit_val val;

public:
  literal_expression(lit_val val, yy::location loc);
  void accept(expression_visitor &visitor) override;
  lit_val get_val() const;
};

class identifier_expression : public expression {
private:
  std::string identificator;

public:
  identifier_expression(std::string identificator, yy::location loc);
  void accept(expression_visitor &visitor) override;
  const std::string &get_identificator() const;
};

} // namespace intrp