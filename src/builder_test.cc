#include <fstream>
#include <sstream>
#include <iostream>
#include <gtest/gtest.h>

#include "toml/toml.h"
#include "build.h"

using namespace std;

namespace {

void compare_content_parsed_from_file(const char *name, toml::Value& value)
{
    string filepath = string(TESTCASE_DIR) + "/success/" + name + ".toml";;
    ifstream file(filepath, ifstream::in);
    ASSERT_TRUE(file.is_open());

    toml::ParseResult pr = toml::parse(file);
    ASSERT_TRUE(pr.valid()) << pr.errorReason << file.rdbuf();

    toml::Value& parsed = pr.value;
    EXPECT_EQ(parsed, value) << "parsed=" << parsed << " value=" << value;
}

} // namespace anonymous

TEST(BuildTest, build_parse_array_01)
{
    toml::Value root = build_array_01();
    compare_content_parsed_from_file("array-01", root);
}

TEST(BuildTest, build_parse_array_table_01)
{
    toml::Value root = build_array_table_01();
    compare_content_parsed_from_file("array-table-01", root);
}

TEST(BuildTest, build_parse_array_table_02)
{
    toml::Value root = build_array_table_02();
    compare_content_parsed_from_file("array-table-02", root);
}

TEST(BuildTest, build_parse_boolean_01)
{
    toml::Value root = build_boolean_01();
    compare_content_parsed_from_file("boolean-01", root);
}

TEST(BuildTest, build_parse_datetime_01)
{
    toml::Value root = build_datetime_01();
    compare_content_parsed_from_file("datetime-01", root);
}

// Unable to properly generate a float-01.toml (yet)
// TODO: Use test if build_float_01 is able to generate correct state.
TEST(BuildTest, DISABLED_build_parse_float_01)
{
    toml::Value root = build_float_01();
    compare_content_parsed_from_file("float-01", root);
}

TEST(BuildTest, build_parse_inlinetable_01)
{
    toml::Value root = build_inlinetable_01();
    compare_content_parsed_from_file("inlinetable-01", root);
}

TEST(BuildTest, build_parse_integer_01)
{
    toml::Value root = build_integer_01();
    compare_content_parsed_from_file("integer-01", root);
}

TEST(BuildTest, build_parse_string_01)
{
    toml::Value root = build_string_01();
    compare_content_parsed_from_file("string-01", root);
}

TEST(BuildTest, build_parse_table_01)
{
    toml::Value root = build_table_01();
    compare_content_parsed_from_file("table-01", root);
}

TEST(BuildTest, build_parse_table_02)
{
    toml::Value root = build_table_02();
    compare_content_parsed_from_file("table-02", root);
}

TEST(BuildTest, build_parse_table_03)
{
    toml::Value root = build_table_03();
    compare_content_parsed_from_file("table-03", root);
}
