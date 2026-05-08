#include "dfa.h"
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>

static std::vector<std::string> splitWords(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> v; std::string x;
    while (iss >> x) v.push_back(x);
    return v;
}

bool DFA::loadFromFile(const std::string& path) {
    std::ifstream fin(path);
    if (!fin) return false;
    alphabet.clear(); states.clear(); acceptStates.clear(); trans.clear(); startState.clear();
    std::string line;
    enum Section { NONE, ALPHABET, STATES, START, ACCEPT, TRANS } sec = NONE;
    while (std::getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line == "alphabet:") { sec = ALPHABET; continue; }
        if (line == "states:") { sec = STATES; continue; }
        if (line == "start:") { sec = START; continue; }
        if (line == "accept:") { sec = ACCEPT; continue; }
        if (line == "transitions:") { sec = TRANS; continue; }
        auto w = splitWords(line);
        if (sec == ALPHABET) {
            for (auto& s: w) if (!s.empty()) alphabet.insert(s[0]);
        } else if (sec == STATES) {
            for (auto& s: w) states.insert(s);
        } else if (sec == START && !w.empty()) {
            startState = w[0];
        } else if (sec == ACCEPT) {
            for (auto& s: w) acceptStates.insert(s);
        } else if (sec == TRANS && w.size() >= 3) {
            trans[{w[0], w[1][0]}] = w[2];
        }
    }
    return true;
}

bool DFA::validate(std::string* error) const {
    if (startState.empty() || !states.count(startState)) {
        if (error) *error = "开始状态为空或不属于状态集";
        return false;
    }
    if (acceptStates.empty()) {
        if (error) *error = "接受状态集为空";
        return false;
    }
    for (auto& a: acceptStates) if (!states.count(a)) {
        if (error) *error = "接受状态不属于状态集: " + a;
        return false;
    }
    for (auto& kv: trans) {
        if (!states.count(kv.first.first)) {
            if (error) *error = "转移起点不属于状态集: " + kv.first.first;
            return false;
        }
        if (!alphabet.count(kv.first.second)) {
            if (error) *error = std::string("转移字符不属于字母表: ") + kv.first.second;
            return false;
        }
        if (!states.count(kv.second)) {
            if (error) *error = "转移终点不属于状态集: " + kv.second;
            return false;
        }
    }
    return true;
}

bool DFA::accepts(const std::string& input, std::vector<std::string>* trace) const {
    std::string cur = startState;
    if (trace) trace->push_back("start: " + cur);
    for (char ch: input) {
        auto it = trans.find({cur, ch});
        if (it == trans.end()) {
            if (trace) trace->push_back(cur + " --" + ch + "--> ERROR");
            return false;
        }
        if (trace) trace->push_back(cur + " --" + ch + "--> " + it->second);
        cur = it->second;
    }
    bool ok = acceptStates.count(cur);
    if (trace) trace->push_back(std::string("final: ") + cur + (ok ? " ACCEPT" : " REJECT"));
    return ok;
}

void DFA::genDfs(const std::string& prefix, int maxLen, std::vector<std::string>& out) const {
    if ((int)prefix.size() > maxLen) return;
    if (accepts(prefix)) out.push_back(prefix.empty() ? "ε" : prefix);
    if ((int)prefix.size() == maxLen) return;
    for (char c: alphabet) genDfs(prefix + c, maxLen, out);
}

std::vector<std::string> DFA::generateAccepted(int maxLen) const {
    std::vector<std::string> out;
    genDfs("", maxLen, out);
    return out;
}

void DFA::print() const {
    std::cout << "Alphabet: "; for (char c: alphabet) std::cout << c << ' '; std::cout << "\n";
    std::cout << "States: "; for (auto& s: states) std::cout << s << ' '; std::cout << "\n";
    std::cout << "Start: " << startState << "\nAccept: ";
    for (auto& s: acceptStates) std::cout << s << ' ';
    std::cout << "\nTransitions:\n";
    for (auto& kv: trans) std::cout << "  " << kv.first.first << " " << kv.first.second << " " << kv.second << "\n";
}
