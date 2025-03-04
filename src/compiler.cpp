#include <print>

#include "compiler.hpp"
#include "scanner.hpp"

namespace clox {

void Emitter::expression() { parsePrecedence(PREC_ASSIGNMENT); }

void Emitter::block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Emitter::varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  defineVariable(global);
}

void Emitter::expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

void Emitter::printStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

void Emitter::synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON)
      return;
    switch (parser.current.type) {
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

void Emitter::declaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode)
    synchronize();
}

void Emitter::statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

void Emitter::binary(bool /*canAssign*/) {
  TokenType operatorType = parser.previous.type;
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

void Emitter::literal(bool /*canAssign*/) {
  switch (parser.previous.type) {
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

void Emitter::grouping(bool /*canAssign*/) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Emitter::number(bool /*canAssign*/) {
  // Would like to use `std::from_chars`, but it requires at least libc++ 20
  // This is okay since we know the source string is null-terminated
  double value = std::strtod(parser.previous.str.data(), nullptr); // NOLINT
  emitConstant(Value::Number(value));
}

void Emitter::string(bool /*canAssign*/) {
  emitConstant(Value::Object(vm.copyString(
      parser.previous.str.substr(1, parser.previous.str.size() - 2))));
}

void Emitter::namedVariable(Token name, bool canAssign) {
  uint8_t getOp = 0;
  uint8_t setOp = 0;
  int arg = resolveLocal(name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = identifierConstant(name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, static_cast<uint8_t>(arg));
  } else {
    emitBytes(getOp, static_cast<uint8_t>(arg));
  }
}

void Emitter::variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
}

void Emitter::unary(bool /*canAssign*/) {
  TokenType operatorType = parser.previous.type;

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

void Emitter::parsePrecedence(Precedence precedence) {
  advance();
  ParseFn prefixRule = getRule(parser.previous.type).prefix;
  if (prefixRule == nullptr) {
    error("Expect expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  (this->*prefixRule)(canAssign);

  while (precedence <= getRule(parser.current.type).precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type).infix;
    (this->*infixRule)(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }
}

uint8_t Emitter::identifierConstant(Token &name) {
  return makeConstant(Value::Object(vm.copyString(name.str)));
}

static bool identifiersEqual(Token &a, Token &b) { return a.str == b.str; }

int Emitter::resolveLocal(Token &name) {
  for (int i = compiler.localCount - 1; i >= 0; i--) {
    Local &local = compiler.locals[i];
    if (identifiersEqual(name, local.name)) {
      if (local.depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }

  return -1;
}

void Emitter::addLocal(Token name) {
  if (compiler.localCount == UINT8_COUNT) {
    error("Too many local variables in function.");
    return;
  }

  Local &local = compiler.locals[compiler.localCount++];
  local.name = name;
  local.depth = -1;
}

void Emitter::declareVariable() {
  if (compiler.scopeDepth == 0)
    return;

  Token &name = parser.previous;
  for (int i = compiler.localCount - 1; i >= 0; i--) {
    Local &local = compiler.locals[i];
    if (local.depth != -1 && local.depth < compiler.scopeDepth) {
      break;
    }

    if (identifiersEqual(name, local.name)) {
      error("Already a variable with this name in this scope.");
    }
  }

  addLocal(name);
}

uint8_t Emitter::parseVariable(std::string_view errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);

  declareVariable();
  if (compiler.scopeDepth > 0)
    return 0;

  return identifierConstant(parser.previous);
}

void Emitter::markInitialized() {
  compiler.locals[compiler.localCount - 1].depth = compiler.scopeDepth;
}

void Emitter::defineVariable(uint8_t global) {
  if (compiler.scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_GLOBAL, global);
}

}; // namespace clox
