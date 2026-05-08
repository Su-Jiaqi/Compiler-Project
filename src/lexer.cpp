#include "lexer.h"
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

std::string Lexer::readFile(const std::string& path) {
    std::ifstream fin(path);
    if (!fin) throw std::runtime_error("cannot open file: " + path);
    std::ostringstream ss; ss << fin.rdbuf(); return ss.str();
}

void Lexer::writeTokens(const std::vector<Token>& tokens, const std::string& path) {
    std::ofstream fout(path);
    for (auto& tk: tokens) if (tk.type != TokenType::END) fout << tokenToString(tk) << "\n";
}

char Lexer::peek(int k) const { return i + k < src.size() ? src[i+k] : '\0'; }
char Lexer::advance() {
    char c = peek();
    if (c == '\0') return c;
    i++;
    if (c == '\n') { line++; col = 1; } else col++;
    return c;
}
bool Lexer::match(char c) { if (peek() == c) { advance(); return true; } return false; }
Token Lexer::make(TokenType type, const std::string& lexeme, int l, int c) const { return {type, lexeme, l, c}; }

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        while (std::isspace((unsigned char)peek())) advance();
        if (peek() == '/' && peek(1) == '/') {
            while (peek() && peek() != '\n') advance();
        } else if (peek() == '/' && peek(1) == '*') {
            advance(); advance();
            while (peek() && !(peek() == '*' && peek(1) == '/')) advance();
            if (peek()) { advance(); advance(); }
        } else break;
    }
}

Token Lexer::identifier() {
    int l = line, c = col;
    std::string s;
    while (std::isalnum((unsigned char)peek()) || peek() == '_') s += advance();
    static std::map<std::string, TokenType> kw = {
        {"int",TokenType::INT}, {"float",TokenType::FLOAT}, {"void",TokenType::VOID},
        {"if",TokenType::IF}, {"else",TokenType::ELSE}, {"while",TokenType::WHILE},
        {"return",TokenType::RETURN}, {"input",TokenType::INPUT}, {"print",TokenType::PRINT}
    };
    auto it = kw.find(s);
    return make(it == kw.end() ? TokenType::ID : it->second, s, l, c);
}

Token Lexer::number() {
    int l = line, c = col;
    std::string s;
    bool isFloat = false;
    while (std::isdigit((unsigned char)peek())) s += advance();
    if (peek() == '.') {
        isFloat = true; s += advance();
        while (std::isdigit((unsigned char)peek())) s += advance();
    }
    if (peek() == 'e' || peek() == 'E') {
        isFloat = true; s += advance();
        if (peek() == '+' || peek() == '-') s += advance();
        while (std::isdigit((unsigned char)peek())) s += advance();
    }
    return make(isFloat ? TokenType::FLO : TokenType::NUM, s, l, c);
}

std::vector<Token> Lexer::scan(const std::string& source) {
    src = source; i = 0; line = 1; col = 1;
    std::vector<Token> out;
    while (peek()) {
        skipWhitespaceAndComments();
        if (!peek()) break;
        int l = line, c = col;
        char ch = peek();
        if (std::isalpha((unsigned char)ch) || ch == '_') { out.push_back(identifier()); continue; }
        if (std::isdigit((unsigned char)ch)) { out.push_back(number()); continue; }
        if (ch == '.' && std::isdigit((unsigned char)peek(1))) { out.push_back(number()); continue; }
        switch (advance()) {
            case '+': if (match('+')) out.push_back(make(TokenType::AAA,"++",l,c)); else if (match('=')) out.push_back(make(TokenType::AAS,"+=",l,c)); else out.push_back(make(TokenType::ADD,"+",l,c)); break;
            case '-': out.push_back(make(TokenType::SUB,"-",l,c)); break;
            case '*': out.push_back(make(TokenType::MUL,"*",l,c)); break;
            case '/': out.push_back(make(TokenType::DIV,"/",l,c)); break;
            case '<': if (match('=')) out.push_back(make(TokenType::LE,"<=",l,c)); else out.push_back(make(TokenType::LT,"<",l,c)); break;
            case '>': if (match('=')) out.push_back(make(TokenType::GE,">=",l,c)); else out.push_back(make(TokenType::GT,">",l,c)); break;
            case '=': if (match('=')) out.push_back(make(TokenType::EQ,"==",l,c)); else out.push_back(make(TokenType::ASG,"=",l,c)); break;
            case '!': if (match('=')) out.push_back(make(TokenType::NE,"!=",l,c)); else out.push_back(make(TokenType::NOT,"!",l,c)); break;
            case '&': if (match('&')) out.push_back(make(TokenType::AND,"&&",l,c)); else out.push_back(make(TokenType::UNKNOWN,"&",l,c)); break;
            case '|': if (match('|')) out.push_back(make(TokenType::OR,"||",l,c)); else out.push_back(make(TokenType::UNKNOWN,"|",l,c)); break;
            case '(': out.push_back(make(TokenType::LPA,"(",l,c)); break;
            case ')': out.push_back(make(TokenType::RPA,")",l,c)); break;
            case '[': out.push_back(make(TokenType::LBK,"[",l,c)); break;
            case ']': out.push_back(make(TokenType::RBK,"]",l,c)); break;
            case '{': out.push_back(make(TokenType::LBR,"{",l,c)); break;
            case '}': out.push_back(make(TokenType::RBR,"}",l,c)); break;
            case ',': out.push_back(make(TokenType::CMA,",",l,c)); break;
            case ';': out.push_back(make(TokenType::SCO,";",l,c)); break;
            case '.': out.push_back(make(TokenType::UNKNOWN,".",l,c)); break;
            default: out.push_back(make(TokenType::UNKNOWN,std::string(1,ch),l,c)); break;
        }
    }
    out.push_back(make(TokenType::END, "$", line, col));
    return out;
}
