#include "common.hpp"
#include "vm.hpp"

namespace clox {
InterpretResult VM::run() {
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    std::print("          ");
    for (Value value : stack) {
      std::print("[ {} ]", value);
    }
    std::println();
    chunk.disassembleInstruction(ip);
#endif
    uint8_t instruction;
    switch (instruction = readByte()) {
    case OP_CONSTANT: {
      Value constant = readConstant();
      push(constant);
      break;
    }
    case OP_NEGATE:
      push(Value::Number(-pop().asNumber()));
      break;
    case OP_RETURN:
      std::println("{}", pop());
      return INTERPRET_OK;
    }
  }
}
} // namespace clox
