#ifndef PARSER_HPP
#define PARSER_HPP

#include "ast.hpp"
#include "lexico.hpp"
#include <memory>
#include <cstring>
#include <vector>

class Parser {
private:
  std::vector<Token> tokens; // Lista de tokens gerada pelo scanner
  size_t current;            // Posição do token atual sendo analisado

  // --- Métodos Auxiliares de Navegação ---

  // Retorna o token atual sem consumi-lo
  Token peek() const;

  // Retorna o token anterior
  Token previous() const;

  // Verifica se chegamos ao fim dos tokens
  bool isAtEnd() const;

  // Avança para o próximo token e o devolve
  Token advance();

  // Verifica se o token atual tem o tipo esperado; consome ou lança erro
  Token match(TokenType type, const std::string &errorMessage);

  // Lança um erro sintático formatado com a linha do token
  void error(const Token &token, const std::string &message);

public:
  // Construtor recebe a lista de tokens do scanner
  Parser(const std::vector<Token> &tokenList);

  // --- Métodos de Parsing ---
  std::unique_ptr<ProgramNode> parseProgram();
  std::unique_ptr<StatementNode> parseStatement();
  std::unique_ptr<DeclarationNode> parseDeclaration();
  std::unique_ptr<AssignmentNode> parseAssignment();
  std::unique_ptr<PrintNode> parsePrint();
  std::unique_ptr<IfNode> parseIf();
  std::unique_ptr<WhileNode> parseWhile();

  // Expressão – nível de precedência
  std::unique_ptr<ExpressionNode> parseExpression();
  std::unique_ptr<ExpressionNode> parseEquality();
  std::unique_ptr<ExpressionNode> parseComparison();
  std::unique_ptr<ExpressionNode> parseTerm();
  std::unique_ptr<ExpressionNode> parseFactor();
  std::unique_ptr<ExpressionNode> parseUnary();
  std::unique_ptr<ExpressionNode> parsePrimary();
};

#endif // PARSER_HPP
