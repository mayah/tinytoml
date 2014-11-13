#include "toml/toml.h"

#include <iostream>
using namespace std;

int main(void)
{
    toml::Parser p(std::cin);
    toml::Value v = p.parse();

    if (!v.valid())
        cout << p.errorReason();
    else
        cout << v;
}
