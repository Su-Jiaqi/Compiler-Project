#ifndef LEXER_H
#define LEXER_H
#include "token.h"
#include <string>
#include <vector>

class Lexer {
public:
    std::vector<Token> scan(const std::string& source);
    static std::string readFile(const std::string& path);
    static void writeTokens(const std::vector<Token>& tokens, const std::string& path);
private:
    size_t i = 0;
    int line = 1, col = 1;
    std::string src;
    char peek(int k = 0) const;
    char advance();
    bool match(char c);
    void skipWhitespaceAndComments();
    Token make(TokenType type, const std::string& lexeme, int l, int c) const;
    Token identifier();
    Token number();
};
#endif
