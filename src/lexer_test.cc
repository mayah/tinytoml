#include "toml/toml.h"

#include <sstream>
#include <string>
#include <gtest/gtest.h>

using namespace std;
using namespace toml;
using namespace toml::internal;

TEST(LexerTest, empty)
{
    stringstream ss("");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextKeyToken().type());
}

TEST(LexerTest, newline)
{
    stringstream ss("\n");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextKeyToken().type());
}

TEST(LexerTest, comment1)
{
    stringstream ss("#");
    Lexer lexer(ss);
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextKeyToken().type());
}

TEST(LexerTest, punctuation)
{
    stringstream ss("[]{},.=");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::LBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::LBRACE, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::RBRACE, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::COMMA, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::DOT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextKeyToken().type());
}

TEST(LexerTest, string1_key)
{
    stringstream ss("\"foo bar\"");
    Lexer lexer(ss);

    Token t = lexer.nextKeyToken();
    EXPECT_EQ(TokenType::STRING, t.type());
    EXPECT_EQ("foo bar", t.strValue());
}

TEST(LexerTest, string1_value)
{
    stringstream ss("\"foo bar\"");
    Lexer lexer(ss);

    Token t = lexer.nextValueToken();
    EXPECT_EQ(TokenType::STRING, t.type());
    EXPECT_EQ("foo bar", t.strValue());
}

TEST(LexerTest, string2)
{
    stringstream ss("\"\"\"foo\nbar\"\"\"");
    Lexer lexer(ss);

    Token t = lexer.nextValueToken();
    EXPECT_EQ(TokenType::MULTILINE_STRING, t.type());
    EXPECT_EQ("foo\nbar", t.strValue());
}

TEST(LexerTest, integer1)
{
    stringstream ss("1");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, integer2)
{
    stringstream ss("0");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, integer3)
{
    stringstream ss("+1");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, integer4)
{
    stringstream ss("-1");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, integer5)
{
    stringstream ss("-100_000");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, time1)
{
    stringstream ss("1979-05-27T07:32:00Z");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::TIME, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, exmaple1)
{
    stringstream ss("x = true\n");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::BOOL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, exmaple2)
{
    stringstream ss(
        "x = [1, 2, 3]\n"
        "y = []\n"
        "z = [\"\", \"\", ]");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::LBRACKET, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::COMMA, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::COMMA, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextValueToken().type());

    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::LBRACKET, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextValueToken().type());

    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::LBRACKET, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::STRING, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::COMMA, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::STRING, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::COMMA, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, exmaple3)
{
    stringstream ss(
        "[[kotori]]\n"
        "foo = 1\n"
        "[[kotori]]\n"
        "bar = 2\n");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::LBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::LBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextKeyToken().type());

    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextValueToken().type());

    EXPECT_EQ(TokenType::LBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::LBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextKeyToken().type());

    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextValueToken().type());

    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}

TEST(LexerTest, exmaple4)
{
    stringstream ss(
        "[kotori]# foo bar\n"
        "kotori = 1#foo bar");
    Lexer lexer(ss);

    EXPECT_EQ(TokenType::LBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::RBRACKET, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::END_OF_LINE, lexer.nextKeyToken().type());

    EXPECT_EQ(TokenType::IDENT, lexer.nextKeyToken().type());
    EXPECT_EQ(TokenType::EQUAL, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::INT, lexer.nextValueToken().type());
    EXPECT_EQ(TokenType::END_OF_FILE, lexer.nextValueToken().type());
}
