#include "dfa.h"
#include "lexer.h"
#include "grammar.h"
#include "semantic_ir.h"
#include <iostream>
#include <filesystem>

static void usage() {
    std::cout << "Usage:\n"
              << "  ./compiler dfa <dfa_file> <max_len> [test_string]\n"
              << "  ./compiler lexer <src_file> <out_tokens>\n"
              << "  ./compiler lr0 <grammar_file> <out_items>\n"
              << "  ./compiler slr <grammar_file> <out_table>\n"
              << "  ./compiler ir <src_file> <out_dir>\n"
              << "  ./compiler all <src_file> <grammar_file> <out_dir>\n";
}

int main(int argc, char** argv) {
    if (argc < 2) { usage(); return 1; }
    std::string mode = argv[1];
    try {
        if (mode == "dfa") {
            if (argc < 4) { usage(); return 1; }
            DFA dfa;
            if (!dfa.loadFromFile(argv[2])) { std::cerr << "cannot load dfa\n"; return 1; }
            std::string err;
            if (!dfa.validate(&err)) { std::cerr << "DFA invalid: " << err << "\n"; return 1; }
            dfa.print();
            int maxLen = std::stoi(argv[3]);
            auto acc = dfa.generateAccepted(maxLen);
            std::cout << "Accepted strings length <= " << maxLen << ":\n";
            for (auto& s: acc) std::cout << s << "\n";
            if (argc >= 5) {
                std::vector<std::string> trace;
                bool ok = dfa.accepts(argv[4], &trace);
                std::cout << "Trace for " << argv[4] << ":\n";
                for (auto& t: trace) std::cout << t << "\n";
                std::cout << (ok ? "ACCEPT\n" : "REJECT\n");
            }
        } else if (mode == "lexer") {
            if (argc < 4) { usage(); return 1; }
            Lexer lx;
            auto tokens = lx.scan(Lexer::readFile(argv[2]));
            Lexer::writeTokens(tokens, argv[3]);
            for (auto& t: tokens) if (t.type != TokenType::END) std::cout << tokenToString(t) << "\n";
        } else if (mode == "lr0" || mode == "slr") {
            if (argc < 4) { usage(); return 1; }
            Grammar g;
            if (!g.loadFromFile(argv[2])) { std::cerr << "cannot load grammar\n"; return 1; }
            g.augment();
            LR0Builder b(g); b.build();
            if (mode == "lr0") b.writeItems(argv[3]);
            else b.writeSLRTable(argv[3]);
            std::cout << "written to " << argv[3] << "\n";
        } else if (mode == "ir") {
            if (argc < 4) { usage(); return 1; }
            std::filesystem::create_directories(argv[3]);
            Lexer lx; auto tokens = lx.scan(Lexer::readFile(argv[2]));
            Lexer::writeTokens(tokens, std::string(argv[3]) + "/tokens.txt");
            SemanticIRGenerator gen; bool ok = gen.analyze(tokens);
            gen.writeAST(std::string(argv[3]) + "/ast.txt");
            gen.writeSymbols(std::string(argv[3]) + "/symbols.txt");
            gen.writeIR(std::string(argv[3]) + "/ir.txt");
            std::cout << (ok ? "semantic/IR done\n" : "semantic/IR finished with errors\n");
        } else if (mode == "all") {
            if (argc < 5) { usage(); return 1; }
            std::filesystem::create_directories(argv[4]);
            Lexer lx; auto tokens = lx.scan(Lexer::readFile(argv[2]));
            Lexer::writeTokens(tokens, std::string(argv[4]) + "/tokens.txt");
            Grammar g; if (g.loadFromFile(argv[3])) { g.augment(); LR0Builder b(g); b.build(); b.writeItems(std::string(argv[4]) + "/lr0_items.txt"); b.writeSLRTable(std::string(argv[4]) + "/slr_table.txt"); }
            SemanticIRGenerator gen; gen.analyze(tokens);
            gen.writeAST(std::string(argv[4]) + "/ast.txt"); gen.writeSymbols(std::string(argv[4]) + "/symbols.txt"); gen.writeIR(std::string(argv[4]) + "/ir.txt");
            std::cout << "all outputs written to " << argv[4] << "\n";
        } else { usage(); return 1; }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n"; return 1;
    }
    return 0;
}
