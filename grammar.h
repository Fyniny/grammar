#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_
#include <map>
#include <set>
#include <string>
#include <vector>

// �ŵ����������ʽ
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
    // �ķ�����ʽ��
    std::map<NonTerminal, std::set<Prodution>> m_production;
    // �ķ���ʼ����
    NonTerminal m_S;
    // �ս���ż�
    std::set<Terminal> m_termianlSet;
    // ���ս���ż�
    std::set<NonTerminal> m_nonterminalSet;

    // �����ű��ʽģ��
public:
    // �ķ�����ʽ---�����ű��ʽ
    Grammar& RemoveEmptyExpression();

private:
    // ��ȡ�����Ƶ����ŵķ��ս����
    std::set<NonTerminal> *DerivateEmptyExpress();

    // ������ݹ�
public:
    void ExtractLeftRecursion();

private:
    // ת�������ݹ�Ϊֱ����ݹ�(����),��Ҫ����滻����
    void indirectRecursionToDirect(std::string express, NonTerminal Vn, std::vector<NonTerminal> &VnSet, std::set<std::string> &deletedProductions, std::set<std::string> &insertedProductions);
    // ����ֱ����ݹ�
    bool ExtractDirectRecursion();
    // �������ʽ,�������ɴ�ı��ʽ
public:
    // �������ʽ,�������ɴ���ʽ
    void ExtractUnReachableProductions();
private:
    void extractUnReachableProductions(NonTerminal ch, std::set<NonTerminal>& reach);

    // ��������
public:
    void ExtractBacktracking();    

    // ��FIRST, FOLLOW
public:
    void FIRST();
    void FOLLOW(std::map<NonTerminal, std::set<Terminal>>& FIRET);

public:
    Grammar()
    {
        this->m_NextAllocPos = 'A';
        // S��aA��A��BC��B��bB��C��cC��B���ţ�C����
        // S��aA��A��BC��B��bB��C��cC��A��C��B��b��S��a��A��B��C��c
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

        // S��aD��S��bB��A��BC��D��AC��B��bB��C��cC��B���ţ�C����
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

        // S��Qc S��c Q��Rb,Q��b,R��Sa,R��a
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

        // // S��AC S��B A��a C��c C��BC E��aA E��e
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
    // ��Ŵ�����Ϣ
    std::string m_error;
    // ��¼��һ��������ռ�������ʼλ��,Ӧ����ʼ��Ϊ A
    NonTerminal m_NextAllocPos;
    // �����µķ��ս��,Ŀǰ�������26��(��ABCD...)
    NonTerminal AllocNonTerminal();
};

#endif