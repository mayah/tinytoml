#include "toml/toml.h"

#include <iostream>
using namespace std;

int main(void)
{
    toml::ParseResult pr = toml::parse(std::cin);
    if (pr.valid()) {
        cout << pr.value;
    } else {
        cout << pr.errorReason << endl;
    }

    return 0;
}
