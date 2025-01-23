#ifndef clox_vm_h
#define clox_vm_h

#include <cstdint>
#include <iostream>
#include <memory_resource>

#include "chunk.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "value.hpp"

namespace clox {
enum InterpretResult : uint8_t {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
};

class VM {
  GCResource resource;
  std::pmr::polymorphic_allocator<> allocator;

  Chunk chunk;
  size_t ip = 0;
  std::vector<Value> stack;
  std::vector<Obj *> objects;

public:
  explicit VM()
      : resource(GCResource(*this)), allocator(&resource), chunk(allocator) {}

  VM(const VM &) = delete;
  VM &operator=(const VM &) = delete;
  VM(VM &&) = default;
  VM &operator=(VM &&) = delete;

  ~VM() {
    for (Obj *obj : objects) {
      switch (obj->getType()) {
      case OBJ_STRING:
        allocator.delete_object(static_cast<ObjString *>(obj));
        break;
      }
    }
  }

  InterpretResult interpret(const char *source);

  Chunk &getChunk() { return chunk; }

  void push(Value value) { stack.push_back(value); }

  [[nodiscard]] Value peek(size_t distance) const {
    assert(distance < stack.size());
    return stack[stack.size() - 1 - distance];
  }

  Value pop() {
    assert(!stack.empty());
    Value value = stack.back();
    stack.pop_back();
    return value;
  }

  template <class ValueType, class BinaryOp>
  InterpretResult binaryOp(ValueType valueType, BinaryOp op) {
    if (!peek(0).isNumber() || !peek(1).isNumber()) {
      runtimeError("Operands must be numbers.");
      return INTERPRET_RUNTIME_ERROR;
    }
    double b = pop().asNumber();
    double a = pop().asNumber();
    push(valueType(op(a, b)));
    return INTERPRET_OK;
  }

  InterpretResult run();

  template <typename ObjType, typename... Args>
  ObjType *allocateObject(Args... args) {
    ObjType *obj = allocator.new_object<ObjType>(args...);
    objects.push_back(obj);
    return obj;
  }

private:
  uint8_t readByte() { return chunk.getCode(ip++); }

  Value readConstant() { return chunk.getConstant(readByte()); }

  template <typename... Args>
  void runtimeError(std::format_string<Args...> fmt, Args &&...args) {
    std::println(std::cerr, fmt, std::forward(args)...);

    int line = chunk.getLine(ip - 1);
    std::println(std::cerr, "[line {}] in script", line);
    stack.clear();
  }
};

}; // namespace clox

#endif
