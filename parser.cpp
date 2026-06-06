#include "parser.hpp"
#include "lexico.hpp"
#include <sstream>
#include <stdexcept>

// Recebe a lista de tokens e inicia o cursor
Parser::Parser(const std::vector<Token> &tokenList)
    : tokens(tokenList), current(0) {}

// ------------------- Métodos auxiliares de navegação -------------------

// Retorna o token atual sem avançar.
Token Parser::peek() const {
  // Garantimos que não estamos além do fim; se estivermos, retornamos um
  // token EOF fictício (geralmente definido no lexer).
  if (isAtEnd())
    return Token{TokenType::T_EOF, "", -1};
  return tokens[current];
}

// Retorna o token já consumido anteriormente.
Token Parser::previous() const {
  if (current == 0)
    throw std::runtime_error(
        "Tentativa de acessar token anterior antes do início.");
  return tokens[current - 1];
}

// Verifica se já consumimos todos os tokens.
bool Parser::isAtEnd() const {
  return current >= tokens.size() || tokens[current].type == TokenType::T_EOF;
}

// Avança o cursor e devolve o token consumido.
Token Parser::advance() {
  if (!isAtEnd())
    ++current;
  return previous(); // token que acabou de ser consumido
}

// Se o token atual tem o tipo esperado, consome‑o; caso contrário, lança erro.
Token Parser::match(TokenType type, const std::string &errorMessage) {
  if (isAtEnd())
    error(peek(), "Fim inesperado da entrada. " + errorMessage);
  if (peek().type == type)
    return advance();
  error(peek(), "Token inesperado: esperado '" + tokenTypeToString(type) +
                    "'. " + errorMessage);
  // Nunca chega aqui, mas retornamos um token de fallback para satisfazer o
  // compilador.
  return Token{TokenType::T_EOF, "", -1};
}

// Cria uma exceção com a linha do token e mensagem informativa.
void Parser::error(const Token &token, const std::string &message) {
  std::ostringstream oss;
  oss << "Erro sintático na linha " << token.line << ": " << message;
  throw std::runtime_error(oss.str());
}

// tokenTypeToString is defined in lexico.cpp; we use the declaration from
// lexico.hpp

// -----------------------------------------------------------------------------
// Métodos de Parsing
// -----------------------------------------------------------------------------

// Programa inteiro → lista de declarações/expressões
std::unique_ptr<ProgramNode> Parser::parseProgram() {
    std::vector<std::unique_ptr<StatementNode>> statements;
    while (!isAtEnd()) {
        statements.push_back(parseStatement());
    }
    return std::make_unique<ProgramNode>(std::move(statements));
}

// Declaração ou comando (Statement)
std::unique_ptr<StatementNode> Parser::parseStatement() {
    Token tk = peek();
    switch (tk.type) {
        case TokenType::T_LET:
        case TokenType::T_MUT:
        case TokenType::T_INT:
            return parseDeclaration();
        case TokenType::T_ID:
            // Could be assignment
            return parseAssignment();
        case TokenType::T_PRINTLN:
            return parsePrint();
        case TokenType::T_IF:
            return parseIf();
        case TokenType::T_WHILE:
            return parseWhile();
        default:
            error(tk, "Comando ou declaração inesperado.");
            return nullptr; // Unreachable
    }
}

// Declaração de variável (let, mut, int …)
std::unique_ptr<DeclarationNode> Parser::parseDeclaration() {
    // Tipo da variável
    Token typeTok = advance(); // let, mut, ou int
    std::string varType;
    if (typeTok.type == TokenType::T_LET) varType = "let";
    else if (typeTok.type == TokenType::T_MUT) varType = "mut";
    else varType = "int";
    // Identificador
    Token idTok = match(TokenType::T_ID, "Esperado identificador após tipo de variável.");
    std::string identifier = idTok.lexeme;
    // Opcional inicialização
    std::unique_ptr<ExpressionNode> init = nullptr;
    if (peek().type == TokenType::T_ASSIGN) {
        match(TokenType::T_ASSIGN, "Esperado '=' após identificador.");
        init = parseExpression();
    }
    // Consome ponto e vírgula se presente
    if (peek().type == TokenType::T_SEMICOLON) advance();
    return std::make_unique<DeclarationNode>(varType, identifier, std::move(init));
}

// Atribuição (identificador = expressão)
std::unique_ptr<AssignmentNode> Parser::parseAssignment() {
    Token idTok = match(TokenType::T_ID, "Esperado identificador no início da atribuição.");
    match(TokenType::T_ASSIGN, "Esperado '=' na atribuição.");
    auto expr = parseExpression();
    if (peek().type == TokenType::T_SEMICOLON) advance();
    return std::make_unique<AssignmentNode>(idTok.lexeme, std::move(expr));
}

// Comando de impressão (println! …)
std::unique_ptr<PrintNode> Parser::parsePrint() {
    match(TokenType::T_PRINTLN, "Esperado 'println!'.");
    match(TokenType::T_LPAREN, "Esperado '(' após println!.");
    auto expr = parseExpression();
    match(TokenType::T_RPAREN, "Esperado ')' após expressão.");
    if (peek().type == TokenType::T_SEMICOLON) advance();
    return std::make_unique<PrintNode>(std::move(expr));
}

// Estrutura condicional
std::unique_ptr<IfNode> Parser::parseIf() {
    match(TokenType::T_IF, "Esperado 'if'.");
    match(TokenType::T_LPAREN, "Esperado '(' após if.");
    auto condition = parseExpression();
    match(TokenType::T_RPAREN, "Esperado ')' após condição.");
    // Bloco then
    match(TokenType::T_LBRACE, "Esperado '{' para bloco then.");
    std::vector<std::unique_ptr<StatementNode>> thenBranch;
    while (peek().type != TokenType::T_RBRACE && !isAtEnd()) {
        thenBranch.push_back(parseStatement());
    }
    match(TokenType::T_RBRACE, "Esperado '}' ao final do bloco then.");
    // Opcional else
    std::vector<std::unique_ptr<StatementNode>> elseBranch;
    if (peek().type == TokenType::T_ELSE) {
        advance(); // consume else
        match(TokenType::T_LBRACE, "Esperado '{' para bloco else.");
        while (peek().type != TokenType::T_RBRACE && !isAtEnd()) {
            elseBranch.push_back(parseStatement());
        }
        match(TokenType::T_RBRACE, "Esperado '}' ao final do bloco else.");
    }
    return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

// Laço while
std::unique_ptr<WhileNode> Parser::parseWhile() {
    match(TokenType::T_WHILE, "Esperado 'while'.");
    match(TokenType::T_LPAREN, "Esperado '(' após while.");
    auto condition = parseExpression();
    match(TokenType::T_RPAREN, "Esperado ')' após condição.");
    match(TokenType::T_LBRACE, "Esperado '{' para bloco while.");
    std::vector<std::unique_ptr<StatementNode>> body;
    while (peek().type != TokenType::T_RBRACE && !isAtEnd()) {
        body.push_back(parseStatement());
    }
    match(TokenType::T_RBRACE, "Esperado '}' ao final do bloco while.");
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

// ------------------- Expression Parsing -------------------

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    return parseEquality();
}

std::unique_ptr<ExpressionNode> Parser::parseEquality() {
    auto expr = parseComparison();
    while (peek().type == TokenType::T_EQ) {
        Token op = advance();
        auto right = parseComparison();
        expr = std::make_unique<BinaryOpNode>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseComparison() {
    auto expr = parseTerm();
    while (peek().type == TokenType::T_LT || peek().type == TokenType::T_GT) {
        Token op = advance();
        auto right = parseTerm();
        expr = std::make_unique<BinaryOpNode>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseTerm() {
    auto expr = parseFactor();
    while (peek().type == TokenType::T_PLUS || peek().type == TokenType::T_MINUS) {
        Token op = advance();
        auto right = parseFactor();
        expr = std::make_unique<BinaryOpNode>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseFactor() {
    auto expr = parseUnary();
    while (peek().type == TokenType::T_MULT || peek().type == TokenType::T_DIV) {
        Token op = advance();
        auto right = parseUnary();
        expr = std::make_unique<BinaryOpNode>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseUnary() {
    if (peek().type == TokenType::T_MINUS || peek().type == TokenType::T_EXCL) {
        Token op = advance();
        auto right = parseUnary();
        return std::make_unique<BinaryOpNode>(op.lexeme, nullptr, std::move(right));
    }
    return parsePrimary();
}

std::unique_ptr<ExpressionNode> Parser::parsePrimary() {
    Token tk = advance();
    switch (tk.type) {
        case TokenType::T_NUM:
        case TokenType::T_FLOAT:
        case TokenType::T_STRING:
            return std::make_unique<NumberNode>(tk.lexeme);
        case TokenType::T_ID:
            return std::make_unique<VariableNode>(tk.lexeme);
        case TokenType::T_LPAREN: {
            auto expr = parseExpression();
            match(TokenType::T_RPAREN, "Esperado ')' após expressão.");
            return expr;
        }
        default:
            error(tk, "Token inesperado em expressão primária.");
            return nullptr; // Unreachable
    }
}
