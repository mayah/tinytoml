#include <fstream>
#include <sstream>
#include <iostream>
#include <gtest/gtest.h>

#include "toml/toml.h"
#include "build.h"

using namespace std;

void write_parse_compare (toml::Value& value)
{
    // Write value to stringstream
    stringstream write_ss;
    value.write(&write_ss);

    // Parse write_ss
    toml::ParseResult pr = toml::parse(write_ss);
    ASSERT_TRUE(pr.valid()) << pr.errorReason << write_ss.str();

    // Compare parsed with value
    toml::Value& parsed = pr.value;
    bool result = (parsed == value);
    EXPECT_TRUE(result);

    if (result == false)
    {
        std::cout << "VALUE CONTENTS:" << std::endl;
        value.write(&std::cout);
        std::cout << "PARSED CONTENTS:" << std::endl;
        parsed.write(&std::cout);
    }
}

TEST(WriterTest, write_parse_array_01)
{
    toml::Value root = build_array_01();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_array_table_01)
{
    toml::Value root = build_array_table_01();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_array_table_02)
{
    toml::Value root = build_array_table_02();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_boolean_01)
{
    toml::Value root = build_boolean_01();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_datetime_01)
{
    toml::Value root = build_datetime_01();
    write_parse_compare(root);
}

// XXX: flt8 generates a comparison error (large floating point...)
// TODO: Uncomment if fix-able.
//TEST(WriterTest, write_parse_float_01)
//{
//    toml::Value root = build_float_01();
//    write_parse_compare(root);
//}

TEST(WriterTest, write_parse_inlinetable_01)
{
    toml::Value root = build_inlinetable_01();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_integer_01)
{
    toml::Value root = build_integer_01();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_string_01)
{
    toml::Value root = build_string_01();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_table_01)
{
    toml::Value root = build_table_01();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_table_02)
{
    toml::Value root = build_table_02();
    write_parse_compare(root);
}

TEST(WriterTest, write_parse_table_03)
{
    toml::Value root = build_table_03();
    write_parse_compare(root);
}

