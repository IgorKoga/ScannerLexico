#include "lexico.hpp"
#include <iomanip>
#include <iostream>
#include <string>

#ifndef LEXICO_ONLY
#include "ast.hpp"
#include "parser.hpp"
#endif

#include <fstream> // lê arquivos
#include <sstream> // lê o arquivo como string
using namespace std;

int main(int argc, char *argv[]) {
  // Verifica se o executável foi chamado como "lexico" ou "sintatico"
  string execName = argv[0];
  bool isLexicoOnly = (execName.find("lexico") != string::npos);
  bool isSintaticoOnly = (execName.find("sintatico") != string::npos);

#ifdef LEXICO_ONLY
  isLexicoOnly = true;
#endif

  // Se não for especificamente nenhum (ex: compilador.exe ou main.exe), executa ambos
  bool runBoth = !isLexicoOnly && !isSintaticoOnly;

  // 1. Nome do arquivo fixo
  string folder = "codigoRust/"; // endereço padrão
  string fileName;
  cout << "Digite o nome/endereco do arquivo: ";
  cin >> fileName;
  fileName =
      folder + fileName; // concatena o endereço padrão com o nome do arquivo

  // 2. Tenta abrir o arquivo
  ifstream file(fileName);
  if (!file.is_open()) {
    cerr << "Erro: O arquivo '" << fileName << "' nao foi encontrado na pasta!"
         << endl;
    return 1;
  }

  // 3. Lê o conteúdo
  stringstream buffer;
  buffer << file.rdbuf();
  string code = buffer.str();

  // Cria o scanner para a análise léxica (tabela de tokens)
  Scanner lexScanner(code);

  try {
    if (isLexicoOnly || runBoth) {
      if (runBoth) {
        cout << "\n============================================================" << endl;
        cout << "                  1. FASE LEXICA (TOKENS)                   " << endl;
        cout << "============================================================" << endl;
      } else {
        cout << endl;
      }
      
      // Cabeçalho da tabela
      cout << string(60, '-') << endl;
      cout << left << setw(20) << "Token" << setw(30) << "Lexema" << "Linha"
           << endl;
      cout << string(60, '-') << endl;

      // Continua analisando até encontrar o fim da entrada para exibir a tabela
      Token token = lexScanner.nextToken();
      while (token.type != TokenType::T_EOF) {
        // Exibe os dados formatados em colunas
        cout << left << setw(20) << tokenTypeToString(token.type) << setw(30)
             << token.lexeme << token.line << endl;

        // Busca o próximo token
        token = lexScanner.nextToken();
      }

      cout << string(60, '-') << endl;
      cout << "Fim da analise lexica." << endl;
    }

#ifndef LEXICO_ONLY
    if (isSintaticoOnly || runBoth) {
      if (runBoth) {
        cout << "\n============================================================" << endl;
        cout << "                  2. FASE SINTATICA (AST JSON)              " << endl;
        cout << "============================================================" << endl;
      }

      // Cria um scanner separado para o parser
      Scanner parserScanner(code);
      Parser parser(parserScanner);
      auto ast = parser.parseProgram();

      cout << ast->toJson() << endl;

      if (runBoth) {
        cout << "============================================================" << endl;
      }
      cout << "Fim da analise sintatica." << endl;
    }
#endif

  } catch (exception &e) {
    // Caso ocorra erro, a mensagem será exibida aqui
    cerr << e.what() << endl;
  }

  return 0;
}
