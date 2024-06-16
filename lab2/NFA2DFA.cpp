#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
// 右线性文法转化NFA
// NFA确定为DFA

// 规则A->&， &表示空串
struct productionRule
{
    // 左部符号
    char _lhs;
    // 右部符号串
    std::string _rhs;
    productionRule(char lhs = '\0', const std::string& rhs = "") : _lhs(lhs), _rhs(rhs) {}

    friend std::ostream& operator<<(std::ostream& os, const productionRule& p)
    {
        return os << p._lhs << "->" << p._rhs;
    }
};

// 文法
struct Grammar
{
    char _startSymbol;// 开始符号
    std::vector<char> _nonTerminalSymbols;// 非终结符
    std::vector<char> _terminalSymbols;// 终结符
    std::vector<productionRule> _productionRules;// 产生式规则集合
    friend std::ostream& operator<<(std::ostream& os, const Grammar& g)
    {
        os << "文法描述:" << std::endl
            << "开始符号: " << g._startSymbol << std::endl;
        os << "非终结符集合={ " ;
        for(auto& c : g._nonTerminalSymbols)
            os << " " << c;
        os << " }" << std::endl;
        os << "终结符集合={";
        for(auto& c : g._terminalSymbols)
            os << " " << c;
        os << " }" << std::endl;
        os << "产生式规则:" << std::endl;
        for(auto& p : g._productionRules)
            os << p << std::endl;
        return os << std::endl;
    }
    inline int getIndexOfNonTerminal(char c) const
    {
        for(int i = 0; i < _nonTerminalSymbols.size(); ++i)
        {
            if(_nonTerminalSymbols[i] == c)
                return i;
        }
        return -1;
    }
    inline int getIndexOfTerminal(char c) const
    {
        for(int i = 0; i < _terminalSymbols.size(); ++i)
        {
            if(_terminalSymbols[i] == c)
                return i;
        }
        return -1;
    }

} grammar;

const std::string ruleFilePath = "./test.txt";
const std::string sentence = "010101010000100#";


std::vector<std::string> FileRead(const std::string &filepath)
{
    std::vector<std::string> ret;
    std::fstream fin(filepath, std::ios::in);
    if(!fin.is_open())
    {
        std::cerr << "Error: open file failed!" << std::endl;
        exit(-1);
    }
    std::string line;
    while(getline(fin, line))
        ret.push_back(line);
    return ret;
}


void init()
{
    auto lines = FileRead(ruleFilePath);
    int flag = 0;
    for(auto& line : lines)
    {
        int pos = line.find("->");
        if(pos == std::string::npos)
        {
            if(!flag)
            {
                grammar._startSymbol = line[0];
                for(auto& c : line)
                    grammar._nonTerminalSymbols.push_back(c);
                flag = 1;
            }
            else
            {
                for(auto& c : line)
                        grammar._terminalSymbols.push_back(c);
            }
        }
        else
        {
            char lhs = line[0];
            std::string rhs = line.substr(pos + 2);
            grammar._productionRules.push_back({lhs, rhs});
        }
    }
    // 结束符号 # 当作终结符加入到终结符集合中
    grammar._terminalSymbols.push_back('#');
    std::cout << grammar;
}

struct NFA
{
    // 状态图邻接矩阵
    std::vector<std::vector<char>> _stateGraph;
    // 字母表
    std::vector<char> _alaphabet;
    // 状态集合
    std::vector<char> _states;
    // 开始状态
    char _startState;
    // 接受状态
    char _acceptState;

    void constructNFA(const Grammar& g)
    {
        for(auto& p : g._productionRules)
        {
            if(p._rhs.size() == 1)
            {
                if(p._rhs[0] == '&')
                {
                    int i = g.getIndexOfNonTerminal(p._lhs);
                    int j = _states.size() - 1;
                    _stateGraph[i][j] = '&';
                }
                else
                {
                    int i = grammar.getIndexOfNonTerminal(p._lhs);
                    int j = _states.size() - 1;
                    _stateGraph[i][j] = p._rhs[0];
                }
            }
            else if(p._rhs.size() == 2)
            {
                int i = grammar.getIndexOfNonTerminal(p._lhs);
                int j = grammar.getIndexOfNonTerminal(p._rhs[1]);
                _stateGraph[i][j] = p._rhs[0];
            }
            else
            {

                std::cerr << "Error: 产生式右部" << p << "不符合右线性文法!" << std::endl;
                exit(-1);
            }
        }
    }
    NFA() = delete;
    NFA(const Grammar& g)
    {
        _stateGraph.resize(g._nonTerminalSymbols.size() + 1);
        for(auto& v : _stateGraph)
            v.resize(g._nonTerminalSymbols.size() + 1, '\0');
        for(auto& ch : g._terminalSymbols)
            _alaphabet.push_back(ch);
        for(auto& ch : g._nonTerminalSymbols)
            _states.push_back(ch);
        // 将@作为终结状态
        _acceptState = '@';
        _states.push_back(_acceptState);
        // 开始符号作为开始状态
        _startState = g._startSymbol;
        constructNFA(g);
    }
    void printNFA()
    {
        std::cout << "NFA状态图:" << std::endl;
        for(int i = 0; i < _states.size(); ++i)
        {
            for(int j = 0; j < _states.size(); ++j)
            {
                if(_stateGraph[i][j] != '\0')
                    std::cout << _states[i] << "--" << _stateGraph[i][j] << "-->" << _states[j] << std::endl;
            }
        }
    }
};


struct DFA
{
private:
    // 状态图邻接矩阵
    std::vector<std::vector<char>> _stateGraph;
    // 字母表
    std::vector<char> _alaphabet;
    // 状态集合
    std::vector<int> _states;
    // 开始状态
    int _startState;
    // 接受状态
    std::set<char> _acceptStates;

    DFA() = delete;
    DFA(const NFA& nfa) : _alaphabet(nfa._alaphabet)
    {
        // 已经计算出的状态闭包，key:(DFA用数字表示)状态，value:状态闭包（NFA中状态是char）
        std::map<int, std::set<char>> calculatedClosure;
        std::set<char> startClosure = calClosure(nfa._startState);
    }

    std::set<char> calClosure(char state)
    {
        std::set<char> closure;
        closure.insert(state);
        for(int i = 0; i < _states.size(); ++i)
        {
            if(nfa._stateGraph[state][i] == '&')
            {
                closure.insert(nfa._states[i]);
                closure.insert(calClosure(nfa._states[i]).begin(), calClosure(nfa._states[i]).end());
            }
        }
    }

};


int main()
{
    init();
    NFA nfa(grammar);
    nfa.printNFA();
    return 0;
}