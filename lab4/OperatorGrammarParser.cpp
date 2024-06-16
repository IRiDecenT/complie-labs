#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <deque>

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
    // 等价非终结符集合
    std::set<char> _closure;
    inline bool isNonTerminal(char c) const
    {
        return std::find(_nonTerminalSymbols.begin(), _nonTerminalSymbols.end(), c) != _nonTerminalSymbols.end();
    }

    inline bool isTerminal(char c) const
    {
        return std::find(_terminalSymbols.begin(), _terminalSymbols.end(), c) != _terminalSymbols.end();
    }

    char reduction(const std::string &rhs) const
    {

        for(auto& p : _productionRules)
        {
            bool flag = true;
            if(p._rhs == rhs)
                return p._lhs;
            else if(p._rhs.size() == rhs.size())
            {
                for(int i = 0; i < rhs.size(); ++i)
                {
                    if(p._rhs[i] == rhs[i])
                        continue;
                    else
                    {
                        if(isNonTerminal(p._rhs[i]) && isNonTerminal(rhs[i]))
                        {
                            if(_closure.find(p._rhs[i]) != _closure.end() && _closure.find(rhs[i]) != _closure.end())
                                continue;
                            else
                            {
                                flag = false;
                                break;
                            }
                        }
                        else
                        {
                            flag = false;
                            break;
                        }
                    }
                }
                if(flag)
                    return p._lhs;
            }
        }
        return '\0';
    }

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

const std::string ruleFilePath = "./operatorgrammar.txt";
const std::string sentence = "(i+i)*i-i/i#";
// const std::string sentence = "i+i#";

typedef std::vector<std::vector<char>> PriorityTable;


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

class OperatorGrammarParser
{
public:
    std::map<char, std::set<char>> getAllFirstVT(const Grammar& g)
    {
        std::map<char, std::set<char>> firstVT;
        for(auto& c : g._nonTerminalSymbols)
            firstVT[c] = {};
        bool change = true;
        while(change)
        {
            change = false;
            for(auto& p : g._productionRules)
            {
                char lhs = p._lhs;
                std::string rhs = p._rhs;
                if(g.isTerminal(rhs[0]))
                {
                    if(firstVT[lhs].find(rhs[0]) == firstVT[lhs].end())
                    {
                        firstVT[lhs].insert(rhs[0]);
                        change = true;
                    }
                }
                else
                {
                    if(rhs.size() > 1 && g.isTerminal(rhs[1]))
                    {
                        if(firstVT[lhs].find(rhs[1]) == firstVT[lhs].end())
                        {
                            firstVT[lhs].insert(rhs[1]);
                            change = true;
                        }
                    }
                    else
                    {
                        for(auto& c : firstVT[rhs[0]])
                        {
                            if(firstVT[lhs].find(c) == firstVT[lhs].end())
                            {
                                firstVT[lhs].insert(c);
                                change = true;
                            }
                        }
                    }
                }
            }
        }
        return firstVT;
    }

    std::map<char, std::set<char>> getAllLastVT(const Grammar& g)
    {
        std::map<char, std::set<char>> lastVT;
        for(auto& c : g._nonTerminalSymbols)
            lastVT[c] = {};
        bool change = true;
        while(change)
        {
            change = false;
            for(auto& p : g._productionRules)
            {
                char lhs = p._lhs;
                std::string rhs = p._rhs;
                if(g.isTerminal(rhs[rhs.size() - 1]))
                {
                    if(lastVT[lhs].find(rhs[rhs.size() - 1]) == lastVT[lhs].end())
                    {
                        lastVT[lhs].insert(rhs[rhs.size() - 1]);
                        change = true;
                    }
                }
                else
                {
                    if(rhs.size() > 1 && g.isTerminal(rhs[rhs.size() - 2]))
                    {
                        if(lastVT[lhs].find(rhs[rhs.size() - 2]) == lastVT[lhs].end())
                        {
                            lastVT[lhs].insert(rhs[rhs.size() - 2]);
                            change = true;
                        }
                    }
                    else
                    {
                        for(auto& c : lastVT[rhs[rhs.size() - 1]])
                        {
                            if(lastVT[lhs].find(c) == lastVT[lhs].end())
                            {
                                lastVT[lhs].insert(c);
                                change = true;
                            }
                        }
                    }
                }
            }
        }
        return lastVT;
    }

    PriorityTable constructPriorityTable(const Grammar& g)
    {
        auto firstVT = getAllFirstVT(g);
        auto lastVT = getAllLastVT(g);

        PriorityTable table(grammar._terminalSymbols.size(), std::vector<char>(grammar._terminalSymbols.size(), ' '));
        for(auto& p : g._productionRules)
        {
            char lhs = p._lhs;
            std::string rhs = p._rhs;
            for(int i = 0; i < rhs.size() - 1; ++i)
            {
                if(i + 1 < rhs.size() && g.isTerminal(rhs[i]) && g.isTerminal(rhs[i + 1]))
                {
                    int row = g.getIndexOfTerminal(rhs[i]);
                    int col = g.getIndexOfTerminal(rhs[i + 1]);
                    table[row][col] = '=';
                }
                if(i + 1 < rhs.size() && i + 2 < rhs.size() && g.isTerminal(rhs[i]) && g.isNonTerminal(rhs[i + 1]) && g.isTerminal(rhs[i + 2]))
                {
                    int row = g.getIndexOfTerminal(rhs[i]);
                    int col = g.getIndexOfTerminal(rhs[i + 2]);
                    table[row][col] = '=';
                }
                if(i + 1 < rhs.size() && g.isTerminal(rhs[i]) && g.isNonTerminal(rhs[i + 1]))
                {
                    for(auto& c : firstVT[rhs[i + 1]])
                    {
                        int row = g.getIndexOfTerminal(rhs[i]);
                        int col = g.getIndexOfTerminal(c);
                        table[row][col] = '<';
                    }
                }
                if(i + 1 < rhs.size() && g.isNonTerminal(rhs[i]) && g.isTerminal(rhs[i + 1]))
                {
                    for(auto& c : lastVT[rhs[i]])
                    {
                        int row = g.getIndexOfTerminal(c);
                        int col = g.getIndexOfTerminal(rhs[i + 1]);
                        table[row][col] = '>';
                    }
                }
            }
            // 处理终结符 #
            for(int i = 0; i < g._terminalSymbols.size() - 1; ++i)
            {
                table[i][g.getIndexOfTerminal('#')] = '>';
                table[g.getIndexOfTerminal('#')][i] = '<';
            }

        }
        return table;
    }

    void operatorGrammarParser(const Grammar& g, std::string sentence)
    {
        auto priorityTable = constructPriorityTable(g);
        std::deque<char> analyseStack;
        analyseStack.push_back('#');
        int idx = 0;
        int count = 1;
        int pos = 0;
        std::cout << "算符优先文法分析过程:" << std::endl;
        std::cout << "步骤\t\t分析栈\t\t输入串\t\t动作" << std::endl;
        while(1)
        {
            char a = sentence[idx];
            if(analyseStack.size() == 2 && analyseStack.back() == g._startSymbol && sentence[idx] == '#')
            {
                std::cout << count++ << "\t\t";
                for(auto& c : analyseStack)
                    std::cout << c;
                std::cout << "\t\t" << sentence.substr(idx) << "\t\t接受" << std::endl;
                std::cout << "接受: "<< sentence << std::endl;
                return;
            }
            else
            {
                if(g.isTerminal(analyseStack.back()))
                    pos = analyseStack.size() - 1;
                else
                    pos = analyseStack.size() - 2;
                if(priorityTable[g.getIndexOfTerminal(analyseStack[pos])][g.getIndexOfTerminal(a)] == '<'
                    || priorityTable[g.getIndexOfTerminal(analyseStack[pos])][g.getIndexOfTerminal(a)] == '=')
                {

                    std::cout << count++ << "\t\t";
                    for(auto& c : analyseStack)
                        std::cout << c;
                    std::cout << "\t\t" << sentence.substr(idx) << "\t\t移进" << std::endl;
                    analyseStack.push_back(a);
                    ++idx;
                }
                else if(priorityTable[g.getIndexOfTerminal(analyseStack[pos])][g.getIndexOfTerminal(a)] == '>')
                {

                    std::cout << count++ << "\t\t";
                    for(auto& c : analyseStack)
                        std::cout << c;
                    std::cout << "\t\t" << sentence.substr(idx) << "\t\t规约" << std::endl;
                    bool reductable = false;
                    std::string rhs = "";
                    // for(int i = analyseStack.size() - 1; i >= 0; --i)
                    // {
                    //     rhs = "";
                    //     for(int j = i; j < analyseStack.size(); ++j)
                    //         rhs += analyseStack[j];
                    //     if(g.reduction(rhs) != '\0')
                    //     {
                    //         reductable = true;
                    //         break;
                    //     }
                    // }
                    // if(reductable)
                    // {
                    //     int deleteCount = rhs.size();
                    //     while(deleteCount--)
                    //         analyseStack.pop_back();
                    //     analyseStack.push_back(g.reduction(rhs));
                    //     continue;
                    // }
                    for(int i = pos - 1; i >= 0; --i)
                    {
                        if(g.isTerminal(analyseStack[i]) && priorityTable[g.getIndexOfTerminal(analyseStack[i])][g.getIndexOfTerminal(analyseStack[pos])] == '<')
                        {
                            int begin = i + 1;
                            int deleteCount = analyseStack.size() - begin;
                            std::string rhs = "";
                            for(int j = i + 1; j < analyseStack.size(); ++j)
                                rhs += analyseStack[j];
                            // std::cout << rhs << std::endl;
                            while(deleteCount--)
                                analyseStack.pop_back();
                            if(g.reduction(rhs) != '\0')
                            {
                                analyseStack.push_back(g.reduction(rhs));
                                break;
                            }
                            else
                            {
                                std::cout << "Error: 不存在规约的产生式!" << std::endl;
                                exit(-1);
                            }
                        }
                    }

                }
                else
                {
                    std::cout << "Error: 不存在优先关系无法识别的算符优先文法!" << std::endl;
                    exit(-1);
                }

            }
        }
    }

    void printSet(const std::map<char, std::set<char>>& Set, const std::string& tag)
    {
        for(const auto& p : Set)
        {
            std::cout << tag << "(" << p.first << ")={";
            for(const auto& c : p.second)
                std::cout << " " << c;
            std::cout << " }" << std::endl;
        }
    }

    void printPriorityTable(const PriorityTable& priorityTable, const Grammar& g)
    {
        std::cout << "算符优先级表:" << std::endl;
        for(auto& c : g._terminalSymbols)
            std::cout << "\t" << c;
        std::cout << std::endl;
        for(int i = 0; i < priorityTable.size(); ++i)
        {
            std::cout << g._terminalSymbols[i];
            for(int j = 0; j < priorityTable[i].size(); ++j)
            {
                if(priorityTable[i][j] != '\0')
                    std::cout << "\t" << priorityTable[i][j];
            }
            std::cout << std::endl;
        }
    }
};


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
    // 对于单规则而言, 该写法存在bug
    grammar._terminalSymbols.push_back('#');
    for(auto& p : grammar._productionRules)
    {
        if(p._rhs.size() == 1 && grammar.isNonTerminal(p._rhs[0]))
        {
            grammar._closure.insert(p._lhs);
            grammar._closure.insert(p._rhs[0]);

        }
    }
    std::cout << grammar;
}

int main()
{
    init();
    OperatorGrammarParser parser;
    auto firstVT = parser.getAllFirstVT(grammar);
    parser.printSet(firstVT, "FirstVT");
    auto lastVT = parser.getAllLastVT(grammar);
    parser.printSet(lastVT, "LastVT");
    auto table = parser.constructPriorityTable(grammar);
    parser.printPriorityTable(table, grammar);
    parser.operatorGrammarParser(grammar, sentence);
    return 0;
}