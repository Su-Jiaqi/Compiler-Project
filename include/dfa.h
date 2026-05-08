#ifndef DFA_H
#define DFA_H
#include <map>
#include <set>
#include <string>
#include <vector>

class DFA {
public:
    bool loadFromFile(const std::string& path);
    bool validate(std::string* error = nullptr) const;
    bool accepts(const std::string& input, std::vector<std::string>* trace = nullptr) const;
    std::vector<std::string> generateAccepted(int maxLen) const;
    void print() const;
private:
    std::set<char> alphabet;
    std::set<std::string> states;
    std::string startState;
    std::set<std::string> acceptStates;
    std::map<std::pair<std::string,char>, std::string> trans;
    void genDfs(const std::string& prefix, int maxLen, std::vector<std::string>& out) const;
};
#endif
