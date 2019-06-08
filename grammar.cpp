#include "grammar.h"
#include "character.h"
#include <set>
#include <map>
#include <iostream>
#include <queue>

using namespace std;
static void removeUnReachableProduction(const character& left, const map<character, set<production>>& prodctionMap, const set<character> Vn, set<character>& reach);


// 消除epsilon产生式
// @param productions 是传出参数 是消除epsilon后的产生式集合， key是产生式的左部， value是相同左部的所有右部的集合
void grammar::RemoveEpsilon(map<character, set<production>>& productions)
{
    // 可能产生epsilon产生式的非终结符集
    set<character> beta;

    // 不在beta的集合
    set<character> nbeta;

    productions = this->P;

    // 查找A->epsilon的产生式
    for (auto keyProductions : productions)
    {
        auto Production = keyProductions.second;
        for (auto prod : Production)
        {
            // 判断prod产生式是否为空产生式
            // prod -> epsilon
            if (prod.equal(EpsilonProduction))
            {
                // 插入到 beta 集合
                beta.insert(keyProductions.first);
                // 在原集合中删除 pro->epsilon
                productions[keyProductions.first].erase(prod);
                // 若A只存在A->epsilon,则在products中删除A记录
                if(productions[keyProductions.first].empty())
                {
                    productions.erase(keyProductions.first);
                }
                goto inBeta;
            }
        }

        nbeta.insert(keyProductions.first);
        inBeta:
            continue;
    } // for (auto keyProductions : productions) 的结束括号

    // 查找A => + epsilon
    // 这种情况只需在不在beta集合中的元素中查找
    for (auto key : nbeta)
    {
        set<production> Productions = productions[key];
        // 检查不在beta集合的元素的所有产生式
        for (auto prod: Productions)
        {
            size_t size = prod.size();
            for (size_t i = 0; i < size; i++)
            {
                const character* ch = prod.getIndexCharacter(i);
                // 获取不到下表
                if(ch == nullptr)
                {
                    cout << "look for A => +epsilon error:: \"key = ' " << key << " ' production = '" << prod << "' index = ' " << i << "' \" and when call producion.getIndexCharacter() return nullptr" << endl;
                    exit(1);
                }
                // 检查产生式(prod)中的值在不在beta集合中，若不在，不可能推出 epsilon 产生式，跳过该产生式余下查找
                if(beta.find(*ch) != beta.end())
                {
                    continue;
                }
                else
                {
                    goto nextProduction;
                }
                
            }

            // 若经过这里说明满足了条件
            beta.insert(key);
            nbeta.erase(key);

        nextProduction:
            continue;
        }
        
    } // for (auto key : nbeta)

    // 辅助队列
    queue<production> qProduction;
    // 拷贝的产生式集，用于隔离在原产生式集中插入新规则后出现出现重复迭代
    set<production> hproduction;
    // 添加的产生式规则的中间过程，可复用
    production hprod;

    // 对于A => + epsilon 的产生式添加规则
    // A -> Bc B -> epsilon 添加 A -> c
    for (auto KeyProd = productions.begin(); KeyProd != productions.end(); KeyProd++)
    {
        // 初始化辅助队列
        while(qProduction.size() != 0)
        {
            qProduction.pop();
        }

        hproduction = KeyProd->second;
        KeyProd->second.clear();

        // 处理一条产生式
        for (auto prod = hproduction.begin(); prod != hproduction.end(); prod++)
        {
            qProduction.push(*prod);
            while(!qProduction.empty())
            {
                auto tmp = qProduction.front();
                for(size_t i = 0; i < tmp.size(); i++)
                {
                    const character* ch = tmp.getIndexCharacter(i);
                    // 在beta集中查找ch，若查找成功，则需要添加规则
                    // 因为新添加的规则在查找位置之后的位置没有进行查找，故需要添加进辅助队列qProduction中，等待检查
                    // 添加规则是一个迭代的过程
                    if(beta.find(*ch) != beta.end())
                    {
                        hprod.clear();
                        tmp.copy(i, hprod);
                        qProduction.push(hprod);
                    }
                }
                KeyProd->second.insert(tmp);
                qProduction.pop();
            }
        }
    }
 }

// 化间接左递归为直接左递归
// 要求产生式无epsilon产生式 无回路(A->A)
// 若出现上诉情况将导致结果不准确
// 此操作不会产生新的非终结符
 void grammar::IndirectRecursiveToDirectRecursive(set<character>& Vn, map<character, set<production>>& productions)
 {
    // 排列Vn 
    vector<character> vVn;
    // 非终结符的个数
    size_t size = 0;
    // 去除产生式左边第一个非终结符后剩下的产生式，用于拼接成新的产生式
    production extractFisrt;
    // 拼接成的新的产生式的临时存放点
    set<production> mergeProduction;

    for (auto v : Vn)
    {
        vVn.push_back(v);
    }
    size = vVn.size();

    for (auto i = 0; i < size; i++)
    {
        for (auto j = 0; j < i; j++)
        {
            auto& prods = productions[vVn[i]];
            // 每次进行新一轮的遍历式都需要清理一下上一次生成的产生式集合避免混杂
            mergeProduction.clear();
            // 遍历相同左部的每一条产生式
            for (auto prod : prods)
            {
                // 产生式最左符号与 vVn[j]相等，需要进行改写
                if(prod.getIndexCharacter(0)->equal(vVn[j]))
                {
                    prod.copy(0, extractFisrt);
                    auto& vVnjProds = productions[vVn[j]];
                    for(auto jProd : vVnjProds)
                    {
                        // 添加在临时的存放位置是为了减少没必要的遍历次数
                        mergeProduction.insert(jProd + extractFisrt);
                    }
                    //删除
                    prods.erase(prod);
                }
            }
            prods.insert(mergeProduction.begin(), mergeProduction.end());
        }
    }
}

// 消除回溯
// 这是一个内部处理函数，调用着应该调用RemoveBacktrack()
// @param originLeft 是产生式的左部
// @param ptrProductions 是所有相同左部的产生式的右部集合
// @param productionMap 产生式的映射关系表
void grammar::removeBacktrack(const character* originLeft, std::set<production> *ptrProductions, std::map<character, std::set<production>>* const productionMap)
{
    // 分类桶，用来收集相同前缀的产生式，并在之后进行再分类
    map<character, set<production>> bucket;
    // 新注册的非终结符
    character ch;
    // 用于存放抽出相同前缀之后，剩余产生式不必进入下一轮分类分析的式子
    set<production> tmpProduction;
    production tmp;

    if(ptrProductions->size() <= 1)
    {
        return;
    }

    // 将产生式根据前缀进行分桶包装
    for(auto prod = ptrProductions->begin(); prod != ptrProductions->end(); prod++)
    {
        auto first = prod->getIndexCharacter(0);

        // 产生时中得不到前缀所以产生式已经分完，添加Epsilon产生式
        if(first == nullptr)
        {
            tmpProduction.insert(EpsilonProduction);
            continue;
        }

        bucket[*first].insert(prod->copy(0, tmp));
    }

    // 若分类后桶的大小和没分一致说明原产生式正交，无需再分
    if(ptrProductions->size() == bucket.size())
    {
        return;
    }

    // 将原来产生式清空
    ptrProductions->clear();
    ptrProductions->insert(tmpProduction.begin(), tmpProduction.end());

    // 重置tmp
    tmp.clear();

    for(auto KeyProd = bucket.begin(); KeyProd != bucket.end(); KeyProd++)
    {
        if(KeyProd->second.size() == 1)
        {
            ptrProductions->insert(production(KeyProd->first, *originLeft).append(*KeyProd->second.begin()));
            continue;
        }
        this->AllocVn(*originLeft, ch);


        removeBacktrack(&ch, &(KeyProd->second), productionMap);
        // 若 A -> Ab | Abc | Abd 
        // 则根据以上规则应该提取成：
        // A -> AA'    A'-> bA''    A'' -> epsilon | c | d
        // A'' 是多余的，下面逻辑将用于合并这种情况
        // 合并为 A -> AbA' A' -> epsilon | c | d
        // 以上分析可知当产生式大小为1时恰为这种情况
        if(KeyProd->second.size() == 1)
        {
            size_t size = KeyProd->second.begin()->size();
            // 把A添加进tmp
            tmp.append(KeyProd->first);
            // 为保守起见使用循环
            for(size_t i = 0; i < size - 1; i++)
            {
                // second 中的式子应为 bA''，提取出 b
                auto childleft = KeyProd->second.begin()->getIndexCharacter(i);
                // 添加 b
                tmp.append(*childleft);
            }

            ptrProductions->insert(tmp.append(ch));
            // 替换 A'' 为 A'
            auto nextCh = KeyProd->second.begin()->getIndexCharacter(size-1);
            (*productionMap)[ch] = (*productionMap)[*nextCh];
            (*productionMap).erase(*nextCh);
            // 释放A''
            this->ReleaseVn(*nextCh);
            continue;
        }

        // 将消除回溯折后的产生式添加回原来的产生式集
        ptrProductions->insert(production(KeyProd->first, *originLeft).append(ch));
        // 在新申请的非终极符的产生式集注册进产生式集
        (*productionMap)[ch] = KeyProd->second;
    }
    
}

// 消除回溯
// @param productionMap 待处理的产生式集，并作为结果返回，是一个传出参数
void grammar::RemoveBacktrack(std::map<character, std::set<production>> *productionMap)
{
    std::map<character, std::set<production>> productions = *productionMap;
    for(auto production : productions)
    {
        removeBacktrack(&production.first, &(*productionMap)[production.first], productionMap);
    }
}

// 消除直接左递归
// @param productionMap 待处理的产生式集，并作为结果返回，是一个传出参数
// @returns 若成功处理左递归则返回true,否则应该使用尚未消除空产生式的productions产生式集进行后续操作
bool grammar::RemoveRecursive(std::map<character, std::set<production>> *productionMap)
{
    set<production> tmp;
    std::map<character, std::set<production>> newProductionMap;
    character ch = Epsilon;
    production extractFirst;

    for(auto KeyProd = productionMap->begin(); KeyProd != productionMap->end(); KeyProd++)
    {
        const character& key = KeyProd->first;
        for(auto prod = KeyProd->second.begin(); prod != KeyProd->second.end(); )
        {
            if(prod->getIndexCharacter(0)->equal(key))
            {
                // 回路
                if(prod->size() == 1)
                {
                    cout << "production exists loop: \' " << key << "-> " << *prod << "' \n";
                    exit(1);
                }
                // ch 为新的产生式左部
                // 对于相同左部的多个含有左递归的产生式消除时只需要引入一个新的非终结符
                if(ch.equal(Epsilon))
                {
                    // 申请新的非终结符
                    this->AllocVn(key, ch);
                }
                tmp.insert(prod->copy(0, extractFirst).append(ch));
                prod = KeyProd->second.erase(prod);
                continue;
            }
            prod++;
        }

        // ch不为空串说明消除了左递归,需要插入新加入的规则
        if(!ch.equal(Epsilon))
        {
            if(KeyProd->second.size() == 0)
            {
                KeyProd->second.insert(production(ch, key));
            } 
            else
            {
                auto prod = KeyProd->second;
                KeyProd->second.clear();
                for(auto p : prod)
                {
                    KeyProd->second.insert(p.append(ch));
                }
            }
            tmp.insert(EpsilonProduction);
            newProductionMap[ch] = tmp;
        }
        tmp.clear();
        ch = Epsilon;
    } // for(auto KeyProd = productionMap->begin(); KeyProd != productionMap->end(); KeyProd++)

    if (newProductionMap.empty())
    {
        return false;
    }

    productionMap->insert(newProductionMap.begin(), newProductionMap.end());
    return true;
}

// 申请新的非终结符号
bool grammar::AllocVn(const character src, character& ch)
{
    ch = src;
    while(this->Vn.find(ch.append("'")) != this->Vn.end());
    this->Vn.insert(ch);
    return true;
}

// 释放非终结符号
void grammar::ReleaseVn(const character& vn)
{
    this->Vn.erase(vn);
}

// 语法分析
// 1. 消除空产生式
// 2. 消除递归
// 3. 消除回溯
void grammar::AnalyzeGrammar()
{
    auto Vn = this->Vn;
    bool error = false;
    map<character, set<production>> productions;
    // 检查终结符集和非终结符集是否正交
    for (auto vt : this->Vt)
    {
        if(Vn.insert(vt).second){
            continue;
        }
        cout << "[error] keyword repeat:: \"" << vt << "\"\n";
        error = true;
    }

    if(error)
    {
        return;
    }

    this->print(cout << "最初的产生式：\n");
    // 消除空产生式
    this->RemoveEpsilon(productions);
    this->print(cout << "消除空产生式：\n", &productions);
    // 转化间接左递归
    this->IndirectRecursiveToDirectRecursive(this->Vn, productions);
    this->print(cout << "转化间接左递归：\n", &productions);
    // 消除左递归
    // 若成功替代grammar中原先的产生式集
    if(this->RemoveRecursive(&productions))
    {
        cout << "[不存在左递归]\n";
        this->P = productions;
    }
    this->print(cout << "消除左递归：\n");
    // 消除回溯
    this->RemoveBacktrack(&this->P);
    this->print(cout << "消除回溯：\n");
    // 消除不可达
    this->RemoveUnReachableProduction();
    this->print(cout << "消除不可达:\n");
    // 去重
    this->MergeRepeatProduction();   
    this->print(cout << "消除重复：\n");
}

// 合并重复的无意义的表达式
// A -> Ac  D -> Dc 则合并为 A -> Ac
void grammar::MergeRepeatProduction()
{
    bool canReplace = false;
    map<character, character> srcDes;
    // 将产生式个数相同的分为一类
    map<int, map<character, set<production>>> sortedProdtions;
    for (auto keyProd : this->P)
    {
        sortedProdtions[keyProd.second.size()][keyProd.first] = keyProd.second;
    }

    for(auto sizeKeyProd : sortedProdtions)
    {
        // 含有相同产生式的不同左部的组小于1时无需合并
        if(sizeKeyProd.second.size() <= 1)
        {
            continue;
        }
    
        for(auto keyProd = sizeKeyProd.second.begin(); keyProd != sizeKeyProd.second.end(); keyProd++)
        {
            // 在替换名单中的符号无需遍历
            if(srcDes.find(keyProd->first) != srcDes.end())
            {
                continue;
            }

            auto keyProd1 = keyProd;
            for(keyProd1++; keyProd1 != sizeKeyProd.second.end(); keyProd1++)
            {
                for(auto p : keyProd1->second)
                {
                    for(auto i = 0;i < p.size(); i++)
                    {
                        if(p.getIndexCharacter(i)->equal(keyProd1->first))
                        {
                            p.replace(keyProd->first, i);
                        }
                    }
                    
                    if(keyProd->second.find(p) == keyProd->second.end())
                    {
                        canReplace = false;
                        break;
                    }

                    canReplace = true;
                }
                
                if(canReplace)
                {
                    // 将被替换的非终结符记录并从产生式中删除
                    srcDes[keyProd1->first] = keyProd->first;
                    this->P.erase(keyProd1->first);
                    this->ReleaseVn(keyProd1->first);
                }
            }
        }
    }
    
    auto P = this->P;
    // 若为真，需要删除某一条产生式并插入新的替换过后的产生式
    bool needReflash = false;
    for(auto keyProd : P)
    {
        for(auto p : keyProd.second)
        {
            auto tmp = p;
            for(auto i = 0;i < p.size(); i++)
            {
                const character ch = *p.getIndexCharacter(i);
                if(srcDes.find(ch) != srcDes.end())
                {
                    needReflash = true;
                    p.replace(srcDes[ch], i);
                }
            }
            if(needReflash)
            {
                this->P[keyProd.first].erase(tmp);
                this->P[keyProd.first].insert(p);
            }
            needReflash = false;
        }
    }
}

// 输出文法四要素
ostream& grammar::print(std::ostream& os, std::map<character, std::set<production>> *P)
{
    if(P == nullptr)
    {
        P = &(this->P);
    }
    os << "G[ " << this->S << " ] = (Vn, Vt, " << this->S << ", P)\n";
    os << "Vn = "; 
    for(auto vn : this->Vn)
    {
        os << vn << " ";
    }
    os << endl;
    os << "Vt = ";
    for(auto vt : this->Vt)
    {
        os << vt << " ";
    }
    os << endl;
    os << "P: \n";
    for (auto p : *P)
    {
        cout << p.first << "-->";
        for(auto prod : p.second)
        {
            cout << prod << "   ";
        }
        cout << endl;
    }
}

const std::set<character>& grammar::VN() const
{
    return this->Vn;
}
const std::set<character>& grammar::VT() const
{
    return this->Vt;
}
const std::map<character, std::set<production>>& grammar::Prodtions() const
{
    return this->P;
}
const character& grammar::Start() const 
{
    return this->S;
}

//初始化
void grammar::Init(const std::set<character>& Vn, const std::set<character>& Vt, const character& S, const std::map<character, std::set<production>>& P)
{
    this->S = S;
    this->Vn = Vn;
    this->Vt = Vt;
    this->P = P;
}

void grammar::RemoveUnReachableProduction()
{
    set<character> reach;
    removeUnReachableProduction(this->Start(), this->Prodtions(), this->VN(), reach);
    auto vn = this->VN();
    for(auto& ch : vn)
    {
        if(reach.find(ch) == reach.end())
        {
            this->Vn.erase(ch);
            this->P.erase(ch);
        }
    }
}

static void removeUnReachableProduction(const character& left, const map<character, set<production>>& prodctionMap, const set<character> Vn, set<character>& reach)
{
    queue<character> productionReachQueue;

    // 当前非终结符已经被检查过
    if(reach.find(left) != reach.end())
    {
        return;
    }

    reach.insert(left);
    for(auto& prod : prodctionMap.find(left)->second)
    {
        size_t i = 0;
        while(prod.getIndexCharacter(i) != nullptr)
        {
            // A->BC B->C C不可达,因此这一条理论上应该是不合法的,未考虑这个情况
            // A->BC B->C C->a 默认都是这种可达的产生式
            if(Vn.find(*prod.getIndexCharacter(i)) != Vn.end() && reach.find(*prod.getIndexCharacter(i)) == reach.end())
            {
                productionReachQueue.push(*prod.getIndexCharacter(i));
            }
            i++;
        }
    }

    while(!productionReachQueue.empty())
    {
        removeUnReachableProduction(productionReachQueue.front(), prodctionMap, Vn, reach);
        productionReachQueue.pop();
    }
}