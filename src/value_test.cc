#include "toml/toml.h"

#include <gtest/gtest.h>

#include <iostream>
using namespace std;

TEST(ValueTest, boolean)
{
    const toml::Value t(true);
    const toml::Value f(false);

    EXPECT_TRUE(t.is<bool>());
    EXPECT_TRUE(f.is<bool>());

    EXPECT_TRUE(t.as<bool>());
    EXPECT_FALSE(f.as<bool>());

    toml::Value v;
    v = true;
    EXPECT_TRUE(v.is<bool>());
    EXPECT_TRUE(v.as<bool>());

    v = false;
    EXPECT_TRUE(v.is<bool>());
    EXPECT_FALSE(v.as<bool>());

    toml::Value x = t;
    EXPECT_TRUE(x.is<bool>());
    EXPECT_TRUE(x.as<bool>());
}

TEST(ValueTest, int)
{
    const toml::Value zero(0);
    const toml::Value one(1);
    const toml::Value mone(-1);

    EXPECT_TRUE(zero.is<int>());
    EXPECT_TRUE(one.is<int>());
    EXPECT_TRUE(mone.is<int>());

    EXPECT_EQ(0, zero.as<int>());
    EXPECT_EQ(1, one.as<int>());
    EXPECT_EQ(-1, mone.as<int>());

    toml::Value v;
    v = 100;
    EXPECT_TRUE(v.is<int>());
    EXPECT_EQ(100, v.as<int>());
}

TEST(ValueTest, int64_t)
{
    const toml::Value zero(static_cast<int64_t>(0));
    const toml::Value one(static_cast<int64_t>(1));
    const toml::Value mone(static_cast<int64_t>(-1));

    EXPECT_TRUE(zero.is<int64_t>());
    EXPECT_TRUE(one.is<int64_t>());
    EXPECT_TRUE(mone.is<int64_t>());

    // We accept is<int>() also.
    EXPECT_TRUE(zero.is<int>());
    EXPECT_TRUE(one.is<int>());
    EXPECT_TRUE(mone.is<int>());

    EXPECT_EQ(0, zero.as<int64_t>());
    EXPECT_EQ(1, one.as<int64_t>());
    EXPECT_EQ(-1, mone.as<int64_t>());

    toml::Value v;
    v = static_cast<int64_t>(100);
    EXPECT_TRUE(v.is<int64_t>());
    EXPECT_EQ(100, v.as<int64_t>());
}

TEST(ValueTest, double)
{
    const toml::Value zero(0.0);
    const toml::Value one(1.0);
    const toml::Value mone(-1.0);

    EXPECT_TRUE(zero.is<double>());
    EXPECT_TRUE(one.is<double>());
    EXPECT_TRUE(mone.is<double>());

    EXPECT_EQ(0.0, zero.as<double>());
    EXPECT_EQ(1.0, one.as<double>());
    EXPECT_EQ(-1.0, mone.as<double>());

    toml::Value v;
    v = 100.0;
    EXPECT_TRUE(v.is<double>());
    EXPECT_EQ(100.0, v.as<double>());
}

TEST(ValueTest, doubleWrite)
{
    {
        const toml::Value v(1.0);
        ostringstream ss;
        v.write(&ss);
        EXPECT_EQ("1.000000", ss.str());
    }

    {
        const toml::Value v(10000000.0);
        ostringstream ss;
        v.write(&ss);;
        EXPECT_EQ("10000000.000000", ss.str());
    }

    {
        const toml::Value v(123456.789123);
        ostringstream ss;
        v.write(&ss);
        EXPECT_EQ("123456.789123", ss.str());
    }
}

TEST(ValueTest, string)
{
    toml::Value v1(string("foo"));
    EXPECT_TRUE(v1.is<string>());
    EXPECT_EQ("foo", v1.as<string>());

    v1 = "test";
    EXPECT_TRUE(v1.is<string>());
    EXPECT_EQ("test", v1.as<string>());

    v1 = string("kotori");
    EXPECT_TRUE(v1.is<string>());
    EXPECT_EQ("kotori", v1.as<string>());

    toml::Value v2("foo");
    EXPECT_TRUE(v2.is<string>());
    EXPECT_EQ("foo", v2.as<string>());

}

TEST(ValueTest, time)
{
    auto t = std::chrono::system_clock::now();
    toml::Value v1(t);

    EXPECT_TRUE(v1.is<toml::Time>());
}

TEST(ValueTest, time_as_time_t)
{
    auto t = std::chrono::system_clock::now();
    toml::Value v(t);

    EXPECT_EQ(std::chrono::system_clock::to_time_t(t), v.as_time_t());
}

TEST(ValueTest, bool_array)
{
    toml::Value v((toml::Array()));
    v.push(false);
    v.push(true);

    vector<bool> vs = v.as<vector<bool>>();
    EXPECT_EQ(2U, vs.size());
    EXPECT_FALSE(vs[0]);
    EXPECT_TRUE(vs[1]);

    EXPECT_TRUE(v.is<std::vector<bool>>());
    EXPECT_FALSE(v.is<std::vector<int>>());
    EXPECT_FALSE(v.is<std::vector<int64_t>>());
    EXPECT_FALSE(v.is<std::vector<double>>());
    EXPECT_FALSE(v.is<std::vector<string>>());
    EXPECT_FALSE(v.is<std::vector<toml::Time>>());
    EXPECT_FALSE(v.is<std::vector<toml::Array>>());
    EXPECT_FALSE(v.is<std::vector<toml::Table>>());
}

TEST(ValueTest, int_array)
{
    toml::Value v((toml::Array()));
    v.push(0);
    v.push(1);

    vector<int> vs = v.as<vector<int>>();
    EXPECT_EQ(2U, vs.size());
    EXPECT_EQ(0, vs[0]);
    EXPECT_EQ(1, vs[1]);

    vector<int64_t> vs2 = v.as<vector<int64_t>>();
    EXPECT_EQ(2U, vs2.size());
    EXPECT_EQ(0, vs2[0]);
    EXPECT_EQ(1, vs2[1]);

    EXPECT_FALSE(v.is<std::vector<bool>>());
    EXPECT_TRUE(v.is<std::vector<int>>());
    EXPECT_TRUE(v.is<std::vector<int64_t>>());
    EXPECT_FALSE(v.is<std::vector<double>>());
    EXPECT_FALSE(v.is<std::vector<string>>());
    EXPECT_FALSE(v.is<std::vector<toml::Time>>());
    EXPECT_FALSE(v.is<std::vector<toml::Array>>());
    EXPECT_FALSE(v.is<std::vector<toml::Table>>());
}

TEST(ValueTest, double_array)
{
    toml::Value v((toml::Array()));
    v.push(0.0);
    v.push(1.0);

    vector<double> vs = v.as<vector<double>>();
    EXPECT_EQ(2U, vs.size());
    EXPECT_EQ(0.0, vs[0]);
    EXPECT_EQ(1.0, vs[1]);

    EXPECT_FALSE(v.is<std::vector<bool>>());
    EXPECT_FALSE(v.is<std::vector<int>>());
    EXPECT_FALSE(v.is<std::vector<int64_t>>());
    EXPECT_TRUE(v.is<std::vector<double>>());
    EXPECT_FALSE(v.is<std::vector<string>>());
    EXPECT_FALSE(v.is<std::vector<toml::Time>>());
    EXPECT_FALSE(v.is<std::vector<toml::Array>>());
    EXPECT_FALSE(v.is<std::vector<toml::Table>>());
}

TEST(ValueTest, string_array)
{
    toml::Value v((toml::Array()));
    v.push("foo");
    v.push("bar");

    vector<string> vs = v.as<vector<std::string>>();
    EXPECT_EQ(2U, vs.size());
    EXPECT_EQ("foo", vs[0]);
    EXPECT_EQ("bar", vs[1]);

    EXPECT_FALSE(v.is<std::vector<bool>>());
    EXPECT_FALSE(v.is<std::vector<int>>());
    EXPECT_FALSE(v.is<std::vector<int64_t>>());
    EXPECT_FALSE(v.is<std::vector<double>>());
    EXPECT_TRUE(v.is<std::vector<string>>());
    EXPECT_FALSE(v.is<std::vector<toml::Time>>());
    EXPECT_FALSE(v.is<std::vector<toml::Array>>());
    EXPECT_FALSE(v.is<std::vector<toml::Table>>());
}

TEST(ValueTest, time_array)
{
    std::chrono::time_point<std::chrono::system_clock> t;

    toml::Value v((toml::Array()));
    v.push(t);

    vector<toml::Time> vs = v.as<vector<toml::Time>>();
    EXPECT_EQ(1U, vs.size());
    EXPECT_EQ(t, vs[0]);

    EXPECT_FALSE(v.is<std::vector<bool>>());
    EXPECT_FALSE(v.is<std::vector<int>>());
    EXPECT_FALSE(v.is<std::vector<int64_t>>());
    EXPECT_FALSE(v.is<std::vector<double>>());
    EXPECT_FALSE(v.is<std::vector<string>>());
    EXPECT_TRUE(v.is<std::vector<toml::Time>>());
    EXPECT_FALSE(v.is<std::vector<toml::Array>>());
    EXPECT_FALSE(v.is<std::vector<toml::Table>>());
}

TEST(ValueTest, array_array)
{
    toml::Value v((toml::Array()));
    v.push(v);

    vector<toml::Array> vs = v.as<vector<toml::Array>>();
    EXPECT_EQ(1U, vs.size());

    EXPECT_FALSE(v.is<std::vector<bool>>());
    EXPECT_FALSE(v.is<std::vector<int>>());
    EXPECT_FALSE(v.is<std::vector<int64_t>>());
    EXPECT_FALSE(v.is<std::vector<double>>());
    EXPECT_FALSE(v.is<std::vector<string>>());
    EXPECT_FALSE(v.is<std::vector<toml::Time>>());
    EXPECT_TRUE(v.is<std::vector<toml::Array>>());
    EXPECT_FALSE(v.is<std::vector<toml::Table>>());
}

TEST(ValueTest, table_array)
{
    toml::Value v((toml::Array()));
    v.push(toml::Table());

    vector<toml::Table> vs = v.as<vector<toml::Table>>();
    EXPECT_EQ(1U, vs.size());

    EXPECT_FALSE(v.is<std::vector<bool>>());
    EXPECT_FALSE(v.is<std::vector<int>>());
    EXPECT_FALSE(v.is<std::vector<int64_t>>());
    EXPECT_FALSE(v.is<std::vector<double>>());
    EXPECT_FALSE(v.is<std::vector<string>>());
    EXPECT_FALSE(v.is<std::vector<toml::Time>>());
    EXPECT_FALSE(v.is<std::vector<toml::Array>>());
    EXPECT_TRUE(v.is<std::vector<toml::Table>>());
}

TEST(ValueTest, arrayWrite)
{
    toml::Value v((toml::Array()));
    {
        ostringstream ss;
        v.write(&ss);
        EXPECT_EQ("[]", ss.str());
    }

    v.push(1);
    {
        ostringstream ss;
        v.write(&ss);
        EXPECT_EQ("[1]", ss.str());
    }

    v.push(2);
    {
        ostringstream ss;
        v.write(&ss);
        EXPECT_EQ("[1, 2]", ss.str());
    }
}

TEST(ValueTest, table)
{
    toml::Value v;

    // If we call set() to null value, the value will become table automatically.
    v.set("key1", 1);
    v.set("key2", 2);

    EXPECT_EQ(1, v.get<int>("key1"));
    EXPECT_EQ(2, v.get<int>("key2"));
}

TEST(ValueTest, table2)
{
    toml::Value v;

    v.set("key1.key2", 1);
    EXPECT_EQ(1, v.find("key1")->find("key2")->as<int>());
}

TEST(ValueTest, table3)
{
    toml::Value ary;
    ary.push(0);
    ary.push(1);
    ary.push(2);

    toml::Value v;
    v.set("key", ary);

    std::vector<int> vs = v.get<std::vector<int>>("key");
    EXPECT_EQ(3U, vs.size());
    EXPECT_EQ(0, vs[0]);
    EXPECT_EQ(1, vs[1]);
    EXPECT_EQ(2, vs[2]);
}

TEST(ValueTest, tableErase)
{
    toml::Value v;
    v.set("key1.key2", 1);

    EXPECT_TRUE(v.erase("key1.key2"));
    EXPECT_EQ(nullptr, v.find("key1.key2"));

    // key1 will exist.
    EXPECT_TRUE(v.has("key1"));
    v.erase("key1");

    EXPECT_FALSE(v.has("key1"));
}

TEST(ValueTest, number)
{
    toml::Value v(1);
    EXPECT_TRUE(v.isNumber());
    EXPECT_EQ(1.0, v.asNumber());

    v = 2.5;
    EXPECT_TRUE(v.isNumber());
    EXPECT_EQ(2.5, v.asNumber());

    v = false;
    EXPECT_FALSE(v.isNumber());
}

TEST(ValueTest, tableFind)
{
    toml::Value v;
    v.set("foo", 1);

    toml::Value* x = v.find("foo");
    ASSERT_TRUE(x != nullptr);
    *x = 2;

    EXPECT_EQ(2, v.find("foo")->as<int>());
}

TEST(ValueTest, tableHas)
{
    toml::Value v;
    v.set("foo", 1);
    EXPECT_TRUE(v.has("foo"));
    EXPECT_FALSE(v.has("bar"));
}

TEST(ValueTest, merge)
{
    toml::Value v1;
    toml::Value v2;

    v1.set("foo.foo", 1);
    v1.set("foo.bar", 2);
    v1.set("bar", 3);

    v2.set("foo.bar", 4);
    v2.set("foo.baz", 5);
    v2.set("bar", 6);

    EXPECT_TRUE(v1.merge(v2));

    EXPECT_EQ(6, v1.get<int>("bar"));
    EXPECT_EQ(1, v1.get<int>("foo.foo"));
    EXPECT_EQ(4, v1.get<int>("foo.bar"));
    EXPECT_EQ(5, v1.get<int>("foo.baz"));
}

TEST(ValueTest, arrayFind)
{
    toml::Value v;
    v.push(1);

    toml::Value* x = v.find(0);
    ASSERT_TRUE(x != nullptr);
    *x = 2;

    EXPECT_EQ(2, v.find(0)->as<int>());
}

TEST(ValueTest, keyParsing)
{
    toml::Value v;
    v.set("0000.0000", 1);
    EXPECT_EQ(1, v.get<int>("0000.0000"));
    EXPECT_EQ(1, v.find("0000")->get<int>("0000"));
}

TEST(ValueTest, comparing)
{
    toml::Value n1, n2;
    toml::Value b1(true), b2(false), b3(true);
    toml::Value i1(1), i2(2), i3(1);
    toml::Value d1(1.0), d2(2.0), d3(1.0);
    toml::Value s1("foo"), s2("bar"), s3("foo");
    toml::Value a1((toml::Array())), a2((toml::Array())), a3((toml::Array()));
    a1.push(1);
    a2.push(2);
    a3.push(1);

    toml::Value t1((toml::Table())), t2((toml::Table())), t3((toml::Table()));
    t1.set("k1", "v1");
    t2.set("k2", "v2");
    t3.set("k1", "v1");

    EXPECT_TRUE(n1 == n2);
    EXPECT_TRUE(b1 == b3);
    EXPECT_TRUE(i1 == i3);
    EXPECT_TRUE(d1 == d3);
    EXPECT_TRUE(s1 == s3);
    EXPECT_TRUE(t1 == t3);

    EXPECT_TRUE(b1 != b2);
    EXPECT_TRUE(i1 != i2);
    EXPECT_TRUE(d1 != d2);
    EXPECT_TRUE(s1 != s2);
    EXPECT_TRUE(t1 != t2);

    EXPECT_TRUE(i1 != d1);
}

TEST(ValueTest, operatorBox)
{
    toml::Value v;
    v["key"] = "value";
    v["foo.bar"] = "foobar";
    v.setChild("foo", "bar");

    EXPECT_EQ("value", v.findChild("key")->as<std::string>());
    EXPECT_EQ("foobar", v.findChild("foo.bar")->as<std::string>());
    EXPECT_EQ("bar", v["foo"].as<std::string>());
}
