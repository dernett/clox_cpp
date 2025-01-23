#ifndef clox_compiler_h
#define clox_compiler_h

#include "chunk.hpp"
#include "scanner.hpp"
#include "vm.hpp"
#include <iostream>

namespace clox {

enum Precedence : uint8_t {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
};

// NOTE: This also behaves as a parser. We generate bytecode
// as we parse instead of creating an AST.
class Compiler {
  Token current;
  Token previous;
  bool hadError = false;
  bool panicMode = false;

  Scanner scanner;
  VM &vm;
  Chunk &chunk;

public:
  Compiler(const char *source, VM &vm)
      : scanner(source), vm(vm), chunk(vm.getChunk()) {}

  bool compile() {
    advance();

    while (!match(TOKEN_EOF)) {
      declaration();
    }

    endCompiler();
    return !hadError;
  }

private:
  void errorAt(const Token &token, std::string_view message) {
    if (panicMode)
      return;
    panicMode = true;
    std::print(std::cerr, "[line {}] Error", token.line);

    if (token.type == TOKEN_EOF) {
      std::print(std::cerr, " at end");
    } else if (token.type == TOKEN_ERROR) {
      // Nothing.
    } else {
      std::print(std::cerr, " at '{}'", token.str);
    }

    std::println(std::cerr, ": {}", message);
    hadError = true;
  }

  void errorAtCurrent(std::string_view message) { errorAt(current, message); }

  void error(std::string_view message) { errorAt(previous, message); }

  void advance() {
    previous = current;

    for (;;) {
      current = scanner.scanToken();
      if (current.type != TOKEN_ERROR)
        break;

      errorAtCurrent(current.str);
    }
  }

  void consume(TokenType type, std::string_view message) {
    if (current.type == type) {
      advance();
      return;
    }

    errorAtCurrent(message);
  }

  [[nodiscard]] bool check(TokenType type) const {
    return current.type == type;
  }

  bool match(TokenType type) {
    if (!check(type))
      return false;
    advance();
    return true;
  }

  void emitByte(uint8_t byte) { chunk.write(byte, previous.line); }

  void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
  }

  void emitReturn() { emitByte(OP_RETURN); }

  uint8_t makeConstant(Value value) {
    size_t constant = chunk.addConstant(value);
    if (constant > UINT8_MAX) {
      error("Too many constants in one chunk.");
      return 0;
    }

    return static_cast<uint8_t>(constant);
  }

  void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
  }

  void endCompiler() { emitReturn(); }

  void expression();

  void printStatement();

  void declaration();

  void statement();

  void binary();

  void literal();

  void grouping();

  void number();

  void string();

  void unary();

  void parsePrecedence(Precedence precedence);

  using ParseFn = void (Compiler::*)();

  struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
  };

  // clang-format off
  static constexpr ParseRule rules[] = {
      [TOKEN_LEFT_PAREN]    = {&Compiler::grouping, nullptr,           PREC_NONE      },
      [TOKEN_RIGHT_PAREN]   = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_LEFT_BRACE]    = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_RIGHT_BRACE]   = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_COMMA]         = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_DOT]           = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_MINUS]         = {&Compiler::unary,    &Compiler::binary, PREC_TERM      },
      [TOKEN_PLUS]          = {nullptr,             &Compiler::binary, PREC_TERM      },
      [TOKEN_SEMICOLON]     = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_SLASH]         = {nullptr,             &Compiler::binary, PREC_FACTOR    },
      [TOKEN_STAR]          = {nullptr,             &Compiler::binary, PREC_FACTOR    },
      [TOKEN_BANG]          = {&Compiler::unary,    nullptr,           PREC_NONE      },
      [TOKEN_BANG_EQUAL]    = {nullptr,             &Compiler::binary, PREC_EQUALITY  },
      [TOKEN_EQUAL]         = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_EQUAL_EQUAL]   = {nullptr,             &Compiler::binary, PREC_EQUALITY  },
      [TOKEN_GREATER]       = {nullptr,             &Compiler::binary, PREC_COMPARISON},
      [TOKEN_GREATER_EQUAL] = {nullptr,             &Compiler::binary, PREC_COMPARISON},
      [TOKEN_LESS]          = {nullptr,             &Compiler::binary, PREC_COMPARISON},
      [TOKEN_LESS_EQUAL]    = {nullptr,             &Compiler::binary, PREC_COMPARISON},
      [TOKEN_IDENTIFIER]    = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_STRING]        = {&Compiler::string,   nullptr,           PREC_NONE      },
      [TOKEN_NUMBER]        = {&Compiler::number,   nullptr,           PREC_NONE      },
      [TOKEN_AND]           = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_CLASS]         = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_ELSE]          = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_FALSE]         = {&Compiler::literal,  nullptr,           PREC_NONE      },
      [TOKEN_FOR]           = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_FUN]           = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_IF]            = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_NIL]           = {&Compiler::literal,  nullptr,           PREC_NONE      },
      [TOKEN_OR]            = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_PRINT]         = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_RETURN]        = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_SUPER]         = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_THIS]          = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_TRUE]          = {&Compiler::literal,  nullptr,           PREC_NONE      },
      [TOKEN_VAR]           = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_WHILE]         = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_ERROR]         = {nullptr,             nullptr,           PREC_NONE      },
      [TOKEN_EOF]           = {nullptr,             nullptr,           PREC_NONE      },
  };
  // clang-format on

  const ParseRule &getRule(TokenType type) const { return rules[type]; }
};

}; // namespace clox

#endif
