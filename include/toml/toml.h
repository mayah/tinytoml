#ifndef TINYTOML_H_
#define TINYTOML_H_

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace toml
{

[[noreturn]]
inline void failwith(const char* reason, ...)
{
    char buf[1024];
    va_list va;
    va_start(va, reason);
    vsprintf(buf, reason, va);
    va_end(va);

    throw std::runtime_error(buf);
}

inline std::string removeDelimiter(const std::string& s)
{
    std::string r;
    for (char c : s) {
        if (c == '_')
            continue;
        r += c;
    }
    return r;
}

inline std::string unescape(const std::string& codepoint)
{
    std::uint64_t x = strtoll(codepoint.c_str(), nullptr, 16);
    char buf[8];

    if (x <= 0x7FULL) {
        // 0xxxxxxx
        buf[0] = 0x00 | ((x >> 0) & 0x7F);
        buf[1] = '\0';
    } else if (x <= 0x7FFULL) {
        // 110yyyyx 10xxxxxx
        buf[0] = 0xC0 | ((x >> 6) & 0xDF);
        buf[1] = 0x80 | ((x >> 0) & 0xBF);
        buf[2] = '\0';
    } else if (x <= 0xFFFFULL) {
        // 1110yyyy 10yxxxxx 10xxxxxx
        buf[0] = 0xE0 | ((x >> 12) & 0xEF);
        buf[1] = 0x80 | ((x >> 6) & 0xBF);
        buf[2] = 0x80 | ((x >> 0) & 0xBF);
        buf[3] = '\0';
    } else if (x <= 0x10FFFFULL) {
        // 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx
        buf[0] = 0xF0 | ((x >> 18) & 0xF7);
        buf[1] = 0x80 | ((x >> 12) & 0xBF);
        buf[2] = 0x80 | ((x >> 6) & 0xBF);
        buf[3] = 0x80 | ((x >> 0) & 0xBF);
        buf[4] = '\0';
    } else {
        buf[0] = '\0';
    }

    return buf;
}

inline bool isInteger(const std::string& s)
{
    if (s.empty())
        return false;

    std::string::size_type p = 0;
    if (s[p] == '+' || s[p] == '-')
        ++p;

    while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
        ++p;
        if (p < s.size() && s[p] == '_') {
            ++p;
            if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                return false;
        }
    }

    return p == s.size();
}

inline bool isDouble(const std::string& s)
{
    if (s.empty())
        return false;

    std::string::size_type p = 0;
    if (s[p] == '+' || s[p] == '-')
        ++p;

    bool ok = false;
    while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
        ++p;
        ok = true;

        if (p < s.size() && s[p] == '_') {
            ++p;
            if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                return false;
        }
    }

    if (p < s.size() && s[p] == '.')
        ++p;

    while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
        ++p;
        ok = true;

        if (p < s.size() && s[p] == '_') {
            ++p;
            if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                return false;
        }
    }

    if (!ok)
        return false;

    ok = false;
    if (p < s.size() && (s[p] == 'e' || s[p] == 'E')) {
        ++p;
        if (p < s.size() && (s[p] == '+' || s[p] == '-'))
            ++p;
        while (p < s.size() && '0' <= s[p] && s[p] <= '9') {
            ++p;
            ok = true;

            if (p < s.size() && s[p] == '_') {
                ++p;
                if (!(p < s.size() && '0' <= s[p] && s[p] <= '9'))
                    return false;
            }
        }
        if (!ok)
            return false;
    }

    return p == s.size();
}

// static
inline std::string escapeString(const std::string& s)
{
    std::stringstream ss;
    for (size_t i = 0; i < s.size(); ++i) {
        switch (s[i]) {
        case '\n': ss << "\\n"; break;
        case '\r': ss << "\\r"; break;
        case '\t': ss << "\\t"; break;
        case '\"': ss << "\\\""; break;
        case '\'': ss << "\\\'"; break;
        case '\\': ss << "\\\\"; break;
        default: ss << s[i]; break;
        }
    }

    return ss.str();
}

// ----------------------------------------------------------------------

enum class TokenType {
    ERROR,
    END_OF_FILE,
    END_OF_LINE,
    IDENT,
    STRING,
    MULTILINE_STRING,
    BOOL,
    INT,
    DOUBLE,
    TIME,
    COMMA,
    DOT,
    EQUAL,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
};

class Token {
public:
    explicit Token(TokenType type) : type_(type) {}
    Token(TokenType type, const std::string& v) : type_(type), strValue_(v) {}
    Token(TokenType type, bool v) : type_(type), intValue_(v) {}
    Token(TokenType type, std::int64_t v) : type_(type), intValue_(v) {}
    Token(TokenType type, double v) : type_(type), doubleValue_(v) {}
    Token(TokenType type, std::chrono::system_clock::time_point tp) : type_(type), timeValue_(tp) {}

    TokenType type() const { return type_; }
    const std::string& strValue() const { return strValue_; }
    bool boolValue() const { return intValue_; }
    std::int64_t intValue() const { return intValue_; }
    double doubleValue() const { return doubleValue_; }
    std::chrono::system_clock::time_point timeValue() const { return timeValue_; }

private:
    TokenType type_;
    std::string strValue_;
    std::int64_t intValue_;
    double doubleValue_;
    std::chrono::system_clock::time_point timeValue_;
};

class Lexer {
public:
    explicit Lexer(std::istream& is) : is_(is), lineNo_(1) {}

    Token nextKeyToken();
    Token nextValueToken();

    int lineNo() const { return lineNo_; }

private:
    bool current(char* c);
    void next();
    bool consume(char c);

    Token nextToken(bool isValueToken);

    void skipUntilNewLine();

    Token nextStringDoubleQuote();
    Token nextStringSingleQuote();

    Token nextKey();
    Token nextValue();

    Token parseAsTime(const std::string&);

    std::istream& is_;
    int lineNo_;
};

inline bool Lexer::current(char* c)
{
    int x = is_.peek();
    if (x == EOF)
        return false;
    *c = static_cast<char>(x);
    return true;
}

inline void Lexer::next()
{
    int x = is_.get();
    if (x == '\n')
        ++lineNo_;
}

inline bool Lexer::consume(char c)
{
    char x;
    if (!current(&x))
        return false;
    if (x != c)
        return false;
    next();
    return true;
}

inline void Lexer::skipUntilNewLine()
{
    char c;
    while (current(&c)) {
        if (c == '\n')
            return;
        next();
    }
}

inline Token Lexer::nextStringDoubleQuote()
{
    if (!consume('"'))
        return Token(TokenType::ERROR, "string didn't start with '\"'");

    std::string s;
    char c;
    bool multiline = false;

    if (current(&c) && c == '"') {
        next();
        if (!current(&c) || c != '"') {
            // OK. It's empty string.
            return Token(TokenType::STRING, "");
        }

        next();
        // raw string literal started.
        // Newline just after """ should be ignored.
        while (current(&c) && (c == ' ' || c == '\t'))
            next();
        if (current(&c) && c == '\n')
            next();
        multiline = true;
    }

    while (current(&c)) {
        next();
        if (c == '\\') {
            if (!current(&c))
                return Token(TokenType::ERROR, "string has unknown escape sequence");
            next();
            switch (c) {
            case 't': c = '\t'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 'u':
            case 'U': {
                int size = c == 'u' ? 4 : 8;
                std::string codepoint;
                for (int i = 0; i < size; ++i) {
                  if (current(&c) && (('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'))) {
                    codepoint += c;
                    next();
                  } else {
                    return Token(TokenType::ERROR, "string has unknown escape sequence");
                  }
                }
                s += unescape(codepoint);
                continue;
            }
            case '"': c = '"'; break;
            case '\'': c = '\''; break;
            case '\\': c = '\\'; break;
            case '\n':
                while (current(&c) && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                    next();
                }
                continue;
            default:
                return Token(TokenType::ERROR, "string has unknown escape sequence");
            }
        } else if (c == '"') {
            if (multiline) {
                if (current(&c) && c == '"') {
                    next();
                    if (current(&c) && c == '"') {
                        next();
                        return Token(TokenType::MULTILINE_STRING, s);
                    } else {
                        s += '"';
                        s += '"';
                        continue;
                    }
                } else {
                    s += '"';
                    continue;
                }
            } else {
                return Token(TokenType::STRING, s);
            }
        }

        s += c;
    }

    return Token(TokenType::ERROR, "string didn't end");
}

inline Token Lexer::nextStringSingleQuote()
{
    if (!consume('\''))
        return Token(TokenType::ERROR, "string didn't start with '\''?");

    std::string s;
    char c;

    if (current(&c) && c == '\'') {
        next();
        if (!current(&c) || c != '\'') {
            // OK. It's empty string.
            return Token(TokenType::STRING, "");
        }
        next();
        // raw string literal started.
        // Newline just after """ should be ignored.
        if (current(&c) && c == '\n')
            next();

        while (current(&c)) {
            if (c == '\'') {
                next();
                if (current(&c) && c == '\'') {
                    next();
                    if (current(&c) && c == '\'') {
                        next();
                        return Token(TokenType::MULTILINE_STRING, s);
                    } else {
                        s += '\'';
                        s += '\'';
                        continue;
                    }
                } else {
                    s += '\'';
                    continue;
                }
            }

            next();
            s += c;
            continue;
        }

        return Token(TokenType::ERROR, "string didn't end with '\'\'\'' ?");
    }

    while (current(&c)) {
        next();
        if (c == '\'') {
            return Token(TokenType::STRING, s);
        }

        s += c;
    }

    return Token(TokenType::ERROR, "string didn't end with '\''?");
}

inline Token Lexer::nextKey()
{
    std::string s;
    char c;
    while (current(&c) && (isalnum(c) || c == '_' || c == '-')) {
        s += c;
        next();
    }

    if (s.empty())
        return Token(TokenType::ERROR, "Unknown key format");

    return Token(TokenType::IDENT, s);
}

inline Token Lexer::nextValue()
{
    std::string s;
    char c;

    if (current(&c) && isalpha(c)) {
        s += c;
        next();
        while (current(&c) && isalpha(c)) {
            s += c;
            next();
        }

        if (s == "true")
            return Token(TokenType::BOOL, true);
        if (s == "false")
            return Token(TokenType::BOOL, false);
        return Token(TokenType::ERROR, "Unknown ident: " + s);
    }

    while (current(&c) && (('0' <= c && c <= '9') || c == '.' || c == 'e' || c == 'E' ||
                           c == 'T' || c == 'Z' || c == '_' || c == ':' || c == '-' || c == '+')) {
        next();
        s += c;
    }

    if (isInteger(s)) {
        std::stringstream ss(removeDelimiter(s));
        std::int64_t x;
        ss >> x;
        return Token(TokenType::INT, x);
    }

    if (isDouble(s)) {
        std::stringstream ss(removeDelimiter(s));
        double d;
        ss >> d;
        return Token(TokenType::DOUBLE, d);
    }

    return parseAsTime(s);
}

inline Token Lexer::parseAsTime(const std::string& str)
{
    const char* s = str.c_str();

    int n;
    int YYYY, MM, DD;
    if (sscanf(s, "%d-%d-%d%n", &YYYY, &MM, &DD, &n) != 3)
        return Token(TokenType::ERROR, "Invalid token");

    if (s[n] == '\0') {
        std::tm t;
        t.tm_sec = 0;
        t.tm_min = 0;
        t.tm_hour = 0;
        t.tm_mday = DD;
        t.tm_mon = MM - 1;
        t.tm_year = YYYY - 1900;
        auto tp = std::chrono::system_clock::from_time_t(timegm(&t));
        return Token(TokenType::TIME, tp);
    }

    if (s[n] != 'T')
        return Token(TokenType::ERROR, "Invalid token");

    s = s + n + 1;

    int hh, mm;
    double ss; // double for fraction
    if (sscanf(s, "%d:%d:%lf%n", &hh, &mm, &ss, &n) != 3)
        return Token(TokenType::ERROR, "Invalid token");

    std::tm t;
    t.tm_sec = ss;
    t.tm_min = mm;
    t.tm_hour = hh;
    t.tm_mday = DD;
    t.tm_mon = MM - 1;
    t.tm_year = YYYY - 1900;
    auto tp = std::chrono::system_clock::from_time_t(timegm(&t));
    ss -= static_cast<int>(ss);
    tp += std::chrono::microseconds(static_cast<int>(std::round(ss * 1000000)));

    if (s[n] == '\0')
        return Token(TokenType::TIME, tp);

    if (s[n] == 'Z' && s[n + 1] == '\0')
        return Token(TokenType::TIME, tp);

    s = s + n;
    // offset
    // [+/-]%d:%d
    char pn;
    int oh, om;
    if (sscanf(s, "%c%d:%d", &pn, &oh, &om) != 3)
        return Token(TokenType::ERROR, "Invalid token");

    if (pn != '+' && pn != '-')
        return Token(TokenType::ERROR, "Invalid token");

    if (pn == '+') {
        tp -= std::chrono::hours(oh);
        tp -= std::chrono::minutes(om);
    } else {
        tp += std::chrono::hours(oh);
        tp += std::chrono::minutes(om);
    }

    return Token(TokenType::TIME, tp);
}

inline Token Lexer::nextKeyToken()
{
    return nextToken(false);
}

inline Token Lexer::nextValueToken()
{
    return nextToken(true);
}

inline Token Lexer::nextToken(bool isValueToken)
{
    char c;
    while (current(&c)) {
        if (c == ' ' || c == '\t' || c == '\r') {
            next();
            continue;
        }

        if (c == '#') {
            skipUntilNewLine();
            continue;
        }

        switch (c) {
        case '\n':
            next();
            return Token(TokenType::END_OF_LINE);
        case '=':
            next();
            return Token(TokenType::EQUAL);
        case '{':
            next();
            return Token(TokenType::LBRACE);
        case '}':
            next();
            return Token(TokenType::RBRACE);
        case '[':
            next();
            return Token(TokenType::LBRACKET);
        case ']':
            next();
            return Token(TokenType::RBRACKET);
        case ',':
            next();
            return Token(TokenType::COMMA);
        case '.':
            next();
            return Token(TokenType::DOT);
        case '\"':
            return nextStringDoubleQuote();
        case '\'':
            return nextStringSingleQuote();
        default:
            if (isValueToken) {
                return nextValue();
            } else {
                return nextKey();
            }
        }
    }

    return Token(TokenType::END_OF_FILE);
}

// ----------------------------------------------------------------------

class Value;
typedef std::chrono::system_clock::time_point Time;
typedef std::vector<Value> Array;
typedef std::map<std::string, Value> Table;

template<typename T> struct call_traits;
#define NONREF_TRAITS(type)                     \
template<> struct call_traits<type> {           \
    typedef const type param_type;              \
    typedef type return_type;                   \
};
#define REF_TRAITS(type)                        \
template<> struct call_traits<type> {           \
    typedef const type& param_type;             \
    typedef const type& return_type;            \
};
NONREF_TRAITS(bool);
NONREF_TRAITS(int);
NONREF_TRAITS(int64_t);
NONREF_TRAITS(double);
REF_TRAITS(std::string);
REF_TRAITS(Time);
REF_TRAITS(Array);
REF_TRAITS(Table);
#undef NONREF_TRAITS
#undef REF_TRAITS

class Value {
public:
    enum Type {
        NULL_TYPE,
        BOOL_TYPE,
        INT_TYPE,
        DOUBLE_TYPE,
        STRING_TYPE,
        TIME_TYPE,
        ARRAY_TYPE,
        TABLE_TYPE,
    };

    Value() : type_(NULL_TYPE), null_(nullptr) {}
    Value(bool v) : type_(BOOL_TYPE), bool_(v) {}
    Value(int v) : type_(INT_TYPE), int_(v) {}
    Value(int64_t v) : type_(INT_TYPE), int_(v) {}
    Value(double v) : type_(DOUBLE_TYPE), double_(v) {}
    Value(const std::string& v) : type_(STRING_TYPE), string_(new std::string(v)) {}
    Value(const char* v) : type_(STRING_TYPE), string_(new std::string(v)) {}
    Value(const Time& v) : type_(TIME_TYPE), time_(new Time(v)) {}
    Value(const Array& v) : type_(ARRAY_TYPE), array_(new Array(v)) {}
    Value(const Table& v) : type_(TABLE_TYPE), table_(new Table(v)) {}
    Value(std::string&& v) : type_(STRING_TYPE), string_(new std::string(v)) {}
    Value(Array&& v) : type_(ARRAY_TYPE), array_(new Array(v)) {}
    Value(Table&& v) : type_(TABLE_TYPE), table_(new Table(v)) {}

    Value(const Value& v);
    Value(Value&& v);
    Value& operator=(const Value& v);
    Value& operator=(Value&& v);

    // Someone might use a value like this:
    // toml::Value v = x->find("foo");
    // But this is wrong. Without this constructor,
    // value will be unexpectedly initialized with bool.
    Value(const void* v) = delete;
    ~Value();

    size_t size() const;
    bool empty() const;
    Type type() const { return type_; }

    bool valid() const { return type_ != NULL_TYPE; }
    template<typename T> bool is() const;
    template<typename T> typename call_traits<T>::return_type as() const;

    bool isNumber() const;
    double asNumber() const;

    void write(std::ostream*, const std::string& keyPrefix = std::string()) const;
    friend std::ostream& operator<<(std::ostream&, const Value&);

    // For table value
    template<typename T> typename call_traits<T>::return_type get(const std::string&) const;
    Value* set(const std::string& key, const Value& v);
    void erase(const std::string& key);
    const Value* find(const std::string& key) const;
    Value* find(const std::string& key);
    bool has(const std::string& key) const { return find(key) != nullptr; }
    // Merge table. Returns true if succeeded. Otherwise, |this| might be corrupted.
    bool merge(const Value&);

    // For array value
    template<typename T> typename call_traits<T>::return_type get(size_t index) const;
    const Value* find(size_t index) const;
    Value* find(size_t index);
    Value* push(const Value& v);

    // key should not contain '.'
    Value* findSingle(const std::string& key);
    const Value* findSingle(const std::string& key) const;
    Value* setSingle(const std::string& key, const Value& v);

private:
    static const char* typeToString(Type);

    Value* ensureValue(const std::string& key);

    Type type_;
    union {
        void* null_;
        bool bool_;
        int64_t int_;
        double double_;
        std::string* string_;
        Time* time_;
        Array* array_;
        Table* table_;
    };
};

// static
inline const char* Value::typeToString(Value::Type type)
{
    switch (type) {
    case NULL_TYPE:   return "null";
    case BOOL_TYPE:   return "bool";
    case INT_TYPE:    return "int";
    case DOUBLE_TYPE: return "double";
    case STRING_TYPE: return "string";
    case TIME_TYPE:   return "time";
    case ARRAY_TYPE:  return "array";
    case TABLE_TYPE:  return "table";
    default:          return "unknown";
    }
}

inline Value::Value(const Value& v) :
    type_(v.type_)
{
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = new std::string(*v.string_); break;
    case TIME_TYPE: time_ = new Time(*v.time_); break;
    case ARRAY_TYPE: array_ = new Array(*v.array_); break;
    case TABLE_TYPE: table_ = new Table(*v.table_); break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }
}

inline Value::Value(Value&& v) :
    type_(v.type_)
{
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = v.string_; break;
    case TIME_TYPE: time_ = v.time_; break;
    case ARRAY_TYPE: array_ = v.array_; break;
    case TABLE_TYPE: table_ = v.table_; break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }

    v.type_ = NULL_TYPE;
    v.null_ = nullptr;
}

inline Value& Value::operator=(const Value& v)
{
    if (this == &v)
        return *this;

    this->~Value();

    type_ = v.type_;
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = new std::string(*v.string_); break;
    case TIME_TYPE: time_ = new Time(*v.time_); break;
    case ARRAY_TYPE: array_ = new Array(*v.array_); break;
    case TABLE_TYPE: table_ = new Table(*v.table_); break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }

    return *this;
}

inline Value& Value::operator=(Value&& v)
{
    if (this == &v)
        return *this;

    this->~Value();

    type_ = v.type_;
    switch (v.type_) {
    case NULL_TYPE: null_ = v.null_; break;
    case BOOL_TYPE: bool_ = v.bool_; break;
    case INT_TYPE: int_ = v.int_; break;
    case DOUBLE_TYPE: double_ = v.double_; break;
    case STRING_TYPE: string_ = v.string_; break;
    case TIME_TYPE: time_ = v.time_; break;
    case ARRAY_TYPE: array_ = v.array_; break;
    case TABLE_TYPE: table_ = v.table_; break;
    default:
        assert(false);
        type_ = NULL_TYPE;
        null_ = nullptr;
    }

    v.type_ = NULL_TYPE;
    v.null_ = nullptr;
    return *this;
}

inline Value::~Value()
{
    switch (type_) {
    case STRING_TYPE:
        delete string_;
        break;
    case TIME_TYPE:
        delete time_;
        break;
    case ARRAY_TYPE:
        delete array_;
        break;
    case TABLE_TYPE:
        delete table_;
        break;
    default:
        break;
    }
}

inline size_t Value::size() const
{
    switch (type_) {
    case NULL_TYPE:
        return 0;
    case ARRAY_TYPE:
        return array_->size();
    case TABLE_TYPE:
        return table_->size();
    default:
        return 1;
    }
}

inline bool Value::empty() const
{
    return size() == 0;
}

template<> inline bool Value::is<bool>() const { return type_ == BOOL_TYPE; }
template<> inline bool Value::is<int>() const { return type_ == INT_TYPE; }
template<> inline bool Value::is<int64_t>() const { return type_ == INT_TYPE; }
template<> inline bool Value::is<double>() const { return type_ == DOUBLE_TYPE; }
template<> inline bool Value::is<std::string>() const { return type_ == STRING_TYPE; }
template<> inline bool Value::is<Time>() const { return type_ == TIME_TYPE; }
template<> inline bool Value::is<Array>() const { return type_ == ARRAY_TYPE; }
template<> inline bool Value::is<Table>() const { return type_ == TABLE_TYPE; }

template<> inline typename call_traits<bool>::return_type Value::as<bool>() const
{
    if (!is<bool>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "bool");
    return bool_;
}
template<> inline typename call_traits<int64_t>::return_type Value::as<int64_t>() const
{
    if (!is<int64_t>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "int64_t");
    return int_;
}
template<> inline typename call_traits<int>::return_type Value::as<int>() const
{
    if (!is<int>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "int");
    return static_cast<int>(int_);
}
template<> inline typename call_traits<double>::return_type Value::as<double>() const
{
    if (!is<double>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "double");
    return double_;
}
template<> inline typename call_traits<std::string>::return_type Value::as<std::string>() const
{
    if (!is<std::string>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "string");
    return *string_;
}
template<> inline typename call_traits<Time>::return_type Value::as<Time>() const
{
    if (!is<Time>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "time");
    return *time_;
}
template<> inline typename call_traits<Array>::return_type Value::as<Array>() const
{
    if (!is<Array>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "array");
    return *array_;
}
template<> inline typename call_traits<Table>::return_type Value::as<Table>() const
{
    if (!is<Table>())
        failwith("type error: this value is %s but %s was requested", typeToString(type_), "table");
    return *table_;
}

inline bool Value::isNumber() const
{
    return is<int>() || is<double>();
}

inline double Value::asNumber() const
{
    if (is<int>())
        return as<int>();
    if (is<double>())
        return as<double>();

    failwith("type error: this value is %s but number is requested", typeToString(type_));
    return 0.0;
}

inline void Value::write(std::ostream* os, const std::string& keyPrefix) const
{
    switch (type_) {
    case NULL_TYPE:
        failwith("null type value is not a valid value");
        break;
    case BOOL_TYPE:
        (*os) << (bool_ ? "true" : "false");
        break;
    case INT_TYPE:
        (*os) << int_;
        break;
    case DOUBLE_TYPE: {
        (*os) << std::fixed << std::showpoint << double_;
        break;
    }
    case STRING_TYPE:
        (*os) << '"' << escapeString(*string_) << '"';
        break;
    case TIME_TYPE: {
        time_t tt = std::chrono::system_clock::to_time_t(*time_);
        std::tm t;
        gmtime_r(&tt, &t);
        char buf[256];
        sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02dZ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
        (*os) << buf;
        break;
    }
    case ARRAY_TYPE:
        (*os) << '[';
        for (size_t i = 0; i < array_->size(); ++i) {
            if (i)
                (*os) << ", ";
            (*array_)[i].write(os, keyPrefix);
        }
        (*os) << ']';
        break;
    case TABLE_TYPE:
        for (const auto& kv : *table_) {
            if (kv.second.is<Table>())
                continue;
            if (kv.second.is<Array>() && kv.second.size() > 0 && kv.second.find(0)->is<Table>())
                continue;
            (*os) << kv.first << " = ";
            kv.second.write(os, keyPrefix);
            (*os) << '\n';
        }
        for (const auto& kv : *table_) {
            if (kv.second.is<Table>()) {
                std::string key(keyPrefix);
                if (!keyPrefix.empty())
                    key += ".";
                key += kv.first;
                (*os) << "\n[" << key << "]\n";
                kv.second.write(os, key);
            }
            if (kv.second.is<Array>() && kv.second.size() > 0 && kv.second.find(0)->is<Table>()) {
                std::string key(keyPrefix);
                if (!keyPrefix.empty())
                    key += ".";
                key += kv.first;
                for (const auto& v : kv.second.as<Array>()) {
                    (*os) << "\n[[" << key << "]]\n";
                    v.write(os, key);
                }
            }
        }
        break;
    default:
        failwith("writing unknown type");
        break;
    }
}

// static
inline std::ostream& operator<<(std::ostream& os, const toml::Value& v)
{
    v.write(&os);
    return os;
}

template<typename T>
inline typename call_traits<T>::return_type Value::get(const std::string& key) const
{
    if (!is<Table>())
        failwith("type must be table to do get(key).");

    const Value* obj = find(key);
    if (!obj)
        failwith("key %s was not found.", key.c_str());
    return obj->as<T>();
}

inline const Value* Value::find(const std::string& key) const
{
    if (!is<Table>())
        return nullptr;

    std::istringstream ss(key);
    Lexer lexer(ss);

    const Value* current = this;
    while (true) {
        Token t = lexer.nextKeyToken();
        if (!(t.type() == TokenType::IDENT || t.type() == TokenType::STRING))
            return nullptr;

        std::string part = t.strValue();
        t = lexer.nextKeyToken();
        if (t.type() == TokenType::DOT) {
            current = current->findSingle(part);
            if (!current || !current->is<Table>())
                return nullptr;
        } else if (t.type() == TokenType::END_OF_FILE) {
            return current->findSingle(part);
        } else {
            return nullptr;
        }
    }
}

inline Value* Value::find(const std::string& key)
{
    return const_cast<Value*>(const_cast<const Value*>(this)->find(key));
}

inline bool Value::merge(const toml::Value& v)
{
    if (this == &v)
        return true;
    if (!is<Table>() || !v.is<Table>())
        return false;

    for (const auto& kv : *v.table_) {
        if (Value* tmp = find(kv.first)) {
            // If both are table, we merge them.
            if (tmp->is<Table>() && kv.second.is<Table>()) {
                if (!tmp->merge(kv.second))
                    return false;
            } else {
                setSingle(kv.first, kv.second);
            }
        } else {
            setSingle(kv.first, kv.second);
        }
    }

    return true;
}

inline Value* Value::set(const std::string& key, const Value& v)
{
    Value* result = ensureValue(key);
    *result = v;
    return result;
}

inline Value* Value::setSingle(const std::string& key, const Value& v)
{
    if (!valid())
        *this = Value((Table()));

    if (!is<Table>())
        failwith("type must be table to do set(key, v).");

    (*table_)[key] = v;
    return &(*table_)[key];
}

inline void Value::erase(const std::string& key)
{
    if (!is<Table>())
        failwith("type must be table to do erase(key).");

    table_->erase(key);
}

template<typename T>
inline typename call_traits<T>::return_type Value::get(size_t index) const
{
    if (!is<Array>())
        failwith("type must be array to do get(index).");

    if (array_->size() <= index)
        failwith("index out of bound");

    return (*array_)[index].as<T>();
}

inline const Value* Value::find(size_t index) const
{
    if (!is<Array>())
        return nullptr;
    if (index < array_->size())
        return &(*array_)[index];
    return nullptr;
}

inline Value* Value::find(size_t index)
{
    return const_cast<Value*>(const_cast<const Value*>(this)->find(index));
}

inline Value* Value::push(const Value& v)
{
    if (!valid())
        *this = Value((Array()));
    else if (!is<Array>())
        failwith("type must be array to do push(Value).");

    array_->push_back(v);
    return &array_->back();
}

inline Value* Value::ensureValue(const std::string& key)
{
    if (!valid())
        *this = Value((Table()));
    if (!is<Table>()) {
        failwith("encountered non table value");
        return nullptr;
    }

    std::istringstream ss(key);
    Lexer lexer(ss);

    Value* current = this;
    while (true) {
        Token t = lexer.nextKeyToken();
        if (!(t.type() == TokenType::IDENT || t.type() == TokenType::STRING)) {
            failwith("invalid key");
            return nullptr;
        }

        std::string part = t.strValue();
        t = lexer.nextKeyToken();
        if (t.type() == TokenType::DOT) {
            if (Value* candidate = current->findSingle(part)) {
                if (!candidate->is<Table>())
                    failwith("encountered non table value");
                current = candidate;
            } else {
                current = current->setSingle(part, Table());
            }
        } else if (t.type() == TokenType::END_OF_FILE) {
            if (Value* v = current->findSingle(part))
                return v;
            return current->setSingle(part, Value());
        } else {
            failwith("invalid key");
            return nullptr;
        }
    }
}

inline Value* Value::findSingle(const std::string& key)
{
    assert(is<Table>());

    auto it = table_->find(key);
    if (it == table_->end())
        return nullptr;

    return &it->second;
}

inline const Value* Value::findSingle(const std::string& key) const
{
    assert(is<Table>());

    auto it = table_->find(key);
    if (it == table_->end())
        return nullptr;

    return &it->second;
}

// ----------------------------------------------------------------------

class Parser {
public:
    explicit Parser(std::istream& is) : lexer_(is), token_(TokenType::ERROR) { nextKey(); }

    // Parses. If failed(), value should be null value. You can get
    // the error by calling errorReason().
    Value parse();
    const std::string& errorReason();

private:
    const Token& token() const { return token_; }
    void nextKey() { token_ = lexer_.nextKeyToken(); }
    void nextValue() { token_ = lexer_.nextValueToken(); }

    void skipForKey();
    void skipForValue();

    bool consumeForKey(TokenType);
    bool consumeForValue(TokenType);
    bool consumeEOLorEOFForKey();

    Value* parseGroupKey(Value* root);

    bool parseKeyValue(Value*);
    bool parseKey(std::string*);
    bool parseValue(Value*);
    bool parseBool(Value*);
    bool parseNumber(Value*);
    bool parseArray(Value*);
    bool parseInlineTable(Value*);

    void addError(const std::string& reason);

    Lexer lexer_;
    Token token_;
    std::string errorReason_;
};

inline void Parser::skipForKey()
{
    while (token().type() == TokenType::END_OF_LINE)
        nextKey();
}

inline void Parser::skipForValue()
{
    while (token().type() == TokenType::END_OF_LINE)
        nextValue();
}

inline bool Parser::consumeForKey(TokenType type)
{
    if (token().type() == type) {
        nextKey();
        return true;
    }

    return false;
}

inline bool Parser::consumeForValue(TokenType type)
{
    if (token().type() == type) {
        nextValue();
        return true;
    }

    return false;
}

inline bool Parser::consumeEOLorEOFForKey()
{
    if (token().type() == TokenType::END_OF_LINE || token().type() == TokenType::END_OF_FILE) {
        nextKey();
        return true;
    }

    return false;
}

inline void Parser::addError(const std::string& reason)
{
    if (!errorReason_.empty())
        return;

    std::stringstream ss;
    ss << "Error: line " << lexer_.lineNo() << ": " << reason;
    errorReason_ = ss.str();
}

inline const std::string& Parser::errorReason()
{
    return errorReason_;
}

inline Value Parser::parse()
{
    Value root((Table()));
    Value* currentValue = &root;

    while (true) {
        skipForKey();
        if (token().type() == TokenType::END_OF_FILE)
            break;
        if (token().type() == TokenType::LBRACKET) {
            currentValue = parseGroupKey(&root);
            if (!currentValue) {
                addError("error when parsing group key");
                return Value();
            }
            continue;
        }

        if (!parseKeyValue(currentValue)) {
            addError("error when parsing key Value");
            return Value();
        }
    }
    return root;
}

inline Value* Parser::parseGroupKey(Value* root)
{
    if (!consumeForKey(TokenType::LBRACKET))
        return nullptr;

    bool isArray = false;
    if (token().type() == TokenType::LBRACKET) {
        nextKey();
        isArray = true;
    }

    Value* currentValue = root;
    while (true) {
        if (token().type() != TokenType::IDENT && token().type() != TokenType::STRING)
            return nullptr;

        std::string key = token().strValue();
        nextKey();

        if (token().type() == TokenType::DOT) {
            nextKey();
            if (Value* candidate = currentValue->findSingle(key)) {
                if (candidate->is<Array>() && candidate->size() > 0)
                    candidate = candidate->find(candidate->size() - 1);
                if (!candidate->is<Table>())
                    return nullptr;
                currentValue = candidate;
            } else {
                currentValue = currentValue->setSingle(key, Table());
            }
            continue;
        }

        if (token().type() == TokenType::RBRACKET) {
            nextKey();
            if (Value* candidate = currentValue->findSingle(key)) {
                if (isArray) {
                    if (!candidate->is<Array>())
                        return nullptr;
                    currentValue = candidate->push(Table());
                } else {
                    if (candidate->is<Array>() && candidate->size() > 0)
                        candidate = candidate->find(candidate->size() - 1);
                    if (!candidate->is<Table>())
                        return nullptr;
                    currentValue = candidate;
                }
            } else {
                if (isArray) {
                    currentValue = currentValue->setSingle(key, Array());
                    currentValue = currentValue->push(Table());
                } else {
                    currentValue = currentValue->setSingle(key, Table());
                }
            }
            break;
        }

        return nullptr;
    }

    if (isArray) {
        if (!consumeForKey(TokenType::RBRACKET))
            return nullptr;
    }

    if (!consumeEOLorEOFForKey())
        return nullptr;

    return currentValue;
}

inline bool Parser::parseKeyValue(Value* current)
{
    std::string key;
    if (!parseKey(&key)) {
        addError("parse key failed");
        return false;
    }
    if (!consumeForValue(TokenType::EQUAL)) {
        addError("no equal?");
        return false;
    }

    Value v;
    if (!parseValue(&v))
        return false;
    if (!consumeEOLorEOFForKey())
        return false;

    if (current->has(key)) {
        addError("Multiple same key: " + key);
        return false;
    }

    current->setSingle(key, std::move(v));
    return true;
}

inline bool Parser::parseKey(std::string* key)
{
    key->clear();

    if (token().type() == TokenType::IDENT || token().type() == TokenType::STRING) {
        *key = token().strValue();
        nextValue();
        return true;
    }

    return false;
}

inline bool Parser::parseValue(Value* v)
{
    switch (token().type()) {
    case TokenType::STRING:
    case TokenType::MULTILINE_STRING:
        *v = token().strValue();
        nextValue();
        return true;
    case TokenType::LBRACKET:
        return parseArray(v);
    case TokenType::LBRACE:
        return parseInlineTable(v);
    case TokenType::BOOL:
        *v = token().boolValue();
        nextValue();
        return true;
    case TokenType::INT:
        *v = token().intValue();
        nextValue();
        return true;
    case TokenType::DOUBLE:
        *v = token().doubleValue();
        nextValue();
        return true;
    case TokenType::TIME:
        *v = token().timeValue();
        nextValue();
        return true;
    case TokenType::ERROR:
        addError(token().strValue());
        return false;
    default:
        addError("unexpected token");
        return false;
    }
}

inline bool Parser::parseBool(Value* v)
{
    if (token().strValue() == "true") {
        nextValue();
        *v = true;
        return true;
    }

    if (token().strValue() == "false") {
        nextValue();
        *v = false;
        return true;
    }

    return false;
}

inline bool Parser::parseArray(Value* v)
{
    if (!consumeForValue(TokenType::LBRACKET))
        return false;

    Array a;
    while (true) {
        skipForValue();

        if (token().type() == TokenType::RBRACKET)
            break;

        skipForValue();
        Value x;
        if (!parseValue(&x))
            return false;

        if (!a.empty()) {
            if (a.front().type() != x.type()) {
                addError("type check failed");
                return false;
            }
        }

        a.push_back(std::move(x));
        skipForValue();
        if (token().type() == TokenType::RBRACKET)
            break;
        if (token().type() == TokenType::COMMA)
            nextValue();
    }

    if (!consumeForValue(TokenType::RBRACKET))
        return false;
    *v = std::move(a);
    return true;
}

inline bool Parser::parseInlineTable(Value* value)
{
    // For inline table, next is KEY, so use consumeForKey here.
    if (!consumeForKey(TokenType::LBRACE))
        return false;

    Value t((Table()));
    bool first = true;
    while (true) {
        if (token().type() == TokenType::RBRACE) {
            break;
        }

        if (!first) {
            if (token().type() != TokenType::COMMA) {
                addError("inline table didn't have ',' for delimiter?");
                return false;
            }
            nextKey();
        }
        first = false;

        std::string key;
        if (!parseKey(&key))
            return false;
        if (!consumeForValue(TokenType::EQUAL))
            return false;
        Value v;
        if (!parseValue(&v))
            return false;

        if (t.has(key)) {
            addError("inline table has multiple same keys: key=" + key);
            return false;
        }

        t.set(key, v);
    }

    if (!consumeForValue(TokenType::RBRACE))
        return false;
    *value = std::move(t);
    return true;
}

} // namespace toml

#endif // TINYTOML_H_
