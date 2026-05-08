#ifndef SEMANTIC_IR_H
#define SEMANTIC_IR_H
#include "token.h"
#include <map>
#include <string>
#include <vector>

struct Symbol {
    std::string name;
    std::string type;
    int offset;
};

struct Quad {
    std::string op, arg1, arg2, result;
};

class SemanticIRGenerator {
public:
    bool analyze(const std::vector<Token>& tokens);
    void writeSymbols(const std::string& path) const;
    void writeIR(const std::string& path) const;
    void writeAST(const std::string& path) const;
    const std::vector<std::string>& errors() const { return errs; }
private:
    std::vector<Token> toks;
    size_t pos = 0;
    int tempId = 0;
    int labelId = 0;
    int offset = 0;
    std::map<std::string, Symbol> symtab;
    std::vector<Quad> quads;
    std::vector<std::string> ast;
    std::vector<std::string> errs;

    const Token& peek(int k = 0) const;
    bool check(TokenType t) const;
    bool match(TokenType t);
    Token consume(TokenType t, const std::string& msg);
    std::string newTemp();
    std::string newLabel();
    void parseProgram();
    void parseBlock();
    void parseStatement();
    void parseDeclaration(const std::string& type);
    void parseAssignmentOrCall();
    void parseIf();
    void parseWhile();
    std::string parseExpression();
    std::string parseEquality();
    std::string parseRelational();
    std::string parseAdditive();
    std::string parseTerm();
    std::string parseFactor();
    void addAst(const std::string& s);
};
#endif
