#include "semantic_ir.h"
#include <fstream>
#include <sstream>

const Token& SemanticIRGenerator::peek(int k) const {
    static Token eof{TokenType::END,"$",0,0};
    return pos+k < toks.size() ? toks[pos+k] : eof;
}
bool SemanticIRGenerator::check(TokenType t) const { return peek().type == t; }
bool SemanticIRGenerator::match(TokenType t) { if (check(t)) { pos++; return true; } return false; }
Token SemanticIRGenerator::consume(TokenType t, const std::string& msg) {
    if (check(t)) return toks[pos++];
    errs.push_back("line " + std::to_string(peek().line) + ": expected " + msg + ", got " + tokenTypeName(peek().type));
    return {t,"",peek().line,peek().col};
}
std::string SemanticIRGenerator::newTemp() { return "t" + std::to_string(++tempId); }
std::string SemanticIRGenerator::newLabel() { return "L" + std::to_string(++labelId); }
void SemanticIRGenerator::addAst(const std::string& s) { ast.push_back(s); }

bool SemanticIRGenerator::analyze(const std::vector<Token>& tokens) {
    toks = tokens; pos = 0; tempId = labelId = offset = 0; symtab.clear(); quads.clear(); ast.clear(); errs.clear();
    parseProgram();
    return errs.empty();
}

void SemanticIRGenerator::parseProgram() {
    addAst("Program");
    while (!check(TokenType::END)) parseStatement();
}

void SemanticIRGenerator::parseBlock() {
    consume(TokenType::LBR, "{");
    addAst("Block");
    while (!check(TokenType::RBR) && !check(TokenType::END)) parseStatement();
    consume(TokenType::RBR, "}");
}

void SemanticIRGenerator::parseStatement() {
    if (match(TokenType::INT)) { parseDeclaration("int"); return; }
    if (match(TokenType::FLOAT)) { parseDeclaration("float"); return; }
    if (check(TokenType::ID)) { parseAssignmentOrCall(); return; }
    if (check(TokenType::IF)) { parseIf(); return; }
    if (check(TokenType::WHILE)) { parseWhile(); return; }
    if (match(TokenType::PRINT)) {
        std::string v = parseExpression();
        quads.push_back({"print", v, "", ""});
        consume(TokenType::SCO, ";");
        addAst("PrintStmt"); return;
    }
    if (match(TokenType::INPUT)) {
        Token id = consume(TokenType::ID, "identifier after input");
        if (!symtab.count(id.lexeme)) errs.push_back("undeclared variable: " + id.lexeme);
        quads.push_back({"input", "", "", id.lexeme});
        consume(TokenType::SCO, ";");
        addAst("InputStmt"); return;
    }
    if (match(TokenType::RETURN)) {
        std::string v = parseExpression();
        quads.push_back({"return", v, "", ""});
        consume(TokenType::SCO, ";");
        addAst("ReturnStmt"); return;
    }
    if (check(TokenType::LBR)) { parseBlock(); return; }
    errs.push_back("line " + std::to_string(peek().line) + ": unsupported statement near " + peek().lexeme);
    pos++;
}

void SemanticIRGenerator::parseDeclaration(const std::string& type) {
    Token id = consume(TokenType::ID, "identifier");
    if (symtab.count(id.lexeme)) errs.push_back("duplicate declaration: " + id.lexeme);
    else { symtab[id.lexeme] = {id.lexeme, type, offset}; offset += (type == "float" ? 8 : 4); }
    addAst("Decl(" + type + " " + id.lexeme + ")");
    if (match(TokenType::ASG)) {
        std::string v = parseExpression();
        quads.push_back({"=", v, "", id.lexeme});
    }
    while (match(TokenType::CMA)) {
        Token id2 = consume(TokenType::ID, "identifier");
        if (symtab.count(id2.lexeme)) errs.push_back("duplicate declaration: " + id2.lexeme);
        else { symtab[id2.lexeme] = {id2.lexeme, type, offset}; offset += (type == "float" ? 8 : 4); }
        addAst("Decl(" + type + " " + id2.lexeme + ")");
    }
    consume(TokenType::SCO, ";");
}

void SemanticIRGenerator::parseAssignmentOrCall() {
    Token id = consume(TokenType::ID, "identifier");
    if (match(TokenType::ASG)) {
        if (!symtab.count(id.lexeme)) errs.push_back("undeclared variable: " + id.lexeme);
        std::string v = parseExpression();
        quads.push_back({"=", v, "", id.lexeme});
        consume(TokenType::SCO, ";");
        addAst("Assign(" + id.lexeme + ")");
        return;
    }
    if (match(TokenType::AAS)) {
        if (!symtab.count(id.lexeme)) errs.push_back("undeclared variable: " + id.lexeme);
        std::string v = parseExpression();
        std::string t = newTemp();
        quads.push_back({"+", id.lexeme, v, t});
        quads.push_back({"=", t, "", id.lexeme});
        consume(TokenType::SCO, ";");
        return;
    }
    if (match(TokenType::AAA)) {
        if (!symtab.count(id.lexeme)) errs.push_back("undeclared variable: " + id.lexeme);
        std::string t = newTemp();
        quads.push_back({"+", id.lexeme, "1", t});
        quads.push_back({"=", t, "", id.lexeme});
        consume(TokenType::SCO, ";");
        return;
    }
    if (match(TokenType::LPA)) {
        int argc = 0;
        if (!check(TokenType::RPA)) { parseExpression(); argc++; while (match(TokenType::CMA)) { parseExpression(); argc++; } }
        consume(TokenType::RPA, ")"); consume(TokenType::SCO, ";");
        quads.push_back({"call", id.lexeme, std::to_string(argc), ""});
        addAst("CallStmt(" + id.lexeme + ")");
        return;
    }
    errs.push_back("line " + std::to_string(id.line) + ": expected assignment or function call");
}

void SemanticIRGenerator::parseIf() {
    consume(TokenType::IF, "if"); consume(TokenType::LPA, "(");
    std::string cond = parseExpression(); consume(TokenType::RPA, ")");
    std::string Lelse = newLabel(), Lend = newLabel();
    quads.push_back({"jz", cond, "", Lelse});
    parseStatement();
    if (match(TokenType::ELSE)) {
        quads.push_back({"j", "", "", Lend});
        quads.push_back({"label", "", "", Lelse});
        parseStatement();
        quads.push_back({"label", "", "", Lend});
    } else quads.push_back({"label", "", "", Lelse});
    addAst("IfStmt");
}

void SemanticIRGenerator::parseWhile() {
    consume(TokenType::WHILE, "while");
    std::string Lbegin = newLabel(), Lend = newLabel();
    quads.push_back({"label", "", "", Lbegin});
    consume(TokenType::LPA, "("); std::string cond = parseExpression(); consume(TokenType::RPA, ")");
    quads.push_back({"jz", cond, "", Lend});
    parseStatement();
    quads.push_back({"j", "", "", Lbegin});
    quads.push_back({"label", "", "", Lend});
    addAst("WhileStmt");
}

std::string SemanticIRGenerator::parseExpression() { return parseEquality(); }
std::string SemanticIRGenerator::parseEquality() {
    std::string left = parseRelational();
    while (check(TokenType::EQ) || check(TokenType::NE)) {
        std::string op = tokenTypeName(peek().type); pos++;
        std::string right = parseRelational(); std::string t = newTemp();
        quads.push_back({op == "EQ" ? "==" : "!=", left, right, t}); left = t;
    }
    return left;
}
std::string SemanticIRGenerator::parseRelational() {
    std::string left = parseAdditive();
    while (check(TokenType::LT) || check(TokenType::LE) || check(TokenType::GT) || check(TokenType::GE)) {
        TokenType tt = peek().type; pos++;
        std::string right = parseAdditive(); std::string t = newTemp();
        std::string op = tt==TokenType::LT?"<":tt==TokenType::LE?"<=":tt==TokenType::GT?">":">=";
        quads.push_back({op, left, right, t}); left = t;
    }
    return left;
}
std::string SemanticIRGenerator::parseAdditive() {
    std::string left = parseTerm();
    while (check(TokenType::ADD) || check(TokenType::SUB)) {
        std::string op = check(TokenType::ADD) ? "+" : "-"; pos++;
        std::string right = parseTerm(); std::string t = newTemp();
        quads.push_back({op, left, right, t}); left = t;
    }
    return left;
}
std::string SemanticIRGenerator::parseTerm() {
    std::string left = parseFactor();
    while (check(TokenType::MUL) || check(TokenType::DIV)) {
        std::string op = check(TokenType::MUL) ? "*" : "/"; pos++;
        std::string right = parseFactor(); std::string t = newTemp();
        quads.push_back({op, left, right, t}); left = t;
    }
    return left;
}
std::string SemanticIRGenerator::parseFactor() {
    if (match(TokenType::LPA)) { std::string v = parseExpression(); consume(TokenType::RPA, ")"); return v; }
    if (check(TokenType::NUM) || check(TokenType::FLO)) return toks[pos++].lexeme;
    if (check(TokenType::ID)) {
        Token id = toks[pos++];
        if (match(TokenType::LPA)) {
            int argc = 0;
            if (!check(TokenType::RPA)) { parseExpression(); argc++; while (match(TokenType::CMA)) { parseExpression(); argc++; } }
            consume(TokenType::RPA, ")"); std::string t = newTemp();
            quads.push_back({"call", id.lexeme, std::to_string(argc), t}); return t;
        }
        if (!symtab.count(id.lexeme)) errs.push_back("undeclared variable: " + id.lexeme);
        return id.lexeme;
    }
    if (match(TokenType::SUB)) {
        std::string v = parseFactor(); std::string t = newTemp(); quads.push_back({"uminus", v, "", t}); return t;
    }
    errs.push_back("line " + std::to_string(peek().line) + ": bad expression factor near " + peek().lexeme);
    pos++; return "?";
}

void SemanticIRGenerator::writeSymbols(const std::string& path) const {
    std::ofstream f(path);
    f << "name\ttype\toffset\n";
    for (auto& kv: symtab) f << kv.second.name << "\t" << kv.second.type << "\t" << kv.second.offset << "\n";
    if (!errs.empty()) { f << "\nErrors:\n"; for (auto& e: errs) f << e << "\n"; }
}
void SemanticIRGenerator::writeIR(const std::string& path) const {
    std::ofstream f(path);
    int i=0; for (auto& q: quads) f << i++ << ": (" << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")\n";
    if (!errs.empty()) { f << "\nErrors:\n"; for (auto& e: errs) f << e << "\n"; }
}
void SemanticIRGenerator::writeAST(const std::string& path) const {
    std::ofstream f(path);
    for (auto& s: ast) f << s << "\n";
}
