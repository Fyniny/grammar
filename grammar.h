#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_
#include <map>
#include <set>
#include <string>
#include <vector>

// ε的输入表现形式
#define EMPTY "$"
#define EMPTY_CHAR '$'
#define NON_TERMINAL_BEGIN 'A'
#define NON_TERMINAL_END 'Z'

typedef char Terminal;
typedef char NonTerminal;
typedef std::string Prodution;

class Grammar
{
private:
    // 文法产生式集
    std::map<NonTerminal, std::set<Prodution>> m_production;
    // 文法开始符号
    NonTerminal m_S;
    // 终结符号集
    std::set<Terminal> m_termianlSet;
    // 非终结符号集
    std::set<NonTerminal> m_nonterminalSet;

    // 消除ε表达式模块
public:
    // 文法产生式---消除ε表达式
    Grammar& RemoveEmptyExpression();

private:
    // 获取可以推导出ε的非终结符集
    std::set<NonTerminal> *DerivateEmptyExpress();

    // 消除左递归
public:
    void ExtractLeftRecursion();

private:
    // 转化间接左递归为直接左递归(若有),主要完成替换功能
    void indirectRecursionToDirect(std::string express, NonTerminal Vn, std::vector<NonTerminal> &VnSet, std::set<std::string> &deletedProductions, std::set<std::string> &insertedProductions);
    // 消除直接左递归
    bool ExtractDirectRecursion();
    // 化简产生式,消除不可达的表达式
public:
    // 化简产生式,消除不可达表达式
    void ExtractUnReachableProductions();
private:
    void extractUnReachableProductions(NonTerminal ch, std::set<NonTerminal>& reach);

    // 消除回溯
public:
    void ExtractBacktracking();    

    // 求FIRST, FOLLOW
public:
    void FIRST();
    void FOLLOW(std::map<NonTerminal, std::set<Terminal>>& FIRET);

public:
    Grammar()
    {
        this->m_NextAllocPos = 'A';
        // S→aA，A→BC，B→bB，C→cC，B→ε，C→ε
        // S→aA，A→BC，B→bB，C→cC，A→C，B→b，S→a，A→B，C→c
        // this->m_S = 'S';
        // this->m_nonterminalSet.insert('S');
        // this->m_nonterminalSet.insert('A');
        // this->m_nonterminalSet.insert('B');
        // this->m_nonterminalSet.insert('C');

        // this->m_termianlSet.insert('a');
        // this->m_termianlSet.insert('b');
        // this->m_termianlSet.insert('c');

        // this->m_production['S'].insert("aA");
        // this->m_production['A'].insert("BC");
        // this->m_production['B'].insert("bB");
        // this->m_production['C'].insert("cC");
        // this->m_production['B'].insert("$");
        // this->m_production['C'].insert("$");

        // S→aD，S→bB，A→BC，D→AC，B→bB，C→cC，B→ε，C→ε
        // this->m_nonterminalSet.insert('S');
        // this->m_nonterminalSet.insert('A');
        // this->m_nonterminalSet.insert('B');
        // this->m_nonterminalSet.insert('C');
        // this->m_nonterminalSet.insert('D');

        // this->m_termianlSet.insert('a');
        // this->m_termianlSet.insert('b');
        // this->m_termianlSet.insert('c');

        // this->m_production['S'].insert("aD");
        // this->m_production['S'].insert("bB");
        // this->m_production['D'].insert("AC");
        // this->m_production['A'].insert("BC");
        // this->m_production['B'].insert("bB");
        // this->m_production['C'].insert("cC");
        // this->m_production['B'].insert("$");
        // this->m_production['C'].insert("$");

        // S→Qc S→c Q→Rb,Q→b,R→Sa,R→a
        this->m_S = 'S';
        this->m_termianlSet.insert('a');
        this->m_termianlSet.insert('b');
        this->m_termianlSet.insert('c');

        this->m_nonterminalSet.insert('S');
        this->m_nonterminalSet.insert('Q');
        this->m_nonterminalSet.insert('R');

        this->m_production['S'].insert("Qc");
        this->m_production['S'].insert("c");
        this->m_production['Q'].insert("Rb");
        this->m_production['Q'].insert("b");
        this->m_production['R'].insert("Sa");
        this->m_production['R'].insert("a");

        // // S→AC S→B A→a C→c C→BC E→aA E→e
        // this->m_termianlSet.insert('a');
        // this->m_termianlSet.insert('e');
        // this->m_termianlSet.insert('c');

        // this->m_nonterminalSet.insert("S");
        // this->m_nonterminalSet.insert("A");
        // this->m_nonterminalSet.insert("C");
        // this->m_nonterminalSet.insert("B");
        // this->m_nonterminalSet.insert("E");

        // this->m_production["S"].insert("AC");
        // this->m_production["S"].insert("B");
        // this->m_production["A"].insert("a");
        // this->m_production["C"].insert("c");
        // this->m_production["C"].insert("BC");
        // this->m_production["E"].insert("aA");
        // this->m_production["E"].insert("e");
    }
public:
    bool IsTerminal(Terminal ch);
    bool IsNonTerminal(NonTerminal ch);

private:
    // 存放错误信息
    std::string m_error;
    // 记录下一次申请非终极符的起始位置,应被初始化为 A
    NonTerminal m_NextAllocPos;
    // 申请新的非终结符,目前最多申请26个(即ABCD...)
    NonTerminal AllocNonTerminal();
};

#endif