#ifndef AST_HPP
#define AST_HPP

#include <string>

// A raiz de qualquer elemento que exista na nossa árvore sintática.
class ASTNode {
public:
    // Destrutor virtual para garantir que ao deletar um nó, as subclasses também sejam limpas da memória corretamente.
    virtual ~ASTNode() = default;
    
    // Método que cada nó concreto terá que implementar para imprimir a si mesmo como JSON.
    // O parâmetro 'indent' serve para formatar o JSON de forma legível (com espaços).
    virtual std::string toJSON(int indent = 0) const = 0;
    
protected:
    // Função auxiliar que gera espaços em branco de recuo (indentação)
    std::string getIndent(int indent) const {
        return std::string(indent * 2, ' ');
    }
};

#endif // AST_HPP
