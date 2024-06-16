#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
// 右线性文法转化NFA
// NFA确定为DFA

const int N = 1024;

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

std::string ruleFilePath = "./test.txt";

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

    inline int getIndexOfState(char c) const
    {
        for(int i = 0; i < _states.size(); ++i)
        {
            if(_states[i] == c)
                return i;
        }
        return -1;
    }

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


// 参考龙书上的子集构造算法实现的NFA转DFA，教材上的写的不好
struct DFA
{
    // 状态图邻接矩阵
    std::vector<std::vector<char>> _stateGraph;
    // 字母表
    std::vector<char> _alaphabet;
    // NFA状态集合和DFA状态的映射表
    std::map<std::set<char>, std::pair<int, bool>> _Dstates;
    // DFA状态集合，DFA状态用int表示即下标，对应找到NFA状态集合
    std::vector<std::set<char>> _DstatesList;
    // 开始状态
    int _startState;
    // 接受状态
    std::set<int> _acceptStates;

    DFA() = delete;

    std::set<char> getUnmarkedState()
    {
        for(auto& p : _Dstates)
        {
            if(!p.second.second)
                return p.first;
        }
        return std::set<char>();
    }

    // 求状态集合T的epsilon闭包
    std::set<char> epsilonClosure(const std::set<char>& T, const NFA& nfa)
    {
        std::set<char> ret = T;
        std::stack<char> st;
        for(auto& u : T)
            st.push(u);
        while(!st.empty())
        {
            char top = st.top();
            st.pop();
            for(int i = 0; i < nfa._states.size(); ++i)
            {
                if(nfa._stateGraph[nfa.getIndexOfState(top)][i] == '&')
                {
                    if(ret.find(nfa._states[i]) == ret.end())
                    {
                        ret.insert(nfa._states[i]);
                        st.push(nfa._states[i]);
                    }
                }
            }
        }
        return ret;
    }

    // 求状态集合T经过字符a的转移集合
    std::set<char> move(const std::set<char>& T, char a, const NFA& nfa)
    {
        std::set<char> ret;
        for(auto& u : T)
        {
            for(int i = 0; i < nfa._states.size(); ++i)
            {
                if(nfa._stateGraph[nfa.getIndexOfState(u)][i] == a)
                    ret.insert(nfa._states[i]);
            }
        }
        return ret;
    }

    DFA(const NFA& nfa) : _alaphabet(nfa._alaphabet), _startState(0)
    {
        _stateGraph.resize(N, std::vector<char>(N, '\0'));
        auto startEpsilonClosure = epsilonClosure({nfa._startState}, nfa);
        _DstatesList.push_back(startEpsilonClosure);
        _Dstates[startEpsilonClosure] = {0, false};
        while(!getUnmarkedState().empty())
        {
            auto T = getUnmarkedState();
            _Dstates[T].second = true;
            for(const auto& a : _alaphabet)
            {
                auto moveSet = move(T, a, nfa);
                if(!moveSet.empty())
                {
                    auto U = epsilonClosure(moveSet, nfa);
                    if(_Dstates.find(U) == _Dstates.end())
                    {
                        _Dstates[U] = {_Dstates.size(), false};
                        _DstatesList.push_back(U);
                    }
                    _stateGraph[_Dstates[T].first][_Dstates[U].first] = a;
                }
            }
        }
        // std::cout << "开始状态的epsilon闭包:";
        // for(auto& c : startEpsilonClosure)
        //     std::cout << c << " ";
        // std::cout << std::endl;

        // 处理DFA的结束状态
        for(int i = 0; i < _DstatesList.size(); ++i)
        {
            if(_DstatesList[i].find(nfa._acceptState) != _DstatesList[i].end())
                _acceptStates.insert(i);
        }
    }

    void printDFA()
    {
        std::cout << "DFA状态图:" << std::endl;
        for(int i = 0; i < _DstatesList.size(); ++i)
        {
            for(int j = 0; j < _DstatesList.size(); ++j)
            {
                if(_stateGraph[i][j] != '\0')
                    std::cout << i << "--" << _stateGraph[i][j] << "-->" << j << std::endl;
            }
        }
        // 打印DFA的状态集合
        std::cout << "DFA状态集合:" << std::endl;
        for(int i = 0; i < _DstatesList.size(); ++i)
        {
            std::cout << i << ": { ";
            for(auto& c : _DstatesList[i])
                std::cout << c << " ";
            std::cout << "}" << std::endl;
        }
        // 打印DFA的开始状态
        std::cout << "DFA开始状态:" << _startState << std::endl;
        // 打印DFA的接受状态
        std::cout << "DFA接受状态:" << "{ ";
        for(auto& c : _acceptStates)
            std::cout << c << " ";
        std::cout << "}" << std::endl;
    }
};

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <ruleFilePath>" << std::endl;
        exit(-1);
    }
    ruleFilePath = argv[1];
    init();
    NFA nfa(grammar);
    nfa.printNFA();
    DFA dfa(nfa);
    dfa.printDFA();
    return 0;
}