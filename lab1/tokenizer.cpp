#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

const int _EOF_ = -2;
const int _ERROR_ = -1;
enum {
    _ID_, _INT_, _DOUBLE_, _OPERATOR_, _DELIMITER_, _KEYWORD_, _CHAR_, _STRING_, _COMMENT_, _SPACE_
};  // 类型
std::string cat[10] = { "id", "int", "double", "operator", "delimiter", "keyword", "char", "string", "comment", "space" };
const std::string op = "+-*/%=!&|<>";
std::unordered_map<std::string, int> catagoryCodeTable;

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

const int KEYWORD_NUM = 22;
const int OPERATOR_NUM = 28;
const int DELIMITER_NUM = 13;
std::vector<std::string> Keyword; // 22
std::vector<std::string> Operator; // 28
std::vector<std::string> Delimiter; // 13

struct token{
    int _type;
    std::string _catagory; // 类别
    std::string _value;    // 值
    token()=delete;
    token(int type, const std::string &val, const std::string &cat)
        :_type(type), _value(val), _catagory(cat){}
    friend std::ostream& operator << (std::ostream& os, token& t)
    {
        return os << t._catagory
                << ", type:" << t._type << ", "
                << t._value << std::endl;
    }
};


class Tokenizer{
private:
    std::string _src;
    int _pos;
    int _line;
    std::vector<token> _tokenList;
    std::string _curToken;
private:
    char peek()
    {
        if(_pos + 1 < _src.size())
            return _src[_pos + 1];
        return '\0';
    }
    inline bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
    // 是否为字母或下划线
    inline bool isLetter(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }
    inline bool isKeyword(const std::string &s) {
        return Keyword.end() != std::find(Keyword.begin(), Keyword.end(), s);
    }
    inline bool isOP(char ch) {
        return op.find(ch) != std::string::npos;
    }
    inline bool isOperator(const std::string& s) {
        return Operator.end() != std::find(Operator.begin(), Operator.end(), s);
    }
    inline bool isDelimiter(char ch) {
        for(auto& s : Delimiter)
            if(s[0] == ch) return true;
        return false;
    }

    int judge(char ch)
    {
        if(ch == '\n') ++_line;
        if(ch == '\n' || ch == ' ') return _SPACE_;
        if(isDigit(ch)) {
            char nextChar = peek();
            if(ch == '0' && nextChar == '.') { // 0.多少
                ++_pos;
                if(!isDigit(peek()))   // .后面不是数字
                    return _ERROR_;
                _curToken = "0.";
                while(isDigit(peek())) {
                    _curToken += peek();
                    ++_pos;
                }
                return _DOUBLE_;    // 8
            }  else if(ch == '0' && isLetter(nextChar)) {  // digit1
                return _ERROR_;
            }else if(ch == '0' && !isDigit(nextChar))
            { // 不是数字也不是.，说明是单纯的一个0
                _curToken = "0";
                return _INT_;   // 5
            }else if(ch != '0') {  // digit1
                _curToken = ch;
                while(isDigit(peek())) {
                    _curToken += peek();
                    ++_pos;
                }
                char nextChar = peek();
                if(nextChar == '.') {
                    _curToken += nextChar;
                    ++_pos;
                    nextChar = peek();
                    if(isDigit(nextChar)) {
                        _curToken += peek();
                        ++_pos;
                        while(isDigit(peek())) {
                            _curToken += peek();
                            ++_pos;
                        }
                        return _DOUBLE_;    // 8
                    } else return _ERROR_;
                } else return _INT_;    // 6
            } else {    // 0+数字
                ++_pos;
                return _ERROR_;         // ERROR
            }
        }
        if(isLetter(ch)) {
            _curToken = ch;
            char nextChar = peek();
            while( isLetter(nextChar) || isDigit(nextChar) ) { // 标识符~
                _curToken += nextChar;
                ++_pos;
                nextChar = peek();
            }
            return isKeyword(_curToken) ? _KEYWORD_ : _ID_;
        }
        // if(ch == '\"') {
        //     tokenList.push_back(Token(54, "\"", cat[_DELIMITER_]));
        //     tempToken = "";
        //     char nextChar = peek();
        //     while(nextChar != '\"') {
        //         tempToken += nextChar;
        //         ++pos;
        //         nextChar = peek();
        //     }
        //     tokenList.push_back(Token(69, tempToken, cat[_STRING_]));
        //     tokenList.push_back(Token(54, "\"", cat[_DELIMITER_]));
        //     pos += 2;
        //     return _STRING_;
        // }
        // if(ch == '\'') {
        //     tempToken = "";
        //     ++pos;
        //     char nextChar = peek();
        //     if(nextChar == '\'') {
        //         tokenList.push_back(Token(53, "\'", cat[_DELIMITER_]));
        //         tempToken += code[pos];
        //         tokenList.push_back(Token(68, tempToken, cat[_CHAR_]));
        //         tokenList.push_back(Token(53, "\'", cat[_DELIMITER_]));
        //         ++pos;
        //         return _CHAR_;
        //     } else if(code[pos] == '\'') {
        //         tokenList.push_back(Token(53, "\'", cat[_DELIMITER_]));
        //         tokenList.push_back(Token(68, tempToken, cat[_CHAR_]));  // 空字符串
        //         tokenList.push_back(Token(53, "\'", cat[_DELIMITER_]));
        //         return _CHAR_;
        //     } else {
        //         while(pos < len && nextChar != '\'') {
        //             ++pos;
        //             nextChar = peek();
        //         }
        //         ++pos;
        //         return _ERROR_;
        //     }
        // }
        if(ch == '/') {
            if(peek() == '*') {
                ++_pos;
                char nextChar = peek();
                ++_pos;
                _curToken = "";
                while(_pos < _src.size()) {
                    if(nextChar == '*' && peek() == '/') {
                        _tokenList.push_back(token(catagoryCodeTable["/*"], "/*", cat[_DELIMITER_]));
                        _tokenList.push_back(token(64, _curToken, cat[_COMMENT_]));
                        _tokenList.push_back(token(catagoryCodeTable["*/"], "*/", cat[_DELIMITER_]));
                        ++_pos;
                        ++_pos;
                        return _COMMENT_;
                    } else {
                        _curToken += nextChar;
                        nextChar = peek();
                        ++_pos;
                    }
                }
            } else return _ERROR_;
        }

        if(isOP(ch)) {   // op运算符
            _curToken = "";
            _curToken += ch;
            char nextChar = peek();
            if(isOP(nextChar)) {
                if(isOperator(_curToken + nextChar)) {
                    _curToken += nextChar;
                    ++_pos;
                    return _OPERATOR_;      // 15
                } else return _OPERATOR_;   // 14
            } else return _OPERATOR_;       // 14
        }
        if(isDelimiter(ch)) {
            _curToken = "";
            _curToken += ch;
            return _DELIMITER_;
        }
        return _ERROR_;
    }

    int next()
    {
        int type = judge(_src[_pos]);
        // 处理空格和换行
        while(_pos < _src.size() && type == _SPACE_) {
            ++_pos;
            type = judge(_src[_pos]);
        }
        // 位于本文末尾 EOF
        if(_pos >= _src.size()) return _EOF_;
        ++_pos;

        if(type == _ERROR_) return _ERROR_;
        if(type == _DOUBLE_) {
            _tokenList.push_back(token(catagoryCodeTable[cat[_DOUBLE_]], _curToken, cat[_DOUBLE_]));
            return _DOUBLE_;
        }
        if(type == _INT_) {
            _tokenList.push_back(token(catagoryCodeTable[cat[_INT_]], _curToken, cat[_INT_]));
            return _INT_;
        }
        if(type == _ID_) {  // 标识符
            _tokenList.push_back(token(catagoryCodeTable[cat[_ID_]], _curToken, cat[_ID_]));
            return _ID_;
        }
        if(type == _OPERATOR_ || type == _KEYWORD_ || type == _DELIMITER_) {
            _tokenList.push_back(token(catagoryCodeTable[_curToken], _curToken, cat[type]));
            return type;
        }
        if(type == _COMMENT_) {
            return _COMMENT_;
        }
        return _ERROR_;
    }

public:
    Tokenizer(): _src(), _pos(0), _line(1), _tokenList()
    {
        Keyword.resize(KEYWORD_NUM);
        Operator.resize(OPERATOR_NUM);
        Delimiter.resize(DELIMITER_NUM);
        auto catagory = FileRead("./catagory.txt");
        for(int i = 0; i < KEYWORD_NUM; ++i)
        {
            Keyword[i] = catagory[i];
            catagoryCodeTable[Keyword[i]] = i;
        }
        for(int i = 0; i < OPERATOR_NUM; ++i)
        {
            Operator[i] = catagory[i + KEYWORD_NUM];
            catagoryCodeTable[Operator[i]] = i + KEYWORD_NUM;
        }
        for(int i = 0; i < DELIMITER_NUM; ++i)
        {
            Delimiter[i] = catagory[i + KEYWORD_NUM + OPERATOR_NUM];
            catagoryCodeTable[Delimiter[i]] = i + KEYWORD_NUM + OPERATOR_NUM;
        }
        // for(auto &i : catagoryCodeTable)
        //     std::cout << i.first << " " << i.second << std::endl;
        // std::cout << catagoryCodeTable.size() << std::endl;
    }
    Tokenizer(const std::string &src, int, int) = delete;
    Tokenizer(const Tokenizer&) = delete;
    ~Tokenizer() {}
    void loadSrcCode(const std::string& filepath)
    {
        _tokenList.clear();

        _pos = 0;
        _line = 1;
        auto lines = FileRead(filepath);
        for(auto &line : lines){
            _src += line;
            _src += '\n';
        }
        //std::cout << _src << std::endl;
    }

    void Tokenize()
    {
        while(_pos < _src.size())
        {
            auto flag = next();
            if(flag == _EOF_) break;
            if(flag == _ERROR_)
                _tokenList.push_back(token(_ERROR_, "FIND ERROR in line " + std::to_string(_line), "ERROR"));
        }
        std::cout << "Tokenize finished!" << std::endl;
    }


    std::vector<token> getTokenList()
    {
        return _tokenList;
    }
};


int main()
{
    Tokenizer tokenizer;
    tokenizer.loadSrcCode("./test.c");
    tokenizer.Tokenize();
    auto tokenList = tokenizer.getTokenList();
    for(auto &t : tokenList)
        std::cout << t;
    return 0;
}