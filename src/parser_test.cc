#include "toml/toml.h"

#include <sstream>
#include <string>
#include <gtest/gtest.h>

using namespace std;

#if defined(_MSC_VER)
using toml::timegm;
#endif

static toml::Value parse(const std::string& s)
{
    stringstream ss(s);
    toml::internal::Parser p(ss);

    toml::Value v = p.parse();
    EXPECT_TRUE(v.valid()) << p.errorReason();
    return v;
}

TEST(ParserTest, parseEmpty)
{
    toml::Value v = parse("");

    EXPECT_TRUE(v.is<toml::Table>());
    EXPECT_EQ(0UL, v.size());
}

TEST(ParserTest, parseComment)
{
    toml::Value v = parse(
        "# hogehoge\n"
        "# fuga hoge\n");

    EXPECT_TRUE(v.is<toml::Table>());
    EXPECT_EQ(0UL, v.size());
}

TEST(ParserTest, parseCommentAndEmptyLine)
{
    toml::Value v = parse(
        "# hogehoge\n"
        "# fuga hoge\n"
        "\n"
        "\n"
        "# piyo piyo\n");

    EXPECT_TRUE(v.is<toml::Table>());
    EXPECT_EQ(0UL, v.size());
}

TEST(ParserTest, parseBool)
{
    toml::Value v = parse(
        "x = true\n"
        "y = false\n");

    EXPECT_TRUE(v.get<bool>("x"));
    EXPECT_FALSE(v.get<bool>("y"));
}

TEST(ParserTest, parseInt)
{
    toml::Value v = parse(
        "x = 1\n"
        "y = 0\n"
        "z = -1\n");

    EXPECT_EQ(1, v.get<int>("x"));
    EXPECT_EQ(0, v.get<int>("y"));
    EXPECT_EQ(-1, v.get<int>("z"));
}

TEST(ParserTest, parseIntWithDelimiter)
{
    toml::Value v = parse(
        "x =  100_000\n"
        "y = +100_000\n"
        "z = -100_000\n");

    EXPECT_EQ(100000, v.get<int>("x"));
    EXPECT_EQ(100000, v.get<int>("y"));
    EXPECT_EQ(-100000, v.get<int>("z"));
}

TEST(ParserTest, parseDouble)
{
    toml::Value v = parse(
        "x = 1.0\n"
        "y = -1.0\n"
        "z = -.5\n");

    EXPECT_EQ(1.0, v.get<double>("x"));
    EXPECT_EQ(-1.0, v.get<double>("y"));
    EXPECT_EQ(-0.5, v.get<double>("z"));
}

TEST(ParserTest, parseDoubleWithDelimiter)
{
    toml::Value v = parse(
        "x =  100_000.000_1\n"
        "y = +100_000.000_1\n"
        "z = -100_000.000_1\n");

    EXPECT_EQ( 100000.0001, v.get<double>("x"));
    EXPECT_EQ( 100000.0001, v.get<double>("y"));
    EXPECT_EQ(-100000.0001, v.get<double>("z"));
}

TEST(ParserTest, parseTime)
{
    toml::Value v = parse(
        "x = 1979-05-27T07:32:00Z\n");

    std::tm t;
    t.tm_year = 79;
    t.tm_mon = 4;
    t.tm_mday = 27;
    t.tm_hour = 7;
    t.tm_min = 32;
    t.tm_sec = 0;

    time_t tt = timegm(&t);

    EXPECT_EQ(tt, std::chrono::system_clock::to_time_t(v.get<toml::Time>("x")));
}

TEST(ParserTest, parseTimeWithTimeZone)
{
    toml::Value v = parse(
        "x = 1979-05-27T07:32:00+09:00\n");

    std::tm t;
    t.tm_year = 79;
    t.tm_mon = 4;
    t.tm_mday = 26;
    t.tm_hour = 22;
    t.tm_min = 32;
    t.tm_sec = 0;

    time_t tt = timegm(&t);

    EXPECT_EQ(tt, std::chrono::system_clock::to_time_t(v.get<toml::Time>("x")));
}

TEST(ParserTest, parseStringDoubleQuote)
{
    toml::Value v = parse(
        "x = \"hoge\"\n"
        "y = \"hoge \\\"fuga\\\" hoge\"\n"
        "z = \"\\u003F\\U0000003F\"");

    EXPECT_EQ("hoge", v.get<string>("x"));
    EXPECT_EQ("hoge \"fuga\" hoge", v.get<string>("y"));
    EXPECT_EQ("??", v.get<string>("z"));
}

TEST(ParserTest, parseStringDoubleQuoteMultiLine1)
{
    toml::Value v = parse(R"(
x = """
foo bar"""
y = """
foo bar \
     foo bar"""
)");

    EXPECT_EQ("foo bar", v.get<string>("x"));
    EXPECT_EQ("foo bar foo bar", v.get<string>("y"));
}

TEST(ParserTest, parseStringDoubleQuoteMultiLine2)
{
    toml::Value v = parse(R"(x = """foo bar""")");

    EXPECT_EQ("foo bar", v.get<string>("x"));
}

TEST(ParserTest, parseStringDoubleQuoteMultiLine3)
{
    toml::Value v = parse("x = \"\"\"foo\\nbar\"\"\"");

    EXPECT_EQ("foo\nbar", v.get<string>("x"));
}

TEST(ParserTest, parseStringSinleQuote)
{
    toml::Value v = parse(
        "x = 'foo bar \"foo bar\"'\n");

    EXPECT_EQ("foo bar \"foo bar\"", v.get<string>("x"));
}

TEST(ParserTest, parseStringSinleQuoteMultiLine)
{
    toml::Value v = parse(R"(
x = '''foo bar "foo bar"'''
y = '''
foo bar
foo bar'''
z = '''
foo bar \
   foo bar'''
)");

    EXPECT_EQ("foo bar \"foo bar\"", v.get<string>("x"));
    EXPECT_EQ("foo bar\nfoo bar", v.get<string>("y"));
    EXPECT_EQ("foo bar \\\n   foo bar", v.get<string>("z"));
}

TEST(ParserTest, parseArray)
{
    toml::Value v = parse(
        "x = [1, 2, 3]\n"
        "y = []\n"
        "z = [\"\", \"\", ]");

    const toml::Value& x = v.get<toml::Array>("x");
    EXPECT_EQ(3UL, x.size());
    EXPECT_EQ(1, x.get<int>(0));
    EXPECT_EQ(2, x.get<int>(1));
    EXPECT_EQ(3, x.get<int>(2));

    const toml::Value& y = v.get<toml::Array>("y");
    EXPECT_EQ(0UL, y.size());

    const toml::Value& z = v.get<toml::Array>("z");
    EXPECT_EQ(2UL, z.size());
    EXPECT_EQ("", z.get<string>(0));
    EXPECT_EQ("", z.get<string>(1));
}

TEST(ParserTest, parseInlineTable)
{
    toml::Value v = parse(R"(
x = {}
y = { a = 1, b = 2 })");

    EXPECT_TRUE(v.find("x") != nullptr);
    EXPECT_EQ(1, v.get<int>("y.a"));
    EXPECT_EQ(2, v.get<int>("y.b"));
}

TEST(ParserTest, parseTable)
{
    toml::Value v = parse(
        "[size]\n"
        "chihaya = 72\n"
        "miki = 84\n"
        "[piyo.piyo]\n"
        "puyo = 2\n"
        "[piyo.piyo.piyo]\n"
        "puyo = 3\n");

    EXPECT_EQ(72, v.get<int>("size.chihaya"));
    EXPECT_EQ(84, v.get<int>("size.miki"));

    EXPECT_EQ(2, v.get<int>("piyo.piyo.puyo"));
    EXPECT_EQ(3, v.get<int>("piyo.piyo.piyo.puyo"));
}

TEST(ParserTest, parseArrayTable)
{
    toml::Value v = parse(
        "[[kotori]]\n"
        "foo = 1\n"
        "[[kotori]]\n"
        "bar = 2\n");

    toml::Array ar = v.get<toml::Array>("kotori");
    EXPECT_EQ(2UL, ar.size());
    EXPECT_EQ(1, ar[0].get<int>("foo"));
    EXPECT_EQ(2, ar[1].get<int>("bar"));
}

TEST(ParserTest, commentAfterLine)
{
    toml::Value v = parse(
        "x = 1 # hogehoge");

    EXPECT_EQ(1, v.get<int>("x"));
}

TEST(ParserTest, commentInArray)
{
    toml::Value v = parse(
        "x = [ 0, # hogehoge\n"
        "1, 2, # hogehoge\n"
        "3 #hogehoge\n"
        ", ]");

    const toml::Array& ar = v.get<toml::Array>("x");
    EXPECT_EQ(4UL, ar.size());
    EXPECT_EQ(0, ar[0].as<int>());
    EXPECT_EQ(1, ar[1].as<int>());
    EXPECT_EQ(2, ar[2].as<int>());
    EXPECT_EQ(3, ar[3].as<int>());
}

TEST(ParserTest, parseNumberKey)
{
    toml::Value v = parse(
        "0000 = 1\n"
        "0001 = 2\n");

    EXPECT_EQ(1, v.get<int>("0000"));
    EXPECT_EQ(2, v.get<int>("0001"));
}
