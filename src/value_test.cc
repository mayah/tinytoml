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
}

TEST(ValueTest, array)
{
    toml::Value v;

    // If we call push to null value, the value will become array automatically.
    v.push(0);
    v.push(1);

    EXPECT_TRUE(v.is<toml::Array>());
    EXPECT_EQ(0, v.get<int>(0));
    EXPECT_EQ(1, v.get<int>(1));
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
