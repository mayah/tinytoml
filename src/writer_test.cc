#include <fstream>
#include <sstream>
#include <iostream>
#include <gtest/gtest.h>

#include "toml/toml.h"
#include "build.h"

using namespace std;

namespace {

void write_parse_compare(toml::Value& value)
{
    // Write value to stringstream
    stringstream write_ss;
    value.write(&write_ss);

    // Parse write_ss
    toml::ParseResult pr = toml::parse(write_ss);
    ASSERT_TRUE(pr.valid()) << pr.errorReason << write_ss.str();

    // Compare parsed with value
    toml::Value& parsed = pr.value;
    EXPECT_EQ(parsed, value) << "parsed=" << parsed << " value=" << value;
}

} // namespace anonymous

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

TEST(WriterTest, DISABLED_write_parse_datetime_01)
{
    toml::Value root = build_datetime_01();
    write_parse_compare(root);
}

// XXX: flt8 generates a comparison error (large floating point...)
// TODO: Uncomment if fix-able.
TEST(WriterTest, DISABLED_write_parse_float_01)
{
    toml::Value root = build_float_01();
    write_parse_compare(root);
}

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

TEST(WriterTest, write_table_keys)
{
    struct {
        const char* key;
        const char* expect;
    } TEST_CASES[] = {
        {"bare_key", "bare_key"} ,
        {"barekey", "barekey"},
        {"bare key", "\"bare key\""},
        {"bare-key", "bare-key"},
        {"bareKey", "bareKey"},
    };

    for (auto testCase : TEST_CASES) {
        toml::Value root((toml::Table()));
        root.setChild(testCase.key, "value");

        ostringstream oss;
        root.write(&oss);
        string expect = string(testCase.expect) + " = \"value\"\n";
        EXPECT_EQ(expect, oss.str());
    }
}

TEST(WriterTest, write_parse_nested_array_of_tables)
{
    // {"A": [[{"value": 1}]]}
    toml::Value object((toml::Table()));
    object.setChild("value", 1);
    toml::Array inner;
    inner.push_back(object);
    toml::Array outer;
    outer.push_back(inner);
    toml::Value root((toml::Table()));
    root.setChild("A", outer);

    ostringstream oss;
    root.write(&oss);
    EXPECT_EQ("A = [[{value = 1}]]\n", oss.str());

    write_parse_compare(root);
}

TEST(WriterTest, write_parse_nested_array_of_nested_tables)
{
    // {"A": [[{"x": {"y": 1}, "z": [{"w": "s"}]}, {}]]}
    toml::Value object((toml::Table()));
    object.set("x.y", 1);
    toml::Value w((toml::Table()));
    w.setChild("w", "s");
    toml::Array z;
    z.push_back(w);
    object.setChild("z", z);
    toml::Array inner;
    inner.push_back(object);
    inner.push_back(toml::Table());
    toml::Array outer;
    outer.push_back(inner);
    toml::Value root((toml::Table()));
    root.setChild("A", outer);

    ostringstream oss;
    root.write(&oss);
    EXPECT_EQ("A = [[{x = {y = 1}, z = [{w = \"s\"}]}, {}]]\n", oss.str());

    write_parse_compare(root);
}
