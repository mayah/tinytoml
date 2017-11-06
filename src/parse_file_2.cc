#include "toml/toml.h"

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Usage:" << endl
             << "  " << argv[0] << " <input-file>" << endl;
        return 1;
    }

    ifstream ifs(argv[1]);
    // Pass ifs without checking error (to test toml::parse detects and error).
    // In usual case, I recommend to check ifs state.
    toml::ParseResult pr = toml::parse(ifs);
    if (pr.valid()) {
        cout << pr.value;
    } else {
        cout << pr.errorReason << endl;
    }
}
