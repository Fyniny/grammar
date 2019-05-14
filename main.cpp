#include "grammar.h"
#include <iostream>

using namespace std;

int main() {
    Grammar g;
    g.RemoveEmptyExpression();
    g.ExtractLeftRecursion();
    return 0;
}