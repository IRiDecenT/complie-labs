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

const std::string ruleFilePath = "./production_rule.txt";
const std::string sentence = "(n+n)*n-n/n#";
typedef std::vector<std::vector<productionRule>> PredictTable;

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



class LL1Parser{
public:
    LL1Parser() = default;
    ~LL1Parser() = default;
    inline bool isTerminal(char c, const Grammar& g)
    {
        return std::find(g._terminalSymbols.begin(), g._terminalSymbols.end(), c) != g._terminalSymbols.end();
    }

    inline bool isNonTerminal(char c, const Grammar& g)
    {
        return std::find(g._nonTerminalSymbols.begin(), g._nonTerminalSymbols.end(), c) != g._nonTerminalSymbols.end();
    }

    // 对一个文法符号串求First集合，在求Follow集合以及构造预测分析表时使用
    std::set<char> getStringFirstSet(const std::string& s, const Grammar& g)
    {
        std::set<char> firstSet;
        if(s.empty())
            return firstSet;
        // 如果第一个符号是终结符，直接加入到First集合中
        if(isTerminal(s[0], g))
            firstSet.insert(s[0]);
        // 如果第一个符号是非终结符，递归计算其First集合
        else
        {
            auto allFirstSet = calculateFirstSet(g);
            firstSet = allFirstSet[s[0]];
            // 如果First集合中包含空串，继续往后推
            if(firstSet.find('&') != firstSet.end())
            {
                int idx = 1;
                bool next = true;
                while(next && idx < s.size())
                {
                    next = false;
                    char Y = s[idx];
                    std::set<char> firstY = allFirstSet[Y];
                    for(const auto& c : firstY)
                    {
                        if(c != '&')
                            firstSet.insert(c);
                        // 当前符号的First集合中包含空串，继续往后推
                        else
                            next = true;
                    }
                    if(next)
                        idx++;
                }
                // 如果所有符号的First集合中都包含空串，那么该符号串的First集合中也要加入空串
                if(idx == s.size())
                    firstSet.insert('&');
            }
        }
        return firstSet;
    }

    // 求对应文法的First集合
    std::map<char, std::set<char>> calculateFirstSet(const Grammar& g)
    {
        std::map<char, std::set<char>> firstSet;
        // 终结符的First集合为其本身
        for(const auto& c : g._terminalSymbols)
            firstSet[c].insert(c);
        // 非终结符的First集合初始化为空
        for(const auto& c : g._nonTerminalSymbols)
            firstSet[c] = {};
        // 空串对应的First集合为{&}
        // 需要有这个初始化，否则后续构造分析表的时候，对于空串的First集合返回为空集合，无法判断(其实可以判断，在判断时看集合是否为空，但是这样写更加清晰)
        firstSet['&'].insert('&');
        // 遍历产生式规则，计算First集合
        bool change = true;
        while(change)
        {
            change = false;
            for(const auto& p : g._productionRules)
            {
                char lhs = p._lhs;
                std::string rhs = p._rhs;
                // 如果右部第一个符号是终结符或者右部是空串，直接加入到First集合中
                if(isTerminal(rhs[0], g) || rhs[0] == '&')
                {
                    // 如果First集合中没有该终结符，加入到First集合中
                    if(firstSet[lhs].find(rhs[0]) == firstSet[lhs].end())
                    {
                        firstSet[lhs].insert(rhs[0]);
                        change = true; // 标注First集合发生了变化，最外层while循环需要继续
                    }
                }
                // 此时右部第一个符号一定是非终结符
                else
                {
                    // 如果右部第一个非终结符可以推出空，还需要继续往后推，需要一个next标志
                    bool next = true;
                    // 待判断符号的下标
                    int idx = 0;
                    while(next && idx < rhs.size())
                    {
                        next = false;
                        char Y = rhs[idx]; // 第一次循环时Y是右部第一个非终结符
                        std::set<char> firstY = firstSet[Y];
                        // 可以直接把First(Y)加入到First(lhs)中
                        // 在此处两种情况：第一次循环进入，说明是右部第一个非终结符，直接加入没问题
                        // 此后再次能进入循环说明前一个符号为空串，也可以直接加入
                        for(const auto& c : firstY)
                        {
                            if(c != '&')
                            {
                                if(firstSet[lhs].find(c) == firstSet[lhs].end())
                                {
                                    firstSet[lhs].insert(c);
                                    change = true;
                                }
                            }
                            else
                                next = true;
                        }
                        // 如果First(Y)中包含空串，继续往后推,下标加1
                        if(next)
                            idx++;
                    }
                    // X->Y1Y2...Yn, 如果Y1Y2...Yn都能推出空串，那么X的First集合中也要加入空串
                    if(idx == rhs.size())
                    {
                        if(firstSet[lhs].find('&') == firstSet[lhs].end())
                        {
                            firstSet[lhs].insert('&');
                            change = true;
                        }
                    }
                }

            }
        }
        return firstSet;
    }

    // 求对应文法的Follow集合
    std::map<char, std::set<char>> calculateFollowSet(const Grammar& g)
    {
        std::map<char, std::set<char>> followSet;
        // 非终结符的Follow集合初始化为空
        for(const auto& c : g._nonTerminalSymbols)
            followSet[c] = {};
        // 开始符号的Follow集合中加入结束符号#
        followSet[g._startSymbol].insert('#');

        // 遍历产生式规则，计算Follow集合
        bool change = true;
        while(change)
        {
            change = false;
            for(auto& p : g._productionRules)
            {
                char lhs = p._lhs;
                std::string rhs = p._rhs;
                for(int i = 0; i < rhs.size(); ++i)
                {
                    char X = rhs[i];
                    // 如果X是非终结符
                    if(isNonTerminal(X, g))
                    {
                        // X->aBb, 那么First(b)中的非空串加入到Follow(B)中
                        std::set<char>& FollowB = followSet[X];
                        auto b = rhs.substr(i + 1);
                        std::set<char> firstb = getStringFirstSet(b, g);
                        // 将firstb中的非空串加入到FollowB中
                        for(auto& c : firstb)
                        {
                            if(c != '&')
                            {
                                if(FollowB.find(c) == FollowB.end())
                                {
                                    FollowB.insert(c);
                                    change = true;
                                }
                            }
                        }
                        // X->aB, 那么Follow(A)中的非空串加入到Follow(B)中
                        // 如果B是最后一个符号或者b->&，那么Follow(A)中的所有元素加入到Follow(B)中
                        if(i == rhs.size() - 1 || firstb.find('&') != firstb.end())
                        {
                            std::set<char>& FollowA = followSet[lhs];
                            for(auto& c : FollowA)
                            {
                                if(FollowB.find(c) == FollowB.end())
                                {
                                    FollowB.insert(c);
                                    change = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return followSet;
    }

    // 构造LL1预测分析表
    PredictTable constructPredictTable(const Grammar& g)
    {
        PredictTable predictTable;
        // 初始化预测分析表
        predictTable.resize(g._nonTerminalSymbols.size());
        for(auto& v : predictTable)
            v.resize(g._terminalSymbols.size());
        // std::cout << "预测分析表大小: " << predictTable.size() << " x " << predictTable[0].size() << std::endl;
        // 遍历每一条产生式规则，填充预测分析表, A->a
        for(auto& p : g._productionRules)
        {
            auto lhs = p._lhs;
            int row = g.getIndexOfNonTerminal(lhs);
            auto Firsta = getStringFirstSet(p._rhs, g);
            auto FollowA = calculateFollowSet(g)[lhs];
            // std::cout << "First(" << p._rhs << ")={";
            // for(auto& c : Firsta)
            //     std::cout << " " << c;
            // std::cout << " }" << std::endl;
            // std::cout << "Follow(" << lhs << ")={";
            // for(auto& c : FollowA)
            //     std::cout << " " << c;
            // std::cout << " }" << std::endl;
            // 遍历First(a)中的每一个终结符，填充预测分析表
            for(auto& a : Firsta)
            {
                if(a != '&')
                {
                    int col = g.getIndexOfTerminal(a);
                    predictTable[row][col] = p;
                    // std::cout << "predictTable[" << row << "][" << col << "]=" << p << std::endl;
                }
            }
            // 如果First(a)中包含空串，那么对于每一个b属于Follow(A)，填充预测分析表
            if(Firsta.find('&') != Firsta.end() || Firsta.empty())
            {
                for(auto& b : FollowA)
                {
                    int col = g.getIndexOfTerminal(b);
                    predictTable[row][col] = p;
                    // std::cout << "predictTable[" << row << "][" << col << "]=" << p << std::endl;
                }
            }
        }
        return predictTable;
    }

    // LL1分析
    bool LL1Parse(const std::string& str, const Grammar& grammar)
    {
        // 为了方便打印栈里的元素，将deque当作栈用，栈顶在deque的尾部
        std::deque<char> symbolStack;
        PredictTable predictTable = constructPredictTable(grammar);
        symbolStack.push_back('#');
        symbolStack.push_back(grammar._startSymbol);
        int idx = 0; // 当前输入串的下标
        int count = 1; // 步骤计数
        std::cout << "LL1分析过程:" << std::endl;
        std::cout << "步骤" << "\t\t" << "栈" << "\t\t" << "输入" << "\t\t" << "推导" << std::endl;
        std::cout << count++ << "\t\t";
        for(auto& c : symbolStack)
            std::cout << c;
        std::cout << "\t\t";
        for(int i = idx; i < str.size(); ++i)
        std::cout << str[i];
        std::cout << "\t\t" << std::endl;
        while(1)
        {
            char cur = str[idx];
            char top = symbolStack.back();
            if(top == '#' && cur == '#')
                return true;
            if(isTerminal(top, grammar))
            {
                if(top == cur)
                {
                    symbolStack.pop_back();
                    idx++;
                    std::cout << count++ << "\t\t";
                    for(auto& c : symbolStack)
                        std::cout << c;
                    std::cout << "\t\t";
                    for(int i = idx; i < str.size(); ++i)
                        std::cout << str[i];
                    std::cout << "\t\t" << std::endl;
                }
                else
                {
                    std::cout << "Error: 无法匹配" << std::endl;
                    return false;
                }
            }
            else
            {
                int row = grammar.getIndexOfNonTerminal(top);
                int col = grammar.getIndexOfTerminal(cur);
                if(predictTable[row][col]._lhs == '\0')
                {
                    std::cout << "Error: 无法匹配," << "当前栈顶" << top << "当前字符" << cur << std::endl;
                    return false;
                }
                else
                {
                    symbolStack.pop_back();
                    std::string rhs = predictTable[row][col]._rhs;
                    // 空串不入栈， 其他符号逆序入栈
                    if(rhs != "&")
                    {
                        for(int i = rhs.size() - 1; i >= 0; --i)
                            symbolStack.push_back(rhs[i]);
                    }
                    std::cout << count++ << "\t\t";
                    for(auto& c : symbolStack)
                        std::cout << c;
                    std::cout << "\t\t";
                    for(int i = idx; i < str.size(); ++i)
                        std::cout << str[i];
                    std::cout << "\t\t" << predictTable[row][col] << std::endl;
                }
            }

        }

    }

    void printSet(const std::map<char, std::set<char>>& Set, const std::string& tag)
    {
        for(const auto& p : Set)
        {
            if(!isTerminal(p.first, grammar) && p.first != '&')
            {
                std::cout << tag << "(" << p.first << ")={";
                for(const auto& c : p.second)
                    std::cout << " " << c;
                std::cout << " }" << std::endl;
            }

        }
    }
    void printPredictTabel(const PredictTable& predictTable, const Grammar& g)
    {
        std::cout << "预测分析表:" << std::endl;
        for(auto& c : g._terminalSymbols)
            std::cout << "\t\t" << c;
        std::cout << std::endl;
        for(int i = 0; i < predictTable.size(); ++i)
        {
            std::cout << g._nonTerminalSymbols[i];
            for(int j = 0; j < predictTable[i].size(); ++j)
            {
                if(predictTable[i][j]._lhs == '\0')
                    std::cout << "\t\t" << "err";
                else
                    std::cout << "\t\t" << predictTable[i][j];
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
    grammar._terminalSymbols.push_back('#');
    std::cout << grammar;
}

int main()
{
    init();
    LL1Parser parser;
    auto firstSet = parser.calculateFirstSet(grammar);
    parser.printSet(firstSet, "First");
    auto followSet = parser.calculateFollowSet(grammar);
    parser.printSet(followSet, "Follow");
    auto predictTable = parser.constructPredictTable(grammar);
    parser.printPredictTabel(predictTable, grammar);
    if(parser.LL1Parse(sentence, grammar))
        std::cout << "对句子" << sentence <<"LL1分析成功!" << std::endl;
    else
        std::cout << "对句子" << sentence << ", LL1分析失败!" << std::endl;
    return 0;
}