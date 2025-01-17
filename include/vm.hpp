#ifndef clox_vm_h
#define clox_vm_h

#include <cstdint>

#include "compiler.hpp"
#include "chunk.hpp"

namespace clox {
enum InterpretResult : uint8_t {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
};

class VM {
  Chunk chunk;
  size_t ip = 0;
  std::vector<Value> stack;

public:
  explicit VM(Chunk &&chunk) : chunk(std::move(chunk)) {}

  InterpretResult interpret(const char *source) {
    compile(source);
    return INTERPRET_OK;
  }

  void push(Value value) { stack.push_back(value); }

  Value pop() {
    assert(!stack.empty());
    Value value = stack.back();
    stack.pop_back();
    return value;
  }

  InterpretResult run();

private:
  uint8_t readByte() { return chunk.getCode(ip++); }

  Value readConstant() { return chunk.getConstant(readByte()); }
};

}; // namespace clox

#endif
