#ifndef TOML_H_
#define TOML_H_

#include <cassert>
#include <chrono>
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

inline std::vector<std::string> split(const std::string& s, char separator)
{
    std::vector<std::string> result;
    std::string::size_type p = 0;
    std::string::size_type q;
    while ((q = s.find(separator, p)) != std::string::npos) {
        result.emplace_back(s, p, q - p);
        p = q + 1;
    }

    result.emplace_back(s, p);
    return result;
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
    void push(const Value& v);

    Value* ensureTable(const std::string& key);
    Value* ensureArrayTable(const std::string& key);

private:
    static const char* typeToString(Type);

    // key should not contain '.'
    Value* unsafeSet(const std::string& key, const Value& v);
    Value* ensureValue(const std::string& key);

    Value* findInternal(const std::string& key);
    const Value* findInternal(const std::string& key) const;

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

#define IS(ctype, ttype)                                \
template<> inline bool Value::is<ctype>() const         \
{                                                       \
    return type_ == ttype;                              \
}

IS(bool, BOOL_TYPE)
IS(int, INT_TYPE)
IS(int64_t, INT_TYPE)
IS(double, DOUBLE_TYPE)
IS(std::string, STRING_TYPE)
IS(Time, TIME_TYPE)
IS(Array, ARRAY_TYPE)
IS(Table, TABLE_TYPE)
#undef IS

#define AS(type, var)                                                   \
template<> inline typename call_traits<type>::return_type Value::as<type>() const \
{                                                                       \
    if (!is<type>()) {                                                  \
        failwith("type error: this value is %s but %s was requested",   \
                 typeToString(type_), #type);                           \
    }                                                                   \
    return var;                                                         \
}

AS(bool, bool_)
AS(int64_t, int_)
AS(int, static_cast<int>(int_))
AS(double, double_)
AS(std::string, *string_)
AS(Time, *time_)
AS(Array, *array_)
AS(Table, *table_)
#undef AS

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

    auto parts = split(key, '.');
    auto lastKey = parts.back();
    parts.pop_back();

    const Value* current = this;
    for (const auto& part : parts) {
        current = current->findInternal(part);
        if (!current || !current->is<Table>())
            return nullptr;
    }

    return current->findInternal(lastKey);
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
                unsafeSet(kv.first, kv.second);
            }
        } else {
            unsafeSet(kv.first, kv.second);
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

inline Value* Value::unsafeSet(const std::string& key, const Value& v)
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

inline void Value::push(const Value& v)
{
    if (!valid())
        *this = Value((Array()));
    else if (!is<Array>())
        failwith("type must be array to do push(Value).");

    array_->push_back(v);
}

inline Value* Value::ensureValue(const std::string& key)
{
    if (!valid())
        *this = Value((Table()));

    if (!is<Table>())
        failwith("encountered non table value");

    auto parts = split(key, '.');
    auto lastKey = parts.back();
    parts.pop_back();

    Value* current = this;
    for (const auto& part : parts) {
        if (part.empty())
            failwith("key contains empty string?");

        if (Value* candidate = current->findInternal(part)) {
            if (!candidate->is<Table>())
                failwith("encountered non table value");
            current = candidate;
            continue;
        }

        current = current->unsafeSet(part, Table());
    }

    if (Value* v = current->findInternal(lastKey))
        return v;
    return current->unsafeSet(lastKey, Value());
}

inline Value* Value::findInternal(const std::string& key)
{
    assert(is<Table>());

    auto it = table_->find(key);
    if (it == table_->end())
        return nullptr;

    return &it->second;
}

inline const Value* Value::findInternal(const std::string& key) const
{
    assert(is<Table>());

    auto it = table_->find(key);
    if (it == table_->end())
        return nullptr;

    return &it->second;
}

inline Value* Value::ensureTable(const std::string& key)
{
    if (!valid())
        *this = Table();

    if (!is<Table>())
        return nullptr;

    auto parts = split(key, '.');
    Value* current = this;
    for (const auto& part : parts) {
        if (part.empty())
            return nullptr;

        if (Value* candidate = current->findInternal(part)) {
            if (!candidate->is<Table>())
                return nullptr;
            current = candidate;
            continue;
        }

        current = current->unsafeSet(part, Table());
    }

    return current;
}

inline Value* Value::ensureArrayTable(const std::string& key)
{
    if (!is<Table>())
        return nullptr;

    auto parts = split(key, '.');
    auto lastKey = parts.back();
    parts.pop_back();

    Value* current = this;
    for (const auto& part : parts) {
        if (part.empty())
            return nullptr;

        if (Value* candidate = current->findInternal(part)) {
            if (!candidate->is<Table>())
                return nullptr;
            current = candidate;
            continue;
        }

        current = current->unsafeSet(part, Table());
    }

    Value* candidate = current->findInternal(lastKey);
    if (!candidate) {
        candidate = current->unsafeSet(lastKey, Array());
    }

    if (!candidate->is<Array>())
        return nullptr;

    Array* internal = candidate->array_;
    if (internal->size() >= 1 && !(*internal)[0].is<Table>()) {
        return nullptr;
    }
    internal->push_back(Table());
    return &internal->back();
}

// ----------------------------------------------------------------------

class Parser {
public:
    Parser(std::istream& is) : is_(is) {}

    // Parses. If failed(), value should be null value. You can get
    // the error by calling errorReason().
    Value parse();
    const std::string& errorReason();

private:
    static bool isValidKeyChar(char c);
    static bool isInteger(const std::string&);
    static bool isDouble(const std::string&);
    static bool parseTime(const std::string&, Value*);

    void next();
    bool cur(char* c);
    bool expect(char c);
    bool expectEOF();
    bool expectEOL();

    void skipWhiteSpaces();
    void skipUntilNextLine();
    void skipUntilNextToken();
    // skip spaces and comments until next line.
    bool skipTrailing();

    Value* parseGroupKey(Value* root);

    bool parseKeyValue(Value*);
    bool parseKey(std::string*);
    bool parseValue(Value*);
    bool parseStringDoubleQuote(Value*);
    bool parseStringSingleQuote(Value*);
    bool parseBool(Value*);
    bool parseNumber(Value*);
    bool parseArray(Value*);
    bool parseInlineTable(Value*);

    void addError(const std::string& reason);

    std::istream& is_;
    int lineNo_ = 1;
    std::string errorReason_;
};

// static
inline bool Parser::isValidKeyChar(char c)
{
    if (c == ' ' || c == '\r' || c == '\n' || c == ']' || c == '[')
        return false;
    return true;
}

// static
inline bool Parser::isInteger(const std::string& s)
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

// static
inline bool Parser::isDouble(const std::string& s)
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
inline bool Parser::parseTime(const std::string& s, Value* v)
{
    // Time has the following form: YYYY-MM-DDThh:mm:ssZ
    // TODO(mayah): Follow RFC?

    int YYYY, MM, DD, hh, mm, ss;
    if (sscanf(s.c_str(), "%d-%d-%dT%d:%d:%d", &YYYY, &MM, &DD, &hh, &mm, &ss) != 6) {
        return false;
    }

    std::tm t;
    t.tm_sec = ss;
    t.tm_min = mm;
    t.tm_hour = hh;
    t.tm_mday = DD;
    t.tm_mon = MM - 1;
    t.tm_year = YYYY - 1900;

    *v = std::chrono::system_clock::from_time_t(timegm(&t));
    return true;
}

inline void Parser::addError(const std::string& reason)
{
    if (!errorReason_.empty())
        return;

    std::stringstream ss;
    ss << "Error: line " << lineNo_ << ": " << reason;
    errorReason_ = ss.str();
}

inline const std::string& Parser::errorReason()
{
    return errorReason_;
}

inline Value Parser::parse()
{
    Value root((Table()));
    Value* current = &root;

    char c;
    while (skipUntilNextToken(), cur(&c)) {
        if (c == '[') {
            current = parseGroupKey(&root);
            if (!current) {
                addError("error when parsing group key");
                return Value();
            }
            continue;
        }
        if (!parseKeyValue(current)) {
            addError("error when parsing key Value");
            return Value();
        }
    }

    return root;
}

inline bool Parser::cur(char* c)
{
    int x = is_.peek();
    if (x == EOF)
        return false;
    *c = static_cast<char>(x);
    return true;
}

inline void Parser::next()
{
    int x = is_.get();
    if (x == '\n')
        ++lineNo_;
}

inline bool Parser::expect(char c)
{
    char c1;
    if (cur(&c1) && c == c1) {
        next();
        return true;
    }

    return false;
}

inline bool Parser::expectEOF()
{
    char c;
    return !cur(&c);
}

inline bool Parser::expectEOL()
{
    char c;
    if (!cur(&c))
        return false;

    if (c == '\n') {
        next();
        return true;
    }
    if (c == '\r') {
        next();
        if (cur(&c) && c == '\n') {
            next();
            return true;
        }
    }

    return false;
}

inline void Parser::skipWhiteSpaces()
{
    char c;
    while (cur(&c)) {
        if (!(c == ' ' || c == '\t'))
            break;
        next();
    }
}

inline void Parser::skipUntilNextLine()
{
    char c;
    while (cur(&c)) {
        next();
        if (c == '\n')
            break;
    }
}

inline void Parser::skipUntilNextToken()
{
    char c;
    while (cur(&c)) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            next();
            continue;
        }
        if (c == '#') {
            skipUntilNextLine();
            continue;
        }
        break;
    }
}

inline bool Parser::skipTrailing()
{
    skipWhiteSpaces();
    char c;
    if (cur(&c) && c == '#') {
        skipUntilNextLine();
        return true;
    }

    return expectEOF() || expectEOL();
}

inline Value* Parser::parseGroupKey(Value* root)
{
    if (!expect('[')) {
        addError("group key is not started with '[' ?");
        return nullptr;
    }

    char c;
    if (cur(&c) && c == '[') {
        next();
        std::string key;
        while (cur(&c) && c != '\n' && c != ']') {
            next();
            key += c;
        }

        if (!expect(']')) {
            addError("array group key is not ended with ']]'");
            return nullptr;
        }
        if (!expect(']')) {
            addError("array group key is not ended with ']]'");
            return nullptr;
        }
        if (!skipTrailing()) {
            addError("some garbage after array group key?");
            return nullptr;
        }
        return root->ensureArrayTable(key);
    } else {
        std::string key;
        while (cur(&c) && c != '\n' && c != ']') {
            next();
            key += c;
        }
        if (!expect(']')) {
            addError("group key is not ended with ']'");
            return nullptr;
        }
        if (!skipTrailing()) {
            addError("some garbage after array group key?");
            return nullptr;
        }
        return root->ensureTable(key);
    }
}

inline bool Parser::parseKeyValue(Value* current)
{
    std::string key;
    if (!parseKey(&key))
        return false;
    skipWhiteSpaces();
    if (!expect('='))
        return false;
    skipWhiteSpaces();
    Value v;
    if (!parseValue(&v))
        return false;

    if (!skipTrailing())
        return false;

    if (current->has(key)) {
        addError("Multiple same key: " + key);
        return false;
    }

    current->set(key, std::move(v));
    return true;
}

inline bool Parser::parseKey(std::string* key)
{
    key->clear();

    char c;
    while (cur(&c) && isValidKeyChar(c)) {
        *key += c;
        next();
    }

    return !key->empty();
}

inline bool Parser::parseValue(Value* v)
{
    char c;
    if (!cur(&c))
        return false;

    switch (c) {
    case '"':
        return parseStringDoubleQuote(v);
    case '\'':
        return parseStringSingleQuote(v);
    case '[':
        return parseArray(v);
    case '{':
        return parseInlineTable(v);
    case 't':
    case 'f':
        return parseBool(v);
    default:
        return parseNumber(v);
    }

    return false;
}

inline bool Parser::parseBool(Value* v)
{
    std::string str;
    char c;
    while (cur(&c) && 'a' <= c && c <= 'z') {
        next();
        str += c;
    }
    if (str == "true") {
        *v = true;
        return true;
    }
    if (str == "false") {
        *v = false;
        return true;
    }

    return false;
}

inline bool Parser::parseStringDoubleQuote(Value* v)
{
    if (!expect('"')) {
        addError("string didn't start with '\"'?");
        return false;
    }

    std::string s;
    char c;

    if (cur(&c) && c == '"') {
        next();
        if (!cur(&c) || c != '"') {
            // OK. It's empty string.
            *v = "";
            return true;
        }
        next();
        // raw string literal started.
        // Newline just after """ should be ignored.
        if (cur(&c) && c == '\n')
            next();

        while (cur(&c)) {
            if (c == '"') {
                next();
                if (cur(&c) && c == '"') {
                    next();
                    if (cur(&c) && c == '"') {
                        next();
                        *v = s;
                        return true;
                    } else {
                        s += '"';
                        s += '"';
                        continue;
                    }
                } else {
                    s += '"';
                    continue;
                }
            }

            if (c == '\\') {
                next();
                if (cur(&c) && c == '\n') {
                    next();
                    while (cur(&c) && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                        next();
                    }
                    continue;
                } else {
                    s += c;
                    continue;
                }
            }

            next();
            s += c;
            continue;
        }

        addError("string didn't end with '\"\"\"' ?");
        return false;
    }

    while (cur(&c)) {
        next();
        if (c == '\\') {
            if (!cur(&c))
                return false;
            next();
            switch (c) {
            case 't': c = '\t'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case '"': c = '"'; break;
            case '\'': c = '\''; break;
            case '\\': c = '\\'; break;
            default: return false;
            }
        } else if (c == '"') {
            *v = s;
            return true;
        }

        s += c;
    }

    addError("string didn't end with '\"'?");
    return false;
}

inline bool Parser::parseStringSingleQuote(Value* v)
{
    if (!expect('\'')) {
        addError("string didn't start with '\''?");
        return false;
    }

    std::string s;
    char c;

    if (cur(&c) && c == '\'') {
        next();
        if (!cur(&c) || c != '\'') {
            // OK. It's empty string.
            *v = "";
            return true;
        }
        next();
        // raw string literal started.
        // Newline just after """ should be ignored.
        if (cur(&c) && c == '\n')
            next();

        while (cur(&c)) {
            if (c == '\'') {
                next();
                if (cur(&c) && c == '\'') {
                    next();
                    if (cur(&c) && c == '\'') {
                        next();
                        *v = s;
                        return true;
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

        addError("string didn't end with '\'\'\'' ?");
        return false;
    }

    while (cur(&c)) {
        next();
        if (c == '\'') {
            *v = s;
            return true;
        }

        s += c;
    }

    addError("string didn't end with '\''?");
    return false;
}

inline bool Parser::parseNumber(Value* v)
{
    std::string s;
    char c;
    while (cur(&c) && c != ' ' && c != '\t' && c != ',' && c != ']' && c != '\n' && c != '\r') {
        next();
        s += c;
    }

    if (isInteger(s)) {
        std::stringstream ss(removeDelimiter(s));
        int64_t x;
        ss >> x;
        *v = x;
        return true;
    }

    if (isDouble(s)) {
        std::stringstream ss(removeDelimiter(s));
        double d;
        ss >> d;
        *v = d;
        return true;
    }

    // Otherwise, try to parse as time.
    return parseTime(s, v);
}

inline bool Parser::parseArray(Value* v)
{
    if (!expect('[')) {
        addError("array didn't start with '['?");
        return false;
    }

    Array a;
    while (true) {
        skipUntilNextToken();

        char c;
        if (cur(&c) && c == ']') {
            break;
        }

        Value x;
        if (!parseValue(&x))
            return false;

        if (!a.empty() && a.front().type() != x.type())
            return false;
        a.push_back(std::move(x));

        skipUntilNextToken();
        if (cur(&c) && c == ',') {
            next();
            continue;
        }
        if (cur(&c) && c == ']')
            break;

        addError("array does not have ','?");
        return false;
    }

    expect(']');
    *v = std::move(a);
    return true;
}

inline bool Parser::parseInlineTable(Value* value)
{
    if (!expect('{')) {
        addError("inline table didn't start with '{'?");
        return false;
    }

    Value t((Table()));
    bool first = true;
    while (true) {
        skipUntilNextToken();

        char c;
        if (cur(&c) && c == '}') {
            break;
        }

        if (!first) {
            if (!expect(',')) {
                addError("inline table didn't have ',' for delimiter?");
                return false;
            }
            skipUntilNextToken();
        }
        first = false;

        std::string key;
        if (!parseKey(&key))
            return false;
        skipWhiteSpaces();
        if (!expect('='))
            return false;
        skipWhiteSpaces();
        Value v;
        if (!parseValue(&v))
            return false;

        if (t.has(key)) {
            addError("inline table has multiple same keys: key=" + key);
            return false;
        }

        t.set(key, v);
    }

    expect('}');
    *value = std::move(t);
    return true;
}

} // namespace toml

#endif
