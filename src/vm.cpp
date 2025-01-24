#include "vm.hpp"
#include "common.hpp"
#include "compiler.hpp"

#include <functional>

namespace clox {

InterpretResult VM::interpret(const char *source) {
  chunk = Chunk(allocator);
  Compiler compiler(source, *this);

  if (!compiler.compile()) {
    return INTERPRET_COMPILE_ERROR;
  }

  ip = 0;

  return run();
}

InterpretResult VM::run() {
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    std::print("          ");
    if (stack.empty()) {
      std::print("<empty>");
    }
    for (Value value : stack) {
      std::print("[ {} ]", value);
    }
    std::println();
    chunk.disassembleInstruction(ip);
#endif
    InterpretResult flag = INTERPRET_OK;
    auto instruction = static_cast<OpCode>(readByte());
    switch (instruction) {
    case OP_CONSTANT:
      push(readConstant());
      break;
    case OP_NIL:
      push(Value::Nil());
      break;
    case OP_TRUE:
      push(Value::Bool(true));
      break;
    case OP_FALSE:
      push(Value::Bool(false));
      break;
    case OP_POP:
      pop();
      break;
    case OP_GET_GLOBAL: {
      ObjString *name = readString();
      if (auto it = globals.find(name); it != globals.end()) {
        push(it->second);
        break;
      }
      runtimeError("Undefined variable '{}'.", name->getString());
      return INTERPRET_RUNTIME_ERROR;
    }
    case OP_DEFINE_GLOBAL: {
      ObjString *name = readString();
      globals.insert_or_assign(name, peek(0));
      pop();
      break;
    }
    case OP_SET_GLOBAL: {
      ObjString *name = readString();
      if (!globals.contains(name)) {
        runtimeError("Undefined variable '{}'.", name->getString());
        return INTERPRET_RUNTIME_ERROR;
      }
      globals.insert_or_assign(name, peek(0));
      break;
    }
    case OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(Value::Bool(a == b));
      break;
    }
    case OP_GREATER:
      flag = binaryOp(Value::Bool, std::greater());
      break;
    case OP_LESS:
      flag = binaryOp(Value::Bool, std::less());
      break;
    case OP_ADD: {
      if (peek(0).isString() && peek(1).isString()) {
        ObjString *b = pop().asString();
        ObjString *a = pop().asString();
        ObjString *result = takeString(a->getString() + b->getString());
        push(Value::Object(result));
      } else if (peek(0).isNumber() && peek(1).isNumber()) {
        double b = pop().asNumber();
        double a = pop().asNumber();
        push(Value::Number(a + b));
      }
      break;
    }
    case OP_SUBTRACT:
      flag = binaryOp(Value::Number, std::minus());
      break;
    case OP_MULTIPLY:
      flag = binaryOp(Value::Number, std::multiplies());
      break;
    case OP_DIVIDE:
      flag = binaryOp(Value::Number, std::divides());
      break;
    case OP_NOT:
      push(Value::Bool(pop().isFalsey()));
      break;
    case OP_NEGATE:
      if (!peek(0).isNumber()) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      push(Value::Number(-pop().asNumber()));
      break;
    case OP_PRINT:
      std::println("{}", pop());
      break;
    case OP_RETURN:
      // Exit interpreter.
      return INTERPRET_OK;
    }
    if (flag != INTERPRET_OK)
      return flag;
  }
}
} // namespace clox
