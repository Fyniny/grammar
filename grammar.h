#include "character.h"
#include <set>
#include <map>
#include <ostream>

#ifndef GRAMMAR_H
#define GRAMMAR_H

// 使用set的原因是因为它不在集合中不重复
// 语法分析---完成消递归 消回溯 消空产生式 消不可达产生式
class grammar {
private:
public:
    // 终结符
    std::set<character> Vt;

    // 非终结符
    std::set<character> Vn;

    // 文法开始符
    character S;

    // 产生式集
    std::map<character, std::set<production>> P;

public:
    // 语法分析
    void AnalyzeGrammar();

    // 输出文法四要素
    std::ostream& print(std::ostream& os, std::map<character, std::set<production>> *P = nullptr);

    // 定义对语法分析中对语法成分的分析处理
public:
    // 消除epsilon产生式
    // @param productions 是一个产生式的集合， key是产生式的左部， value是相同左部的所有右部的集合
    void RemoveEpsilon(std::map<character, std::set<production>>& productions);

    // 化间接左递归为直接左递归
    // 要求产生式无epsilon产生式 无回路(A->A)
    // 若出现上诉情况将导致结果不准确
    // @param Vn 非终结符集
    // @param productions 是一个产生式的集合， key是产生式的左部， value是相同左部的所有右部的集合, 是传出参数
    void IndirectRecursiveToDirectRecursive(std::set<character>& Vn, std::map<character, std::set<production>>& productions);

    // 消除回溯
    // @param productionMap 待处理的产生式集，并作为结果返回，是一个传出参数
    void RemoveBacktrack(std::map<character, std::set<production>> *productionMap);

    // 消除直接左递归
    // 为安全起见，调用之前建议先调用消除间接左递归
    // @param productionMap 待处理的产生式集，并作为结果返回，是一个传出参数
    bool RemoveRecursive(std::map<character, std::set<production>> *productionMap);

    // 消除不可达表达式
    void RemoveUnReachableProduction();

private:
    // 消除回溯
    // 这是一个内部处理函数，调用着应该调用RemoveBacktrack()
    // @param originLeft 是产生式的左部
    // @param ptrProductions 是所有相同左部的产生式的右部集合
    // @param productionMap 产生式的映射关系表
    void removeBacktrack(const character* originLeft, std::set<production> *ptrProductions, std::map<character, std::set<production>>* const productionMap);
    
    // 合并重复的无意义的表达式
    // A -> Ac  D -> Dc 则合并为 A -> Ac
    void MergeRepeatProduction();

private:
    // 申请新的非终结符号
    bool AllocVn(const character src, character& ch);
    // 释放非终结符号
    void ReleaseVn(const character& vn);

public:
    // 返回私有变量的常值
    const std::set<character>& VN() const;

    const std::set<character>& VT() const;

    const std::map<character, std::set<production>>& Prodtions() const;

    const character& Start() const;

    //初始化
    void Init(const std::set<character>& Vn, const std::set<character>& Vt, const character& S, const std::map<character, std::set<production>>& P);
};

#endif