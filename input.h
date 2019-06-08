#include <iostream>
#include "character.h"
#include <map>
#include <set>

#ifndef _INPUT_H_
#define _INPUT_H_
class grammar;
class input
{
private:
    std::istream& cin;
    std::set<character> Vn;
    std::set<character> Vt;
    character S;
    std::map<character, std::set<production>> P;

public:
    input(std::istream& in);
    // 输入需要解析的表达式, 输入空表达式退出
    production CreateProduction();
public:
    void In(grammar& g);
};

#endif