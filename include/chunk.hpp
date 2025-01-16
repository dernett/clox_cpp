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
  OP_RETURN,
};

class Chunk {
  std::vector<uint8_t> code;
  std::vector<int> lines;
  std::vector<std::unique_ptr<Value>> constants;

public:
  void write(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
  }

  size_t addConstant(std::unique_ptr<Value> value) {
    constants.push_back(std::move(value));
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
    std::print("{:<16} {:4} '", name, constant);
    constants[constant]->print();
    std::println("'");
    return offset + 2;
  }

  size_t simpleInstruction(std::string_view name, size_t offset) const {
    std::println("{}", name);
    return offset + 1;
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
