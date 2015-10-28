#include "toml/toml.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

using namespace std;

namespace {

toml::Value parse(const string& name)
{
    string filename = string(TESTCASE_DIR) + "/success/" + name + ".toml";

    ifstream ifs(filename);
    EXPECT_TRUE(ifs.good());

    toml::Parser p(ifs);
    toml::Value v = p.parse();
    EXPECT_TRUE(v.valid());

    return v;
}

} // namespace anonymous

TEST(ParserComplexTest, array)
{
    toml::Value v = parse("array-01");

    toml::Array a = v.find("x")->as<toml::Array>();
    EXPECT_EQ(5U, a.size());
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(i, a[i].as<int>());
    }
}
