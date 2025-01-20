#include <print>

#include "compiler.hpp"
#include "scanner.hpp"

namespace clox {

void Compiler::expression() { parsePrecedence(PREC_ASSIGNMENT); }

void Compiler::binary() {
  TokenType operatorType = previous.type;
  const ParseRule &rule = getRule(operatorType);
  parsePrecedence(static_cast<Precedence>(rule.precedence + 1));

  switch (operatorType) {
  case TOKEN_BANG_EQUAL:
    emitBytes(OP_EQUAL, OP_NOT);
    break;
  case TOKEN_EQUAL_EQUAL:
    emitByte(OP_EQUAL);
    break;
  case TOKEN_GREATER:
    emitByte(OP_GREATER);
    break;
  case TOKEN_GREATER_EQUAL:
    emitBytes(OP_LESS, OP_NOT);
    break;
  case TOKEN_LESS:
    emitByte(OP_LESS);
    break;
  case TOKEN_LESS_EQUAL:
    emitBytes(OP_GREATER, OP_NOT);
    break;
  case TOKEN_PLUS:
    emitByte(OP_ADD);
    break;
  case TOKEN_MINUS:
    emitByte(OP_SUBTRACT);
    break;
  case TOKEN_STAR:
    emitByte(OP_MULTIPLY);
    break;
  case TOKEN_SLASH:
    emitByte(OP_DIVIDE);
    break;
  default:
    std::unreachable();
  }
}

void Compiler::literal() {
  switch (previous.type) {
  case TOKEN_FALSE:
    emitByte(OP_FALSE);
    break;
  case TOKEN_NIL:
    emitByte(OP_NIL);
    break;
  case TOKEN_TRUE:
    emitByte(OP_TRUE);
    break;
  default:
    std::unreachable();
  }
}

void Compiler::grouping() {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::number() {
  // Would like to use `std::from_chars`, but it requires at least libc++ 20
  // This is okay since we know the source string is null-terminated
  double value = std::strtod(previous.str.data(), nullptr); // NOLINT
  emitConstant(Value::Number(value));
}

void Compiler::unary() {
  TokenType operatorType = previous.type;

  // Compile the operand
  parsePrecedence(PREC_UNARY);

  // Emit the operator instruction.
  switch (operatorType) {
  case TOKEN_BANG:
    emitByte(OP_NOT);
    break;
  case TOKEN_MINUS:
    emitByte(OP_NEGATE);
    break;
  default:
    std::unreachable();
  }
}

void Compiler::parsePrecedence(Precedence precedence) {
  advance();
  ParseFn prefixRule = getRule(previous.type).prefix;
  if (prefixRule == nullptr) {
    error("Expect expression.");
    return;
  }

  (this->*prefixRule)();

  while (precedence <= getRule(current.type).precedence) {
    advance();
    ParseFn infixRule = getRule(previous.type).infix;
    (this->*infixRule)();
  }
}

}; // namespace clox
