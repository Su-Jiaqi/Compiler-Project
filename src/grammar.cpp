#include "grammar.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}

std::vector<std::string> Grammar::splitSymbols(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> v; std::string x;
    while (iss >> x) if (x != "ε") v.push_back(x);
    return v;
}

bool Grammar::loadFromFile(const std::string& path) {
    std::ifstream fin(path);
    if (!fin) return false;
    prods.clear(); start.clear(); nonterms.clear(); terms.clear();
    std::string line;
    while (std::getline(fin, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t p = line.find("->");
        if (p == std::string::npos) continue;
        std::string lhs = trim(line.substr(0,p));
        std::string rhsAll = trim(line.substr(p+2));
        if (start.empty()) start = lhs;
        nonterms.insert(lhs);
        std::stringstream ss(rhsAll);
        std::string alt;
        while (std::getline(ss, alt, '|')) {
            prods.push_back({lhs, splitSymbols(trim(alt))});
        }
    }
    computeSymbols();
    return !prods.empty();
}

void Grammar::augment() {
    std::string old = start;
    std::string ns = start + "'";
    while (nonterms.count(ns)) ns += "'";
    prods.insert(prods.begin(), {ns, {old}});
    start = ns;
    computeSymbols();
}

void Grammar::computeSymbols() {
    nonterms.clear(); terms.clear();
    for (auto& p: prods) nonterms.insert(p.lhs);
    for (auto& p: prods) for (auto& x: p.rhs) {
        if (!nonterms.count(x) && x != "ε") terms.insert(x);
    }
    terms.insert("$");
}

std::map<std::string, std::set<std::string>> Grammar::firstSets() const {
    std::map<std::string, std::set<std::string>> first;
    for (auto& t: terms) first[t].insert(t);
    for (auto& nt: nonterms) first[nt];
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& p: prods) {
            if (p.rhs.empty()) {
                changed |= first[p.lhs].insert("ε").second;
                continue;
            }
            bool allEps = true;
            for (auto& sym: p.rhs) {
                for (auto& a: first[sym]) if (a != "ε") changed |= first[p.lhs].insert(a).second;
                if (!first[sym].count("ε")) { allEps = false; break; }
            }
            if (allEps) changed |= first[p.lhs].insert("ε").second;
        }
    }
    return first;
}

std::map<std::string, std::set<std::string>> Grammar::followSets() const {
    auto first = firstSets();
    std::map<std::string, std::set<std::string>> follow;
    for (auto& nt: nonterms) follow[nt];
    follow.at(start).insert("$");
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& p: prods) {
            for (size_t i = 0; i < p.rhs.size(); ++i) {
                std::string B = p.rhs[i];
                if (!nonterms.count(B)) continue;
                bool betaNullable = true;
                for (size_t j = i+1; j < p.rhs.size(); ++j) {
                    std::string X = p.rhs[j];
                    for (auto& a: first[X]) if (a != "ε") changed |= follow[B].insert(a).second;
                    if (!first[X].count("ε")) { betaNullable = false; break; }
                }
                if (i+1 == p.rhs.size()) betaNullable = true;
                if (betaNullable) for (auto& a: follow[p.lhs]) changed |= follow[B].insert(a).second;
            }
        }
    }
    return follow;
}

void Grammar::print() const {
    for (size_t i=0; i<prods.size(); ++i) {
        std::cout << i << ": " << prods[i].lhs << " ->";
        if (prods[i].rhs.empty()) std::cout << " ε";
        for (auto& x: prods[i].rhs) std::cout << ' ' << x;
        std::cout << "\n";
    }
}

std::set<Item> LR0Builder::closure(const std::set<Item>& I) const {
    std::set<Item> J = I;
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<Item> items(J.begin(), J.end());
        for (auto& it: items) {
            const auto& p = grammar.prods[it.prod];
            if (it.dot >= (int)p.rhs.size()) continue;
            std::string B = p.rhs[it.dot];
            if (!grammar.nonterms.count(B)) continue;
            for (int k=0; k<(int)grammar.prods.size(); ++k) {
                if (grammar.prods[k].lhs == B) changed |= J.insert({k,0}).second;
            }
        }
    }
    return J;
}

std::set<Item> LR0Builder::goTo(const std::set<Item>& I, const std::string& X) const {
    std::set<Item> moved;
    for (auto& it: I) {
        const auto& p = grammar.prods[it.prod];
        if (it.dot < (int)p.rhs.size() && p.rhs[it.dot] == X) moved.insert({it.prod, it.dot+1});
    }
    return moved.empty() ? moved : closure(moved);
}

int LR0Builder::stateIndex(const std::set<Item>& s) const {
    for (int i=0; i<(int)C.size(); ++i) if (C[i] == s) return i;
    return -1;
}

void LR0Builder::build() {
    C.clear(); transitions.clear();
    std::set<Item> I0 = closure({{0,0}});
    C.push_back(I0);
    std::queue<int> q; q.push(0);
    std::vector<std::string> symbols;
    for (auto& t: grammar.terms) if (t != "$") symbols.push_back(t);
    for (auto& nt: grammar.nonterms) symbols.push_back(nt);
    while (!q.empty()) {
        int i = q.front(); q.pop();
        for (auto& X: symbols) {
            auto g = goTo(C[i], X);
            if (g.empty()) continue;
            int j = stateIndex(g);
            if (j < 0) { j = (int)C.size(); C.push_back(g); q.push(j); }
            transitions[{i,X}] = j;
        }
    }
}

SLRTable LR0Builder::buildSLRTable() const {
    SLRTable tbl;
    auto follow = grammar.followSets();
    auto setAction = [&](int st, const std::string& a, const std::string& val){
        auto key = std::make_pair(st,a);
        auto it = tbl.action.find(key);
        if (it != tbl.action.end() && it->second != val) tbl.conflicts.push_back("state " + std::to_string(st) + ", symbol " + a + ": " + it->second + " / " + val);
        else tbl.action[key] = val;
    };
    for (auto& tr: transitions) {
        int i = tr.first.first; std::string X = tr.first.second; int j = tr.second;
        if (grammar.terms.count(X) && X != "$") setAction(i, X, "s" + std::to_string(j));
        else if (grammar.nonterms.count(X)) tbl.goTo[{i,X}] = j;
    }
    for (int i=0; i<(int)C.size(); ++i) for (auto& it: C[i]) {
        const auto& p = grammar.prods[it.prod];
        if (it.dot == (int)p.rhs.size()) {
            if (it.prod == 0) setAction(i, "$", "acc");
            else for (auto& a: follow[p.lhs]) setAction(i, a, "r" + std::to_string(it.prod));
        }
    }
    return tbl;
}

std::string LR0Builder::itemString(const Item& it) const {
    const auto& p = grammar.prods[it.prod];
    std::ostringstream oss;
    oss << p.lhs << " ->";
    for (int i=0; i<=(int)p.rhs.size(); ++i) {
        if (i == it.dot) oss << " ·";
        if (i < (int)p.rhs.size()) oss << ' ' << p.rhs[i];
    }
    return oss.str();
}

void LR0Builder::writeItems(const std::string& path) const {
    std::ofstream fout(path);
    for (int i=0; i<(int)C.size(); ++i) {
        fout << "I" << i << ":\n";
        for (auto& it: C[i]) fout << "  " << itemString(it) << "\n";
        fout << "  Goto:\n";
        for (auto& tr: transitions) if (tr.first.first == i) fout << "    " << tr.first.second << " -> I" << tr.second << "\n";
        fout << "----------------\n";
    }
}

void LR0Builder::writeSLRTable(const std::string& path) const {
    auto tbl = buildSLRTable();
    std::ofstream fout(path);
    fout << "ACTION:\n";
    for (auto& kv: tbl.action) fout << "  [" << kv.first.first << ", " << kv.first.second << "] = " << kv.second << "\n";
    fout << "GOTO:\n";
    for (auto& kv: tbl.goTo) fout << "  [" << kv.first.first << ", " << kv.first.second << "] = " << kv.second << "\n";
    if (!tbl.conflicts.empty()) {
        fout << "CONFLICTS:\n";
        for (auto& c: tbl.conflicts) fout << "  " << c << "\n";
    } else fout << "CONFLICTS: none\n";
}
