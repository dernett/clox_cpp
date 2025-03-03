#ifndef clox_chunk_h
#define clox_chunk_h

#include <cstdint>
#include <print>
#include <string_view>
#include <vector>

#include "value.hpp"

namespace clox {
enum OpCode : uint8_t {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_SET_GLOBAL,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_RETURN,
};

class Chunk {
  std::pmr::vector<uint8_t> code;
  std::pmr::vector<int> lines;
  std::pmr::vector<Value> constants;

public:
  using allocator_type = std::pmr::polymorphic_allocator<>;

  Chunk() = default;
  explicit Chunk(const allocator_type &allocator)
      : code(allocator), lines(allocator), constants(allocator) {}

  allocator_type get_allocator() const { return code.get_allocator(); }

  [[nodiscard]] uint8_t getCode(size_t index) const { return code[index]; }

  [[nodiscard]] int getLine(size_t index) const { return lines[index]; }

  [[nodiscard]] Value getConstant(size_t index) const {
    return constants[index];
  }

  void write(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
  }

  size_t addConstant(Value value) {
    constants.push_back(value);
    return constants.size() - 1;
  }

  void disassemble(std::string_view name) const {
    std::println("== {} ==", name);

    for (size_t offset = 0; offset < code.size();) {
      offset = disassembleInstruction(offset);
    }
  }

  size_t constantInstruction(std::string_view name, size_t offset) const {
    uint8_t constant = code[offset + 1];
    std::println("{:<16} {:4} '{}'", name, constant, constants[constant]);
    return offset + 2;
  }

  size_t simpleInstruction(std::string_view name, size_t offset) const {
    std::println("{}", name);
    return offset + 1;
  }

  size_t byteInstruction(std::string_view name, size_t offset) const {
    uint8_t slot = code[offset + 1];
    std::println("{:<16} {:4}", name, slot);
    return offset + 2;
  }

  size_t disassembleInstruction(size_t offset) const {
    std::print("{:04} ", offset);

    if (offset > 0 && lines[offset] == lines[offset - 1]) {
      std::print("   | ");
    } else {
      std::print("{:4} ", lines[offset]);
    }

    uint8_t instruction = code[offset];
    switch (instruction) {
    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", offset);
    case OP_NIL:
      return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);
    case OP_POP:
      return simpleInstruction("OP_POP", offset);
    case OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL", offset);
    case OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL", offset);
    case OP_GET_GLOBAL:
      return constantInstruction("OP_GET_GLOBAL", offset);
    case OP_DEFINE_GLOBAL:
      return constantInstruction("OP_DEFINE_GLOBAL", offset);
    case OP_SET_GLOBAL:
      return constantInstruction("OP_SET_GLOBAL", offset);
    case OP_EQUAL:
      return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
      return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
      return simpleInstruction("OP_LESS", offset);
    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
    case OP_NOT:
      return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case OP_PRINT:
      return simpleInstruction("OP_PRINT", offset);
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    default:
      std::println("Unknown opcode: {}", instruction);
      return offset + 1;
    }
  }
};
} // namespace clox

#endif
