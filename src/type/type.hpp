#pragma once

#include <memory>
#include <variant>
#include <vector>
namespace intrp {

enum class types : char { INT, STRING, BOOL, VOID, FUNCTION };

class type {
public:
  virtual types get_type() const = 0;
  virtual std::unique_ptr<type> clone() const = 0;
  virtual ~type() = default;
  virtual bool equals(const type &t) const {
    return get_type() == t.get_type();
  }
};

class int_type : public type {
public:
  static types static_get_type();
  types get_type() const override;
  std::unique_ptr<type> clone() const override;
};

class string_type : public type {
public:
  static types static_get_type();
  types get_type() const override;
  std::unique_ptr<type> clone() const override;
};

class bool_type : public type {
public:
  static types static_get_type();
  types get_type() const override;
  std::unique_ptr<type> clone() const override;
};

class void_type : public type {
public:
  static types static_get_type();
  types get_type() const override;
  std::unique_ptr<type> clone() const override;
};

class function_type : public type {
  std::vector<std::unique_ptr<type>> signature;

public:
  static types static_get_type();
  function_type(std::vector<std::unique_ptr<type>> &&sign);
  const std::vector<std::unique_ptr<type>> &get_signature();
  bool equals(const type &f) const override;
  types get_type() const override;
  std::unique_ptr<type> clone() const override;
};

} // namespace intrp
