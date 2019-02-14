#include "toml/toml.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#if defined(_MSC_VER)
#define timegm _mkgmtime
#elif defined(_WIN32)
// Because timegm is defined in toml namespace...
using toml::timegm;
#endif

using namespace std;

namespace {

toml::Value parse(const string& name)
{
    string filename = string(TESTCASE_DIR) + "/success/" + name + ".toml";

    ifstream ifs(filename);
    EXPECT_TRUE(ifs.good());

    toml::internal::Parser p(ifs);
    toml::Value v = p.parse();
    EXPECT_TRUE(v.valid());

    return v;
}

std::chrono::system_clock::time_point makeTimePoint(int year, int month, int mday, int hour, int min, int sec, int usec)
{
    std::tm t;
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = false;
    auto tp = std::chrono::system_clock::from_time_t(timegm(&t));
    tp += std::chrono::microseconds(usec);

    return tp;
}

} // namespace anonymous

TEST(ParserComplexTest, boolean)
{
    toml::Value v = parse("boolean-01");

    EXPECT_TRUE(v.get<bool>("bool1"));
    EXPECT_TRUE(v.get<bool>("true"));
    EXPECT_FALSE(v.get<bool>("bool2"));
    EXPECT_FALSE(v.get<bool>("false"));
}

TEST(ParserComplexTest, integer)
{
    toml::Value v = parse("integer-01");

    EXPECT_EQ(99, v.get<int>("int1"));
    EXPECT_EQ(42, v.get<int>("int2"));
    EXPECT_EQ(0, v.get<int>("int3"));
    EXPECT_EQ(-17, v.get<int>("int4"));
    EXPECT_EQ(1000, v.get<int>("int5"));
    EXPECT_EQ(5349221, v.get<int>("int6"));
    EXPECT_EQ(12345, v.get<int>("int7"));
}

TEST(ParserComplexTest, float)
{
    toml::Value v = parse("float-01");

    EXPECT_EQ(1.0, v.get<double>("flt1"));
    EXPECT_EQ(3.1415, v.get<double>("flt2"));
    EXPECT_EQ(-0.01, v.get<double>("flt3"));
    EXPECT_EQ(5e+22, v.get<double>("flt4"));
    EXPECT_EQ(1e6, v.get<double>("flt5"));
    EXPECT_EQ(-2E-2, v.get<double>("flt6"));
    EXPECT_EQ(6.626e-34, v.get<double>("flt7"));
    EXPECT_EQ(9224617.445991228313, v.get<double>("flt8"));

    // 1e+1000 exceeds double precision.
    // EXPECT_EQ(1e1000, v.get<double>("flt9"));
}

TEST(ParserComplexTest, string)
{
    toml::Value v = parse("string-01");

    // U+00E9 = C3A9 in UTF8. https://codepoints.net/U+00E9
    EXPECT_EQ("I'm a string. \"You can quote me\". Name\tJos\xC3\xA9\nLocation\tSF.", v.get<string>("s1"));
    EXPECT_EQ("Roses are red\nViolets are blue", v.get<string>("s2"));

    EXPECT_EQ("The quick brown fox jumps over the lazy dog.", v.get<string>("str1"));
    EXPECT_EQ("The quick brown fox jumps over the lazy dog.", v.get<string>("str2"));
    EXPECT_EQ("The quick brown fox jumps over the lazy dog.", v.get<string>("str3"));

    EXPECT_EQ("C:\\Users\\nodejs\\templates", v.get<string>("winpath"));
    EXPECT_EQ("\\\\ServerX\\admin$\\system32\\", v.get<string>("winpath2"));
    EXPECT_EQ("Tom \"Dubs\" Preston-Werner", v.get<string>("quoted"));
    EXPECT_EQ("<\\i\\c*\\s*>", v.get<string>("regex"));
    EXPECT_EQ("I [dw]on't need \\d{2} apples", v.get<string>("regex2"));
    EXPECT_EQ("The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n", v.get<string>("lines"));
}

TEST(ParserComplexTest, datetime1)
{
    toml::Value v = parse("datetime-01");

    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 0), v.get<toml::Time>("date1"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 0), v.get<toml::Time>("date2"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 999999), v.get<toml::Time>("date3"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 0, 0, 0, 0), v.get<toml::Time>("date4"));
}

TEST(ParserComplexTest, datetime2)
{
    toml::Value v = parse("datetime-02");

    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 0), v.get<toml::Time>("date0"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 100000), v.get<toml::Time>("date1"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 120000), v.get<toml::Time>("date2"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 123000), v.get<toml::Time>("date3"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 123400), v.get<toml::Time>("date4"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 123450), v.get<toml::Time>("date5"));
    EXPECT_EQ(makeTimePoint(1979, 5, 27, 7, 32, 0, 123456), v.get<toml::Time>("date6"));
}

TEST(ParserComplexTest, array)
{
    toml::Value v = parse("array-01");

    EXPECT_EQ(1, v.get<toml::Array>("arr1")[0].as<int>());
    EXPECT_EQ("yellow", v.get<toml::Array>("arr2")[1].as<string>());
    EXPECT_EQ(5, v.get<toml::Array>("arr3")[1].get<int>(2));
    EXPECT_EQ("c", v.get<toml::Array>("arr5")[1].get<string>(2));
}

TEST(ParserComplexTest, table)
{
    toml::Value v = parse("table-01");

    EXPECT_EQ("value", v.get<string>("table.key"));
    EXPECT_EQ("value", v.get<string>("table.bare_key"));
    EXPECT_EQ("value", v.get<string>("table.bare-key"));
    EXPECT_EQ("bare integer", v.get<string>("table.1234"));

    EXPECT_EQ("value", v.get<string>("table.\"127.0.0.1\""));
    EXPECT_EQ("value", v.get<string>("table.\"character encoding\""));
#if !defined(_MSC_VER)
    EXPECT_EQ("value", v.get<string>("table.\"ʎǝʞ\""));
#endif
}

TEST(ParserComplexTest, table_02)
{
    toml::Value v = parse("table-02");

    EXPECT_EQ("pug", v.get<string>("table.\"tater.man\".type"));
    EXPECT_EQ(1, v.get<int>("a.b.c.x"));
    EXPECT_EQ(1, v.get<int>("d.e.f.x"));
    EXPECT_EQ(1, v.get<int>("g.h.i.x"));
#if !defined(_MSC_VER)
    EXPECT_EQ(1, v.get<int>("j.\"ʞ\".l.x"));
#endif
}

TEST(ParserComplexTest, table_03)
{
    toml::Value v = parse("table-03");

    EXPECT_EQ(1, v.get<toml::Array>("a")[0].get<int>("b.x"));
    EXPECT_EQ(2, v.get<toml::Array>("a")[1].get<int>("b.x"));
}

TEST(ParserComplexTest, inlinetable_01)
{
    toml::Value v = parse("inlinetable-01");

    EXPECT_EQ("Tom", v.get<string>("name.first"));
    EXPECT_EQ("Preston-Werner", v.get<string>("name.last"));
    EXPECT_EQ(1, v.get<int>("point.x"));
    EXPECT_EQ(2, v.get<int>("point.y"));
}

TEST(ParserComplexTest, array_table_01)
{
    toml::Value v = parse("array-table-01");

    {
        const toml::Value& firstWidgets = v.get<toml::Array>("widgets")[0];

        EXPECT_EQ("image", firstWidgets.get<string>("type"));
        EXPECT_EQ(1000, firstWidgets.get<int>("width"));

        const toml::Value& firstTopics = firstWidgets.get<toml::Array>("topics")[0];

        EXPECT_EQ("some", firstTopics.get<string>("topic"));
        EXPECT_EQ(3, firstTopics.get<int>("count"));

        const toml::Value& secondTopics = firstWidgets.get<toml::Array>("topics")[1];

        EXPECT_EQ("something", secondTopics.get<string>("topic"));
        EXPECT_EQ(4, secondTopics.get<int>("count"));
    }

    {
        const toml::Value& secondWidgets = v.get<toml::Array>("widgets")[1];

        EXPECT_EQ("foo", secondWidgets.get<string>("type"));
        EXPECT_EQ(2000, secondWidgets.get<int>("width"));

        const toml::Value& firstTopics = secondWidgets.get<toml::Array>("topics")[0];

        EXPECT_EQ("bar", firstTopics.get<string>("topic"));
        EXPECT_EQ(5, firstTopics.get<int>("count"));
    }
}

TEST(ParserComplexTest, array_table_02)
{
    toml::Value v = parse("array-table-02");

    const toml::Value& widgets = v.get<toml::Table>("widgets");

    EXPECT_EQ("image", widgets.get<string>("type"));
    EXPECT_EQ(1000, widgets.get<int>("width"));

    const toml::Value& firstTopics = widgets.get<toml::Array>("topics")[0];

    EXPECT_EQ("some", firstTopics.get<string>("topic"));
    EXPECT_EQ(3, firstTopics.get<int>("count"));

    const toml::Value& secondTopics = widgets.get<toml::Array>("topics")[1];

    EXPECT_EQ("something", secondTopics.get<string>("topic"));
    EXPECT_EQ(4, secondTopics.get<int>("count"));
}

TEST(ParserComplexTest, with_utf8_bom)
{
    toml::Value v = parse("with-utf8-bom");

    const toml::Value& root = v.get<toml::Table>("I");
    EXPECT_EQ("hate utf8 BOM", root.get<string>("really"));
}
