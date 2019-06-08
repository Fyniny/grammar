#include "input.h"
#include <iostream>
#include <map>
#include <set>
#include <queue>
#include "grammar.h"

using namespace std;

input::input(std::istream& in) : cin(in)
{} 

void input::In(grammar& g)
{
    std::ostream &os = std::cout;
    characterType ch;
    os << "input G(S) grammar:(input \"#\" to stop)\n";
    os << "input Vt:\n";
    while (true)
    {
        cin >> ch;
        if (ch == "#")
        {
            os << "stop input Vt\n";
            break;
        }
        this->Vt.insert(character(ch));
    }

    os << "input Vn:\n";
    while (true)
    {
        cin >> ch;
        if (ch == "#")
        {
            os << "stop input Vn\n";
            break;
        }
        this->Vn.insert(character(ch));
    }

    os << "input S(Note that S must in Vn):\n";
    while (true)
    {
        cin >> ch;
        if (this->Vn.find(ch) == this->Vn.end())
        {
            os << "S('" << ch << "') is invalid::not in Vn\n";
            continue;
        }
        this->S = ch;
        break;
    }

    os << "input Productions(start and stop a production with '#', use space ' ' to split token, use '##' to stop inputing productions)\n";
    os << "input a prodution like this '# E $$ A B C #'\n";

    // std::vector<character> prod;
    production prod;
    std::queue<character> q;

    while (true)
    {
        cin >> ch;
        if (ch == "##")
        {
            os << "stop input production\n";
            break;
        }

        if (ch == "#")
        {
            cin >> ch;
            while (!(ch == "#" || ch == "##"))
            {
                q.push(character(ch));
                cin >> ch;
            }
            if (ch == "#")
            {
                production p;
                // 分析输入式子
                
                // 左部不为非终结符
                character left = q.front();
                if(this->Vn.find(left) == this->Vn.end())
                {
                    os << "production left must be in vn\n";
                    while (q.size() != 0)
                    {
                        q.pop();
                    }
                }
                q.pop();
                if (!q.front().equal(character("$$")))
                {
                    os << "production format failed\n";
                    while (q.size() != 0)
                    {
                        q.pop();
                    }
                }
                else
                {
                    q.pop();
                    p.assignLeft(left);
                }

                while (q.size() != 0)
                {
                    character ch = q.front();
                    if (this->Vn.find(ch) == this->Vn.end() && this->Vt.find(ch) == this->Vt.end())
                    {
                        if(ch.equal(Epsilon) && q.size() == 1)
                        {
                            p.append(ch);
                            q.pop();
                            continue;
                        }
                        os << "character " << ch << " isn't in vn or vt\n";
                        while (q.size() != 0)
                        {
                            q.pop();
                        }
                        p.clear();
                        break;
                    }
                    p.append(ch);
                    q.pop();
                }

                if (p.size() > 0)
                {
                    this->P[left].insert(p);
                    p.clear();
                }
            }
            if (ch == "##")
            {
                os << "expected receive '#', but get '##', cancel last input and stop input productions\n";
                break;
            }
        } // if (ch == "#")
    }     // while(true)
    
    g.Init(this->Vn, this->Vt, this->S, this->P);
}

// 输入需要解析的表达式, 输入空表达式退出
production input::CreateProduction()
{
    cout << "input production stop with '#' and exit with '$'\n";
    characterType ch;
    cin >> ch;
    if(ch == "$")
    {
        return EpsilonProduction;
    }

    while(ch != "#")
    {
        cout << "input error\n";
        cin >> ch;
    }
    production p;
    cin >> ch;
    while(ch != "#")
    {
        p.append(ch);
        cin >> ch;
    }
    return p;
}