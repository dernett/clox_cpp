#ifndef clox_value_h
#define clox_value_h

#include <cassert>
#include <print>

#include "object.hpp"

namespace clox {

enum ValueType : uint8_t { VAL_BOOL, VAL_NIL, VAL_NUMBER, VAL_OBJ };

class Value {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj *obj;
  };

public:
  [[nodiscard]] static Value Bool(bool value) {
    Value v{};
    v.type = VAL_BOOL;
    v.boolean = value;
    return v;
  }

  [[nodiscard]] static Value Nil() {
    Value v{};
    v.type = VAL_NIL;
    return v;
  }

  [[nodiscard]] static Value Number(double value) {
    Value v{};
    v.type = VAL_NUMBER;
    v.number = value;
    return v;
  }

  [[nodiscard]] static Value Object(Obj *obj) {
    Value v{};
    v.type = VAL_OBJ;
    v.obj = obj;
    return v;
  }

  [[nodiscard]] bool isBool() const { return getType() == VAL_BOOL; }

  [[nodiscard]] bool isNil() const { return getType() == VAL_NIL; }

  [[nodiscard]] bool isNumber() const { return getType() == VAL_NUMBER; }

  [[nodiscard]] bool isObj() const { return getType() == VAL_OBJ; }

  [[nodiscard]] ObjType objType() const { return asObj()->getType(); }

  [[nodiscard]] bool isObjType(ObjType objtype) const {
    return isObj() && objType() == objtype;
  }

  [[nodiscard]] bool isString() const { return isObjType(OBJ_STRING); }

  [[nodiscard]] bool isFalsey() const {
    return isNil() || (isBool() && !asBool());
  }

  [[nodiscard]] bool asBool() const {
    assert(isBool());
    return boolean;
  }

  [[nodiscard]] double asNumber() const {
    assert(isNumber());
    return number;
  }

  [[nodiscard]] Obj *asObj() const {
    assert(isObj());
    return obj;
  }

  [[nodiscard]] ObjString *asString() const {
    assert(isString());
    return static_cast<ObjString *>(asObj());
  }

  [[nodiscard]] ValueType getType() const { return type; }

  friend bool operator==(const Value &a, const Value &b) {
    if (a.getType() != b.getType())
      return false;
    switch (a.getType()) {
    case VAL_BOOL:
      return a.asBool() == b.asBool();
    case VAL_NIL:
      return true;
    case VAL_NUMBER:
      return a.asNumber() == b.asNumber();
    case VAL_OBJ:
      return *a.asString() == *b.asString();
    }
  }

private:
  Value() = default;
};
}; // namespace clox

template <>
struct std::formatter<clox::Value> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const clox::Value &value, std::format_context &ctx) const {
    switch (value.getType()) {
    case clox::VAL_BOOL:
      return std::format_to(ctx.out(), "{}", value.asBool());
    case clox::VAL_NIL:
      return std::format_to(ctx.out(), "nil");
    case clox::VAL_NUMBER:
      return std::format_to(ctx.out(), "{}", value.asNumber());
    case clox::VAL_OBJ:
      return std::format_to(ctx.out(), "{}", value.asString()->getString());
    }
  }
};

#endif
