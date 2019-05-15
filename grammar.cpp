#include "grammar.h"
#include <set>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>

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

    // ����ֱ����ݹ�
    this->ExtractDirectRecursion();

    // ����,�������ɴ���ʽ
    this->ExtractUnReachableProductions();

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
bool Grammar::ExtractDirectRecursion()
{
    NonTerminal allocNonTerminal = -1;
    NonTerminal Vn;
    bool hasLeftRecrusion = false;
    // ��ȥ������ݹ�֮��Ĳ���ʽ�ļ��ϵĸ���
    std::set<Prodution> tmpProduction;

    for (auto ite = this->m_production.begin(); ite != this->m_production.end(); ite++)
    {
        Vn = ite->first;
        allocNonTerminal = -1;
        hasLeftRecrusion = false;
        tmpProduction.clear();

        // �ж��Ƿ������ݹ�
        for (auto expression : ite->second)
        {
            if (expression[0] == Vn)
            {
                hasLeftRecrusion = true;
                break;
            }
        }

        if (!hasLeftRecrusion)
        {
            continue;
        }

        // ���Է����µķ��ս��
        if ((allocNonTerminal = this->AllocNonTerminal()) == -1)
        {
            return false;
        }

        // �����ݹ�
        // A -> Ab | a     ----------- 1
        // A -> abB        ----------- 2
        // B -> bB | ��     ----------- 3

        // ����ݹ鲿�ִ�productionSet��ɾ��,���ĳ��еݹ����ʽ����allocNonTermianl����
        // ��3��ʵ��
        for (auto expression = ite->second.begin(); expression != ite->second.end(); expression++)
        {
            if ((*expression)[0] == Vn)
            {
                auto tmp = *expression;
                expression = ite->second.erase(expression);
                this->m_production[allocNonTerminal].insert(tmp.erase(0, 1) + allocNonTerminal);
            }
        }

        // ��2��ʵ��
        tmpProduction = ite->second;
        ite->second.clear();
        for (auto express : tmpProduction)
        {
            ite->second.insert(express + allocNonTerminal);
        }
        this->m_production[allocNonTerminal].insert(EMPTY);
    }
}

// �����µķ��ս��,Ŀǰ�������26��(��ABCD...)
NonTerminal Grammar::AllocNonTerminal()
{
    NonTerminal termianl = this->m_NextAllocPos;
    if (!(termianl >= NON_TERMINAL_BEGIN && termianl <= NON_TERMINAL_END))
    {
        this->m_error = "�������ܷ���ķ��ս���ŵ����ֵ\n";
        return -1;
    }

    while (true)
    {
        // �ѵǼǵķ��ս�����в�����ʱ,����Է���
        if (this->m_nonterminalSet.find(termianl) == this->m_nonterminalSet.end())
        {
            this->m_NextAllocPos++;
            this->m_nonterminalSet.insert(termianl);
            return termianl;
        }

        termianl = ++this->m_NextAllocPos;
        if (!(termianl >= NON_TERMINAL_BEGIN && termianl <= NON_TERMINAL_END))
        {
            this->m_error = "�������ܷ���ķ��ս���ŵ����ֵ\n";
            return -1;
        }
    }
}

// �������ʽ,�������ɴ���ʽ
// ���ķ���ʼ����ʼ,�ҳ����п����Ƶ������ķ��ս��
// ʹ����������㷨˼·
void Grammar::ExtractUnReachableProductions()
{
    std::set<NonTerminal> reach;
    // �ķ���ʼ��
    NonTerminal start = this->m_S;
    // �ķ���ʼ���ſ�ʼ���ķ�����ʽ
    std::set<Prodution> productions = this->m_production[start];
    size_t oldReachSize = 0;

    this->extractUnReachableProductions(start, reach);

    this->m_nonterminalSet.clear();
    this->m_nonterminalSet = reach;
    // ���÷�����ս����ָ��
    this->m_NextAllocPos = NON_TERMINAL_BEGIN;
}

// ����㷨
void Grammar::extractUnReachableProductions(NonTerminal ch, std::set<NonTerminal> &reach)
{
    std::queue<NonTerminal> nonTerminalQueue;
    std::set<Prodution> *VnProduction;
    // ch �����Ѿ�������
    if (reach.find(ch) != reach.end() || !this->IsNonTerminal(ch))
    {
        return;
    }

    reach.insert(ch);
    VnProduction = &(this->m_production[ch]);

    // ��ch�Ĳ���ʽ�����еķ��ս�����������,�ų��Ѵ���reach�ķ��ս��
    for (auto production : *VnProduction)
    {
        for (auto c : production)
        {
            // this->m_production.find(c) != this->m_production.end() Ϊ�˷�ֹ����һ���µ�this->m_production[c]��
            // && this->m_production.find(c) != this->m_production.end() &&this->m_production[c].size() > 0 // ���ɴ����,��δ����,����ȫ���ɴ�
            // A->BC B->C C���ɴ�,�����һ��������Ӧ���ǲ��Ϸ���,δ����������
            // A->BC B->C C->a Ĭ�϶������ֿɴ�Ĳ���ʽ
            if (this->IsNonTerminal(c) && reach.find(c) == reach.end())
            {
                nonTerminalQueue.push(c);
            }
        }
    }

    while (nonTerminalQueue.size() != 0)
    {
        this->extractUnReachableProductions(nonTerminalQueue.front(), reach);
        nonTerminalQueue.pop();
    }
}

bool Grammar::IsNonTerminal(NonTerminal ch)
{
    return this->m_nonterminalSet.find(ch) != this->m_nonterminalSet.end();
}

bool Grammar::IsTerminal(Terminal ch)
{
    return this->m_termianlSet.find(ch) != this->m_termianlSet.end();
}

