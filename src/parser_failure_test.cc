#include "toml/toml.h"

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <dirent.h>
#endif

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

using namespace std;

namespace {

bool listFiles(const string& path, vector<string>* files)
{
#if defined(_MSC_VER)
    WIN32_FIND_DATA fileData;
    HANDLE hFind = FindFirstFile(path.c_str(), &fileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    while (true) {
        files->push_back(fileData.cFileName);
        if (!FindNextFile(hFind, &fileData))
            break;
    }
    if (FindClose(hFind))
        return false;
#else
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return false;

    while (true) {
        struct dirent* dent = readdir(dir);
        if (!dent)
            break;
        files->push_back(dent->d_name);
    }

    if (closedir(dir) < 0)
        return false;
#endif

    return true;
}

bool hasSuffix(const string& s, const string& suffix)
{
  if (s.size() < suffix.size())
    return false;

  return s.substr(s.size() - suffix.size()) == suffix;
}

vector<string> listFailureTestcases()
{
    const string dir = string(TESTCASE_DIR) + "/fail";
    vector<string> files;
    if (!listFiles(dir, &files))
        return vector<string>();

    vector<string> testcases;
    for (const auto& f : files) {
        if (hasSuffix(f, ".toml"))
            testcases.push_back(dir + "/" + f);
    }

    sort(testcases.begin(), testcases.end());

    return testcases;
}

} // namespace anonymous

class ParserFailureTest : public testing::TestWithParam<string> {};

TEST_P(ParserFailureTest, parseShouldFail)
{
    const string& filename = GetParam();

    ifstream ifs(filename);
    ASSERT_TRUE(ifs.good());

    toml::internal::Parser p(ifs);

    toml::Value v = p.parse();
    EXPECT_FALSE(v.valid()) << v.type()
                            << p.errorReason();
}

INSTANTIATE_TEST_CASE_P(
    ParserFailureTestCases,
    ParserFailureTest,
    testing::ValuesIn(listFailureTestcases()));

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
