#include "grammar.h"
#include <set>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

static void extractEmpty(int startPos, NonTerminal Vn, std::string express, std::set<std::string> &expressSet);

// 获取可以推导出ε的非终结符集
// 注意: 此操作会删除原来产生式中能够直接推导出 A -> ε
std::set<NonTerminal> *Grammar::DerivateEmptyExpress()
{
    // 定义含有可能推导出ε的非终结符 集合
    std::set<NonTerminal> *derivateEmptyTerminalSet = new std::set<NonTerminal>();
    size_t oldLen = 0;
    do
    {
        oldLen = derivateEmptyTerminalSet->size();
        for (auto Vn : this->m_production)
        {
            // 若Vn左部非终结符已经在终结符号中,跳过
            if (derivateEmptyTerminalSet->find(Vn.first) != derivateEmptyTerminalSet->end())
            {
                continue;
            }
            for (auto express : Vn.second)
            {
                // 处理 A -> ε
                if (express == EMPTY)
                {
                    // 移除 A -> ε 的产生式
                    this->m_production[Vn.first].erase(express);
                    derivateEmptyTerminalSet->insert(Vn.first);
                    break;
                }

                // 处理 A -> BCDEFG...
                // 要使的 A -> ε, 则必须要有 B -> ε && C -> ε && D -> ε && ...
                // 注意:: string 迭代时 \0 不在迭代范围
                for (auto c : express)
                {
                    if (derivateEmptyTerminalSet->find(c) == derivateEmptyTerminalSet->end())
                    {
                        // A -> ε 不存在时,跳过前面插入语句
                        goto next;
                    }
                }

                derivateEmptyTerminalSet->insert(Vn.first);
                break;
            // A -> ε 不存在时,跳过前面插入语句
            next:
                continue;
            }
        }
    } while (oldLen != derivateEmptyTerminalSet->size());

    return derivateEmptyTerminalSet;
}

// 文法产生式---消除ε表达式
Grammar &Grammar::RemoveEmptyExpression()
{
    auto derivateEmptyTerminalSet = this->DerivateEmptyExpress();
    // 用于暂时存储修改的VnSet
    char tmp = '#';
    // 用于对文法进行替换的临时链表
    std::set<std::string> tmpExpresses;
    // 用于记录迭代前各个非终结符的产生式的个数
    std::map<NonTerminal, size_t> total;
    // 记录Vnset在表达式中的索引值
    size_t pos = 0;
    // 遍历存在产生式为 ε 的集合
    // 替换:
    // A -> ε
    // S -> Ab
    // S -> Ab | b
    for (auto VnSet : *derivateEmptyTerminalSet)
    {
        // 遍历所有的产生式
        for (auto ite_production = this->m_production.begin(); ite_production != this->m_production.end(); ite_production++)
        {
            // 在一个新的Vn开始前清空临时集合
            tmpExpresses.clear();
            // 对每一条产生式进行处理,去除时用 tmp 字符代替 VnSet字符,并保存进 expresses链表
            // 替代结束之后遍历链表并将 tmp 字符 还原为 VnSet 字符保存进 this->m_production 产生式中
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

// 递归处理含有 Vn -> ε 的表达式
// 例子:
// A -> ε, S -> AcAdA 扫描过的A 用 # 代替
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
    // 递归出口,查不到记录
    if (-1 == (pos = express.find(Vn, startPos)))
    {
        return;
    }
    // 处理没有去除Vn的表达式
    // pos+1 表示从Vn的下一个开始,因为这个索引为pos的Vn已经做过处理,因而需要忽略该值
    extractEmpty(pos + 1, Vn, express, expressSet);
    express.erase(pos, 1);
    // 排除空串
    // 若产生式为 S -> Vn
    //  则 应该产生的只有 S -> Vn
    if (0 != express.size())
    {
        expressSet.insert(express);
        // 处理去掉Vn的表达式
        // pos 不加1, 因为Vn已经被去掉了
        extractEmpty(pos, Vn, express, expressSet);
    }
}

// 消除左递归,要求所有产生式中不存在能够推导出 ε 的式子
void Grammar::ExtractLeftRecursion()
{
    // 存储给定顺序的重排的非终结符号
    std::vector<NonTerminal> nonTerminalArray;
    // 记录非终结符个数
    size_t size = this->m_nonterminalSet.size();
    // 存放需要删除的产生式
    std::set<std::string> deletedProductions;
    // 存放需要插入的产生式
    std::set<std::string> insertedProductions;

    // 排列非终结符
    for (NonTerminal ch : this->m_nonterminalSet)
    {
        nonTerminalArray.push_back(ch);
    }

    // 将间接左递归转为直接左递归(若有)
    for (size_t i = 0; i < size; i++)
    {
        auto &productionSet = this->m_production[nonTerminalArray[i]];
        // 取一条产生式
        for (auto production = productionSet.begin(); production != productionSet.end(); production++)
        {
            // Aj
            for (size_t j = 0; j < i; j++)
            {
                // 替换
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

    // 化简

    // 消除直接左递归

    nonTerminalArray.clear();
}

// 转化间接左递归为直接左递归(若有)
void Grammar::indirectRecursionToDirect(std::string express, NonTerminal Vn, std::vector<NonTerminal> &VnSet, std::set<std::string> &deletedProductions, std::set<std::string> &insertedProductions)
{
    NonTerminal ch = express[0];

    if (!this->IsNonTerminal(ch) || VnSet.end() == std::find(VnSet.begin(), VnSet.end(), ch))
    {
        // Vn -> ch
        // ch 不在 Vn 中, 无需推导
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

// 消除直接左递归
void Grammar::ExtractDirectRecursion()
{
    for (auto ite = this->m_production.begin(); ite != this->m_production.end(); ite++) {
        auto Vn = ite->first;
        // 若有 S -> Sa
        // 则可以推导 S -> S1d
        int order = 1;

    }
}

void Grammar::ExtractUnReachableProductions()
{
    std::set<std::string> &ofStartProduction = this->m_production[this->m_S];
    std::set<NonTerminal> ReachChar;

    return;
}