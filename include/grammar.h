#ifndef GRAMMAR_H
#define GRAMMAR_H
#include <map>
#include <set>
#include <string>
#include <vector>

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
};

struct Item {
    int prod;
    int dot;
    bool operator<(const Item& other) const {
        if (prod != other.prod) return prod < other.prod;
        return dot < other.dot;
    }
    bool operator==(const Item& other) const { return prod == other.prod && dot == other.dot; }
};

class Grammar {
public:
    std::vector<Production> prods;
    std::string start;
    std::set<std::string> nonterms;
    std::set<std::string> terms;

    bool loadFromFile(const std::string& path);
    void augment();
    void computeSymbols();
    std::map<std::string, std::set<std::string>> firstSets() const;
    std::map<std::string, std::set<std::string>> followSets() const;
    static std::vector<std::string> splitSymbols(const std::string& s);
    void print() const;
};

struct SLRTable {
    std::map<std::pair<int,std::string>, std::string> action;
    std::map<std::pair<int,std::string>, int> goTo;
    std::vector<std::string> conflicts;
};

class LR0Builder {
public:
    explicit LR0Builder(Grammar g): grammar(std::move(g)) {}
    void build();
    std::set<Item> closure(const std::set<Item>& I) const;
    std::set<Item> goTo(const std::set<Item>& I, const std::string& X) const;
    SLRTable buildSLRTable() const;
    void writeItems(const std::string& path) const;
    void writeSLRTable(const std::string& path) const;
    const std::vector<std::set<Item>>& states() const { return C; }
    const Grammar& getGrammar() const { return grammar; }
private:
    Grammar grammar;
    std::vector<std::set<Item>> C;
    std::map<std::pair<int,std::string>, int> transitions;
    int stateIndex(const std::set<Item>& s) const;
    std::string itemString(const Item& it) const;
};
#endif
