#include "grammar.h"
#include <set>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

static void extractEmpty(int startPos, NonTerminal Vn, std::string express, std::set<std::string> &expressSet);

// ��ȡ�����Ƶ����ŵķ��ս����
// ע��: �˲�����ɾ��ԭ������ʽ���ܹ�ֱ���Ƶ��� A -> ��
std::set<NonTerminal> *Grammar::DerivateEmptyExpress()
{
    // ���庬�п����Ƶ����ŵķ��ս�� ����
    std::set<NonTerminal> *derivateEmptyTerminalSet = new std::set<NonTerminal>();
    size_t oldLen = 0;
    do
    {
        oldLen = derivateEmptyTerminalSet->size();
        for (auto Vn : this->m_production)
        {
            // ��Vn�󲿷��ս���Ѿ����ս������,����
            if (derivateEmptyTerminalSet->find(Vn.first) != derivateEmptyTerminalSet->end())
            {
                continue;
            }
            for (auto express : Vn.second)
            {
                // ���� A -> ��
                if (express == EMPTY)
                {
                    // �Ƴ� A -> �� �Ĳ���ʽ
                    this->m_production[Vn.first].erase(express);
                    derivateEmptyTerminalSet->insert(Vn.first);
                    break;
                }

                // ���� A -> BCDEFG...
                // Ҫʹ�� A -> ��, �����Ҫ�� B -> �� && C -> �� && D -> �� && ...
                // ע��:: string ����ʱ \0 ���ڵ�����Χ
                for (auto c : express)
                {
                    if (derivateEmptyTerminalSet->find(c) == derivateEmptyTerminalSet->end())
                    {
                        // A -> �� ������ʱ,����ǰ��������
                        goto next;
                    }
                }

                derivateEmptyTerminalSet->insert(Vn.first);
                break;
            // A -> �� ������ʱ,����ǰ��������
            next:
                continue;
            }
        }
    } while (oldLen != derivateEmptyTerminalSet->size());

    return derivateEmptyTerminalSet;
}

// �ķ�����ʽ---�����ű��ʽ
Grammar &Grammar::RemoveEmptyExpression()
{
    auto derivateEmptyTerminalSet = this->DerivateEmptyExpress();
    // ������ʱ�洢�޸ĵ�VnSet
    char tmp = '#';
    // ���ڶ��ķ������滻����ʱ����
    std::set<std::string> tmpExpresses;
    // ���ڼ�¼����ǰ�������ս���Ĳ���ʽ�ĸ���
    std::map<NonTerminal, size_t> total;
    // ��¼Vnset�ڱ��ʽ�е�����ֵ
    size_t pos = 0;
    // �������ڲ���ʽΪ �� �ļ���
    // �滻:
    // A -> ��
    // S -> Ab
    // S -> Ab | b
    for (auto VnSet : *derivateEmptyTerminalSet)
    {
        // �������еĲ���ʽ
        for (auto ite_production = this->m_production.begin(); ite_production != this->m_production.end(); ite_production++)
        {
            // ��һ���µ�Vn��ʼǰ�����ʱ����
            tmpExpresses.clear();
            // ��ÿһ������ʽ���д���,ȥ��ʱ�� tmp �ַ����� VnSet�ַ�,������� expresses����
            // �������֮����������� tmp �ַ� ��ԭΪ VnSet �ַ������ this->m_production ����ʽ��
            for (auto ite_express = ite_production->second.begin(); ite_express != ite_production->second.end(); ite_express++)
            {
                extractEmpty(0, VnSet, *ite_express, tmpExpresses);
            }
            for (auto expr : tmpExpresses)
            {
                ite_production->second.insert(expr);
            }
        }
    }

    delete derivateEmptyTerminalSet;
}

// �ݹ鴦���� Vn -> �� �ı��ʽ
// ����:
// A -> ��, S -> AcAdA ɨ�����A �� # ����
// AcAdA----
//          |----#cAdA
//          |           |---- #c#dA
//          |           |           |---- #c#d#
//          |           |           |---- #c#d
//          |           |---- #cdA
//          |                       |---- #cd#
//          |                       |---- #cd
//          |----cAdA
//                      |---- c#dA
//                      |           |---- c#d#
//                      |           |---- c#d
//                      |---- cdA
//                                  |---- cd#
//                                  |---- cd
//
static void extractEmpty(int startPos, NonTerminal Vn, std::string express, std::set<std::string> &expressSet)
{
    size_t pos = -1;
    // �ݹ����,�鲻����¼
    if (-1 == (pos = express.find(Vn, startPos)))
    {
        return;
    }
    // ����û��ȥ��Vn�ı��ʽ
    // pos+1 ��ʾ��Vn����һ����ʼ,��Ϊ�������Ϊpos��Vn�Ѿ���������,�����Ҫ���Ը�ֵ
    extractEmpty(pos + 1, Vn, express, expressSet);
    express.erase(pos, 1);
    // �ų��մ�
    // ������ʽΪ S -> Vn
    //  �� Ӧ�ò�����ֻ�� S -> Vn
    if (0 != express.size())
    {
        expressSet.insert(express);
        // ����ȥ��Vn�ı��ʽ
        // pos ����1, ��ΪVn�Ѿ���ȥ����
        extractEmpty(pos, Vn, express, expressSet);
    }
}

// ������ݹ�,Ҫ�����в���ʽ�в������ܹ��Ƶ��� �� ��ʽ��
void Grammar::ExtractLeftRecursion()
{
    // �洢����˳������ŵķ��ս����
    std::vector<NonTerminal> nonTerminalArray;
    // ��¼���ս������
    size_t size = this->m_nonterminalSet.size();
    // �����Ҫɾ���Ĳ���ʽ
    std::set<std::string> deletedProductions;
    // �����Ҫ����Ĳ���ʽ
    std::set<std::string> insertedProductions;

    // ���з��ս��
    for (NonTerminal ch : this->m_nonterminalSet)
    {
        nonTerminalArray.push_back(ch);
    }

    // �������ݹ�תΪֱ����ݹ�(����)
    for (size_t i = 0; i < size; i++)
    {
        auto &productionSet = this->m_production[nonTerminalArray[i]];
        // ȡһ������ʽ
        for (auto production = productionSet.begin(); production != productionSet.end(); production++)
        {
            // Aj
            for (size_t j = 0; j < i; j++)
            {
                // �滻
                auto subNonTerminalArray = std::vector<NonTerminal>(nonTerminalArray.begin(), nonTerminalArray.begin() + i);
                this->indirectRecursionToDirect(*production, nonTerminalArray[i], subNonTerminalArray, deletedProductions, insertedProductions);
            }
        }

        if (deletedProductions.size() != 0)
        {
            for (auto del : deletedProductions)
            {
                productionSet.erase(del);
            }
        }

        if (insertedProductions.size() != 0)
        {
            for (auto ins : insertedProductions)
            {
                productionSet.insert(ins);
            }
        }

        deletedProductions.clear();
        insertedProductions.clear();
    }

    // ����

    // ����ֱ����ݹ�

    nonTerminalArray.clear();
}

// ת�������ݹ�Ϊֱ����ݹ�(����)
void Grammar::indirectRecursionToDirect(std::string express, NonTerminal Vn, std::vector<NonTerminal> &VnSet, std::set<std::string> &deletedProductions, std::set<std::string> &insertedProductions)
{
    NonTerminal ch = express[0];

    if (!this->IsNonTerminal(ch) || VnSet.end() == std::find(VnSet.begin(), VnSet.end(), ch))
    {
        // Vn -> ch
        // ch ���� Vn ��, �����Ƶ�
        if (Vn != EMPTY_CHAR)
        {
            return;
        }
        insertedProductions.insert(express);
        return;
    }

    auto &productions = this->m_production[ch];
    for (auto pro : productions)
    {
        auto ss = (pro + express.substr(1)).c_str();
        this->indirectRecursionToDirect(pro + express.substr(1), EMPTY_CHAR, VnSet, deletedProductions, insertedProductions);
    }

    if (Vn != EMPTY_CHAR)
    {
        deletedProductions.insert(express);
        return;
    }
}

// ����ֱ����ݹ�
void Grammar::ExtractDirectRecursion()
{
    for (auto ite = this->m_production.begin(); ite != this->m_production.end(); ite++) {
        auto Vn = ite->first;
        // ���� S -> Sa
        // ������Ƶ� S -> S1d
        int order = 1;

    }
}

void Grammar::ExtractUnReachableProductions()
{
    std::set<std::string> &ofStartProduction = this->m_production[this->m_S];
    std::set<NonTerminal> ReachChar;

    return;
}