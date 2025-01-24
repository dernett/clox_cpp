#include <print>

#include "compiler.hpp"
#include "scanner.hpp"

namespace clox {

void Compiler::expression() { parsePrecedence(PREC_ASSIGNMENT); }

void Compiler::varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  defineVariable(global);
}

void Compiler::expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

void Compiler::printStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

void Compiler::synchronize() {
  panicMode = false;

  while (current.type != TOKEN_EOF) {
    if (previous.type == TOKEN_SEMICOLON)
      return;
    switch (current.type) {
    case TOKEN_CLASS:
    case TOKEN_FUN:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;
    default:
        // Do nothing.
        ;
    }

    advance();
  }
}

void Compiler::declaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (panicMode)
    synchronize();
}

void Compiler::statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else {
    expressionStatement();
  }
}

void Compiler::binary(bool /*canAssign*/) {
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

void Compiler::literal(bool /*canAssign*/) {
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

void Compiler::grouping(bool /*canAssign*/) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::number(bool /*canAssign*/) {
  // Would like to use `std::from_chars`, but it requires at least libc++ 20
  // This is okay since we know the source string is null-terminated
  double value = std::strtod(previous.str.data(), nullptr); // NOLINT
  emitConstant(Value::Number(value));
}

void Compiler::string(bool /*canAssign*/) {
  emitConstant(Value::Object(
      vm.copyString(previous.str.substr(1, previous.str.size() - 2))));
}

void Compiler::namedVariable(Token name, bool canAssign) {
  uint8_t arg = identifierConstant(name);

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(OP_SET_GLOBAL, arg);
  } else {
    emitBytes(OP_GET_GLOBAL, arg);
  }
}

void Compiler::variable(bool canAssign) { namedVariable(previous, canAssign); }

void Compiler::unary(bool /*canAssign*/) {
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

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  (this->*prefixRule)(canAssign);

  while (precedence <= getRule(current.type).precedence) {
    advance();
    ParseFn infixRule = getRule(previous.type).infix;
    (this->*infixRule)(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }
}

uint8_t Compiler::identifierConstant(Token &name) {
  return makeConstant(Value::Object(vm.copyString(name.str)));
}

uint8_t Compiler::parseVariable(std::string_view errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);
  return identifierConstant(previous);
}

void Compiler::defineVariable(uint8_t global) {
  emitBytes(OP_DEFINE_GLOBAL, global);
}

}; // namespace clox
