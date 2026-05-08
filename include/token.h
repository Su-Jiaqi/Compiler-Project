#ifndef TOKEN_H
#define TOKEN_H
#include <string>
#include <vector>
#include <sstream>

enum class TokenType {
    ID, NUM, FLO,
    INT, FLOAT, VOID, IF, ELSE, WHILE, RETURN, INPUT, PRINT,
    ADD, SUB, MUL, DIV,
    LT, LE, EQ, GT, GE, NE,
    ASG, AAS, AAA,
    AND, OR, NOT,
    LPA, RPA, LBK, RBK, LBR, RBR, CMA, SCO,
    END, UNKNOWN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;
};

inline std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::ID: return "ID";
        case TokenType::NUM: return "NUM";
        case TokenType::FLO: return "FLO";
        case TokenType::INT: return "INT";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::VOID: return "VOID";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::INPUT: return "INPUT";
        case TokenType::PRINT: return "PRINT";
        case TokenType::ADD: return "ADD";
        case TokenType::SUB: return "SUB";
        case TokenType::MUL: return "MUL";
        case TokenType::DIV: return "DIV";
        case TokenType::LT: return "LT";
        case TokenType::LE: return "LE";
        case TokenType::EQ: return "EQ";
        case TokenType::GT: return "GT";
        case TokenType::GE: return "GE";
        case TokenType::NE: return "NE";
        case TokenType::ASG: return "ASG";
        case TokenType::AAS: return "AAS";
        case TokenType::AAA: return "AAA";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::LPA: return "LPA";
        case TokenType::RPA: return "RPA";
        case TokenType::LBK: return "LBK";
        case TokenType::RBK: return "RBK";
        case TokenType::LBR: return "LBR";
        case TokenType::RBR: return "RBR";
        case TokenType::CMA: return "CMA";
        case TokenType::SCO: return "SCO";
        case TokenType::END: return "$";
        default: return "UNKNOWN";
    }
}

inline std::string tokenToString(const Token& tk) {
    std::ostringstream oss;
    oss << tokenTypeName(tk.type) << "\t" << tk.lexeme << "\t(" << tk.line << "," << tk.col << ")";
    return oss.str();
}
#endif
