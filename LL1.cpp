#include "grammar.h"
#include "character.h"
#include "LL1.h"
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <stack>

using namespace std;

LL1::LL1(grammar& g): Vn(g.VN()), Vt(g.VT()), productions(g.Prodtions()), S(g.Start()) {

}

// 求FIRST集合
// @param _FIRST_ 返回FIRST集合的副本
void LL1::FIRST(std::map<character, store>* _FIRST_)
{
    // 存在epsilon产生式的非终结符集
    set<character> beta;
    
    std::map<character, store>& _FIRST = this->m_FIRST;
    // 遍历产生式的指针
    size_t index = 0;

    // 求beta集合
    this->Beta(beta);

    // 分析产生式
    for(auto KeyProd : productions)
    {
        if(Vn.find(KeyProd.first) == Vn.end())
        {
            cout << "producion left not exists in Vn\n";
            exit(1);
        }
        const character& key = KeyProd.first;
        for(auto prod : KeyProd.second)
        {
            index = 0;
            while((prod.getIndexCharacter(index)) != nullptr)
            {
                auto ch = prod.getIndexCharacter(index++);
                // 该字符为终结符或epsilon产生式
                if(Vt.find(*ch) != Vt.end() || ch->equal(Epsilon))
                {
                    _FIRST[key].insert(*ch);
                    break;
                }

                // ch 为非终结符
                if(Vn.find(*ch) != Vn.end())
                {
                    _FIRST[key].insert(&_FIRST[*ch]);
                    // 该终结符还可以推导出epsilon
                    if(beta.find(*ch) != beta.end())
                    {
                        auto nextChar = prod.getIndexCharacter(index);
                        // 说明产生式分析结束，并且该产生式的所有符号都可以退出epsilon
                        if(nextChar == nullptr)
                        {
                            _FIRST[key].insert(Epsilon);
                        }
                        continue;
                    }
                    break;
                }
            } // end while
        }
    }

    for(bool hasInsert = true; hasInsert;)
    {
        size_t result = 0;
        size_t times = 0;
        // 每一趟循环都将每一个first集的结果是否添加在result对应的位中(从低位开始算起)
        // 若结果大于0表示有新元素添加
        for(auto fir = _FIRST.begin(); fir != _FIRST.end(); fir++)
        {
            result |= fir->second.autoInsert() << times++;
        }
        hasInsert = result > 0;
    }

    if(_FIRST_ != nullptr)
    {
        *_FIRST_ = this->m_FIRST;
    }
}

// 求出可能存在可以推导出epsilon产生式的非终极符集beta
// @param beta 结果参数
// @param Vn 非终结符
// @param Vt 终结符
// @param productions 产生式
// @param nBeta 不可能存在可以推导出epsilon产生式的非终极符集，可选参数
void LL1::Beta(set<character>& beta, set<character>* nBeta)
{
    // 不存在epsilon产生式的非终结符集
    set<character> nbeta = Vn;

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
                nbeta.erase(keyProductions.first);
                break;
            }
        }
    } // for (auto keyProductions : productions) 的结束括号


    // 查找A => + epsilon
    // 这种情况只需在nbeta集合中的元素中查找
    for (auto key : nbeta)
    {
        auto Productions = productions.find(key);
        // 检查不在beta集合的元素的所有产生式
        for (auto prod: Productions->second)
        {
            size_t size = prod.size();
            for (size_t i = 0; i < size; i++)
            {
                const character* ch = prod.getIndexCharacter(i);
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

    if(nBeta != nullptr)
    {
        *nBeta = nbeta;
    }
}

// @param _FOLLOW_ 返回FOLLOW集合的副本
void LL1::FOLLOW(std::map<character, store>* _FOLLOW_)
{
    const character& S = this->S;
    std::map<character, store>& _FOLLOW = this->m_FOLLOW;
    set<character> beta;
    Beta(beta);
    auto& _FIRST = this->m_FIRST;

    size_t index = 0;

    // 对文法开始符号添加'#'
    _FOLLOW[S].insert(Border);

    for(auto KeyProd = productions.begin(); KeyProd != productions.end(); KeyProd++)
    {
        auto& key = KeyProd->first;
        for (auto& prod : KeyProd->second)
        {
            index = 0;
            while(true)
            {
                auto ch = prod.getIndexCharacter(index);
                
                // 获取到的字符为nullptr或者epsilon说明产生式扫描结束，退出循环，进入下一个产生式的分析
                if(ch == nullptr || ch->equal(Epsilon))
                {
                    break;
                }

                // 不分析终结符
                if(Vt.find(*ch) != Vt.end()){
                    index++;
                    continue;
                }

                // 不合法的符号
                if(Vn.find(*ch) == Vn.end())
                {
                    cout << "unknown character ::" << *ch << endl;
                    exit(1);
                }

                // 查找ch后面的符号
                auto nextChar = prod.getIndexCharacter(index+1);
                if(nextChar == nullptr)
                {
                    // 右递归(S->AS)时,不把FOLLOW(S)添加到关联表中
                    if(!ch->equal(key))
                    {
                        _FOLLOW[*ch].insert(&_FOLLOW[key]);
                    }
                    index++;
                    continue;
                }

                // 当前扫描到的非终结符的下一个为终结符，则添加其到当前非终结符的FOLLOW集中，指针跳过下一终结符的扫描
                if(Vt.find(*nextChar) != Vt.end())
                {
                    _FOLLOW[*ch].insert(*nextChar);
                    index += 2;
                    continue;
                }

                // 当前扫描到的非终结符的下一个为非终结符S，则将S的FIRST集添加到当前非终结符的FOLLOW集中
                if(Vn.find(*nextChar) != Vn.end())
                {
                    auto& nextCharFirst = _FIRST.find(*nextChar)->second;
                    _FOLLOW[*ch].insert(&nextCharFirst);
                    // 若S存在epsilon,将S的FOLLOw集添加到当前符号的FOLLOW中
                    if(beta.find(*nextChar) != beta.end())
                    {
                        _FOLLOW[*ch].insert(&_FOLLOW[*nextChar]);
                    }
                    index++;
                    continue;
                }
            }// end while
        } // end 'for (auto& prod : KeyProd.second)'
    } // end 'for(auto& KeyProd : productions)'

    for(bool hasInsert = true; hasInsert;)
    {
        size_t result = 0;
        size_t times = 0;
        // 每一趟循环都将每一个first集的结果是否添加在result对应的位中(从低位开始算起)
        // 若结果大于0表示有新元素添加
        for(auto fol = _FOLLOW.begin(); fol != _FOLLOW.end(); fol++)
        {
            result |= fol->second.autoInsert() << times++;
        }
        hasInsert = result > 0;
    }

    if(_FOLLOW_ != nullptr)
    {
        *_FOLLOW_ = this->m_FOLLOW;
    }
}

// 构建LL1表
void LL1::LL1Table()
{
    set<character> beta;
    this->Beta(beta);

    bool hasEpsilonProd = false;

    /*
    * 思路：
    *   遍历每一条产生式
    *     若遇到终结符，则直接将 {左部， 终结符， 产生式}三元组添加进表中
    *     若遇到非终结符, 则将该非终结符的FIRST集元素(除Epsilon)作为三元组的第二项添加进表 {左部 非终结符的FIRST集{i} 产生式}
    *                   若该非终结符可以推出epsilon, 则要迭代下一个字符进行同样的操作
    *     若遇到空产生式，则将左部的FOLLOW集中的元素作为三元组的第二项添加进表 {左部 左部的FOLLOW集{i} 左部->epsilon}
    */
    // keyProd === A -> abc | Bbc | d
    for(auto KeyProd = this->productions.begin(); KeyProd != this->productions.end(); KeyProd++)
    {
        // key = A
        const character& key = KeyProd->first;
        // 相同左部至多只有一个可以推出空产生式
        hasEpsilonProd = false;
        // prod = abc
        for(auto prod : KeyProd->second)
        {
            // ch = a
            auto ch = prod.getIndexCharacter(0);
            // 首字符为终结符
            if(Vt.find(*ch) != Vt.end())
            {
                // A a abc
                this->m_table.push_back(LL1ITEM{key, *ch, prod});
                continue;
            }
            
            // 空产生式
            // A -> epsilon
            if(ch->equal(Epsilon))
            {
                // 产生两个空产生式
                if(hasEpsilonProd)
                {
                    cout << "the same left part has more than one epsilon production:: left part is \"" << key << "\"\n";
                    exit(1);
                }
                // 将A -> epsilon 添加至 A 与 A的follow集 组成的矩阵中
                for(auto& follow : this->m_FOLLOW[key].Elems())
                {
                    this->m_table.push_back(LL1ITEM{key, follow, prod});
                }
            }

            // 非终结符开头

            // A -> BCD 并且 B =*=> epsilon
            if(beta.find(*ch) != beta.end())
            {
                size_t index = 0;
                
                while(prod.getIndexCharacter(index) != nullptr)
                {
                    for(auto& first : this->m_FIRST[*prod.getIndexCharacter(index)].Elems())
                    {
                        if(first.equal(Epsilon))
                        {
                            continue;
                        }
                        this->m_table.push_back(LL1ITEM{key, first, prod});
                    }

                    // 确人 index 序号的非终结符可以推出空串
                    if(prod.getIndexCharacter(index+1) != nullptr && beta.find(*prod.getIndexCharacter(index)) != beta.end())
                    {       
                        index++;
                        // 终结符
                        if(this->Vt.find(*prod.getIndexCharacter(index)) != this->Vt.end())
                        {
                            this->m_table.push_back(LL1ITEM{key, *prod.getIndexCharacter(index), prod});
                            break;
                        }                
                        continue;
                    }
                    break;
                }

                // 说明产生式 对于 A =*=> epsilon 成立
                if(prod.getIndexCharacter(index) == nullptr)
                {
                    if(hasEpsilonProd)
                    {
                        cout << "the same left part has more than one epsilon production:: left part is \"" << key << "\"\n";
                        exit(1);                        
                    }
                    hasEpsilonProd = true;
                }
            }
            else
            {
                // A -> Bbc 并且 B =*=> epsilon 不成立
                for(auto& first : this->m_FIRST[*ch].Elems())
                {
                    this->m_table.push_back(LL1ITEM{key, first, prod});
                }
            }
            hasEpsilonProd = false;
        } // for (auto prod : keyprod->second)
    } // for(auto KeyProd = this->productions.begin(); KeyProd != this->productions.end(); KeyProd++)

}

// 分析产生式
void LL1::Analyze(production& in)
{
    stack<character> output;
    int scan_ptr = 0;
    
    // 添加边界
    production input = production(Border, Epsilon) + in + production(Border, Epsilon);

    // 压栈
    output.push(*input.getIndexCharacter(scan_ptr++));
    output.push(this->S);

    if(this->m_table.size() <= 0 || this->Vt.size() <= 0 || this->Vn.size() <= 0)
    {
        cout << "LL1 hasn't been init\n";
        return;
    }

    character ch;
    character top;
    while(true)
    {
        ch = *input.getIndexCharacter(scan_ptr);
        top = output.top();
   
        // 栈顶符号为终结符
        if(this->Vt.find(top) != this->Vt.end())
        {
            // 栈顶终结符和预期读入一致，弹出
            if(top.equal(ch))
            {
                cout << "output stack pop V = \"" << top << "\" and current scan pointer(\""<< *input.getIndexCharacter(scan_ptr) <<"\")  move to next(\"" << *input.getIndexCharacter(scan_ptr+1) << "\")\n";
                scan_ptr++;
                output.pop();
                continue;
            }
            cout << "syntax error\n";
            return;
        }

        else if(ch.equal(top) && ch.equal(Border))
        {
            cout << "acc\n";
            goto Exit;
        }

        // 栈顶为非终结符
        else if(this->Vn.find(top) != this->Vn.end())
        {
            for(auto v : this->m_table)
            {
                if(v.vn.equal(top) && v.vt.equal(ch))
                {
                    // 空串就弹出栈顶
                    if (v.prod.equal(EpsilonProduction)) {
                        cout << "output stack pop V = \"" << top << "\"\n";
                        output.pop();
                        goto Continue;
                    }

                    // 若长度不为空
                    output.pop();
                    int len = v.prod.size()-1;
                    cout << "output stack pop V = \"" << top << "\" and push V = \"";
                    while (len >= 0) {
                        cout << (*v.prod.getIndexCharacter(len)) << (len == 0 ? "\"" : " ");
                        output.push(*v.prod.getIndexCharacter(len--));
                    }
                    cout << endl;
                    goto Continue;                
                }
            }
            goto SyntaxErr;
        } // if(this->Vn.find(top) != this->Vn.end())
        else
        {
        SyntaxErr:    
            cout << "syntax error:: can't find a match\n";
            return;            
        }

        Continue:
            continue;
    }
    Exit:
        cout << "分析结束\n";
}

void LL1::AnalyzeGrammar()
{
    // 获取FIRST集合
    this->FIRST();
    this->print(cout, this->m_FIRST, "FIRST: \n");
    // 获取FOLLOW
    this->FOLLOW();
    this->print(cout, this->m_FOLLOW, "FOLLOW: \n");
    // 获取表
    this->LL1Table();
    this->print(cout << "ll1 表格：\n");
}

// 打印表格
std::ostream& LL1::print(std::ostream& os)
{
    os << "栈顶符号" << '\t' << "预读字符\t使用的产生式\n";
    for(auto& s : this->m_table)
    {  
        os << s.vn << '\t' << s.vt << '\t' << s.prod << endl;
    }
}

// 打印FIRST、FOLLOW集
std::ostream& LL1::print(std::ostream& os, const std::map<character, store>& sets, const char* title)
{
    if(title != nullptr)
    {
        os << title;
    }

    for(auto& keyChar : sets)
    {
        auto& key = keyChar.first;
        os << key << ": { ";
        for(auto& ch : keyChar.second.Elems())
        {
            os << ch << " ";
        }
        os << "}\n";
    }
}