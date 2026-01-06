#pragma once

#include <memory>
#include <string>
#include <vector>
namespace func {

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

template <typename Derived, types Tp> class type_impl : public type {
public:
  static const types type_enum = Tp;
  types get_type() const override { return Tp; }
  std::unique_ptr<type> clone() const override {
    return std::make_unique<Derived>(static_cast<const Derived &>(*this));
  }

private:
  type_impl() = default;
  friend Derived;
};

std::string types_to_string(func::types t);
std::string type_to_string(const func::type &t);

class int_type : public type_impl<int_type, types::INT> {
public:
  using type_impl::type_enum;
};

class string_type : public type_impl<string_type, types::STRING> {
public:
  using type_impl::type_enum;
};

class bool_type : public type_impl<bool_type, types::BOOL> {
public:
  using type_impl::type_enum;
};

class void_type : public type_impl<void_type, types::VOID> {
public:
  using type_impl::type_enum;
};

class function_type : public type_impl<function_type, types::FUNCTION> {
  std::vector<std::unique_ptr<type>> signature;

public:
  function_type(const function_type &);

public:
  using type_impl::type_enum;
  function_type(std::vector<std::unique_ptr<type>> &&sign);
  const std::vector<std::unique_ptr<type>> &get_signature() const;
  bool equals(const type &f) const override;
};

} // namespace func
