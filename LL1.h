#include <vector>
#include "character.h"
#include <map>
#include <iostream>

#ifndef LL1_H
#define LL1_H

class grammar;

// LL1分析表表项
struct LL1ITEM 
{
    character vn;
    character vt;
    production prod;
};


// FIRST,FOLLOW集合的项
class store
{
private:
    // 关联的其他集合
    std::set<const store*> m_relateTo;
    // 集合的元素
    std::set<character> m_elements;

public:
    // 插入关联表
    bool insert(const store* linkStore)
    {
        if(linkStore == nullptr)
        {
            return false;
        }

        auto result = this->m_relateTo.insert(linkStore);
        return result.second;
    }

    // 插入集合元素
    bool insert(const character& ch)
    {
        auto result = this->m_elements.insert(ch);
        return result.second;
    }

    // 将关联集中的符号自动加入m_elements，并返回结果
    // 只要有一个插入就返回true
    bool autoInsert(bool ignoreEpsilon = true)
    {
        size_t size = this->m_elements.size();
        for(auto LinkStore : this->m_relateTo)
        {
            for(auto& ch : LinkStore->m_elements)
            {
                if(ignoreEpsilon && ch.equal(Epsilon))
                {
                    continue;
                }
                this->m_elements.insert(ch);
            }
        }
        // 相等代表没有元素插入
        return size != this->m_elements.size();
    }
    
    const std::set<character>& Elems() const {
        return this->m_elements;
    }
};

// LL１　分析表
class LL1
{
private:
    const std::set<character>& Vn;
    const std::set<character>& Vt;
    const character& S;
    const std::map<character, std::set<production>>& productions;

public:
    std::map<character, store> m_FIRST;
    std::map<character, store> m_FOLLOW;
    std::vector<LL1ITEM> m_table;

public:
    // 求可以推导出空产生式的集合，并将结果返回给beta,若nbeta不为空，则把beta的补集添加到nbeta
    void Beta(std::set<character>& beta, std::set<character>* nbeta = nullptr);
    // 求FOLLOW集合
    void FOLLOW(std::map<character, store>* _FOLLOW_ = nullptr);
    // 求FIRST集合
    void FIRST(std::map<character, store>* _FIRST_ = nullptr);
    // 构建分析表
    void LL1Table();
    // 输出表
    std::ostream& print(std::ostream& os);
    std::ostream& print(std::ostream& os, const std::map<character, store>& sets, const char* title = nullptr);
    // 分析产生式
    void Analyze(production& in);
    // 分析语法获取LL1表
    void AnalyzeGrammar();
    LL1(grammar& g);
};
#endif