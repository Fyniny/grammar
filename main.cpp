#include "character.h"
#include <set>
#include "grammar.h"
#include "LL1.h"
#include <iostream>
#include "input.h"
using namespace std;


int main()
{
    input in(cin);
    grammar g;
    // 输入文法四要素

// 模拟输入
/*
    string Vn[] = {
        "E", "T", "R", "F", "Q"
    };

    string Vt[] = {
        "+", "*", "(", ")", "i"
    };

    g.S = character("E");
    g.P[character("E")].insert(production(character("T"), character("E")).append(character("R")));
    g.P[character("R")].insert(production(character("+"), character("R")).append(character("T")).append(character("R")));    
    g.P[character("R")].insert(production(character("$"), character("R")));
    g.P[character("T")].insert(production(character("F"), character("T")).append(character("Q")));
    g.P[character("Q")].insert(production(character("*"), character("Q")).append(character("F")).append(character("Q")));
    g.P[character("Q")].insert(production(character("$"), character("Q")));
    g.P[character("F")].insert(production(character("("), character("F")).append(character("E")).append(character(")")));
    g.P[character("F")].insert(production(character("i"), character("F")));
    // g.P[character("E")].insert(production(character("T"), character("E")).append(character("R")));

    for(auto v : Vn)
    {
        g.Vn.insert(v);
    }

    for(auto v : Vt)
    {
        g.Vt.insert(v);
    }
*/
    
    string Vn[] = {
        "S", "Q", "R"
    };

    string Vt[] = {
        "c", "b", "a"
    };

    g.S = character("S");
    g.P[character("S")].insert(production(character("Q"), character("S")).append(character("c")));
    g.P[character("S")].insert(production(character("c"), character("S")));    
    g.P[character("Q")].insert(production(character("R"), character("Q")).append(character("b")));
    g.P[character("Q")].insert(production(character("b"), character("Q")));
    g.P[character("R")].insert(production(character("S"), character("R")).append(character("a")));
    g.P[character("R")].insert(production(character("a"), character("R")));

    for(auto v : Vn)
    {
        g.Vn.insert(v);
    }

    for(auto v : Vt)
    {
        g.Vt.insert(v);
    }    
    
    
    
    
    // in.In(g);



    // 改造文法为LL1文法
    g.AnalyzeGrammar();
    // 解析LL1文法并获取ll1表格
    LL1 l(g);
    l.AnalyzeGrammar();

    production p;
    // 分析输入串
    while(!(p = in.CreateProduction()).equal(EpsilonProduction))
    {
        l.Analyze(p);
    }
}