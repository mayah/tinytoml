#ifndef TOML_TEST_BUILD_H
#define TOML_TEST_BUILD_H

// Because timegm is defined in toml namespace...
#if defined(_WIN32)
using toml::timegm;
#endif

// Include me last in your _test.cc if you need me.

inline toml::Value build_array_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top= &root;

    {
        auto array = top->setChild("arr1", toml::Array());
        array->push(1);
        array->push(2);
        array->push(3);
    }

    {
        auto array = top->setChild("arr2", toml::Array());
        array->push("red");
        array->push("yellow");
        array->push("green");
    }

    {
        auto array = top->setChild("arr3", toml::Array());
        auto nested1 = array->push((toml::Array()));
        nested1->push(1);
        nested1->push(2);

        auto nested2 = array->push(toml::Array());
        nested2->push(3);
        nested2->push(4);
        nested2->push(5);
    }

    {
        auto array = top->setChild("arr4", toml::Array());
        array->push("all");
        array->push("strings");
        array->push("are the same");
        array->push("type");
    }

    {
        auto array = top->setChild("arr5", toml::Array());
        auto nested1 = array->push((toml::Array()));
        nested1->push(1);
        nested1->push(2);

        auto nested2 = array->push(toml::Array());
        nested2->push("a");
        nested2->push("b");
        nested2->push("c");
    }

    {
        auto array = top->setChild("arr7", toml::Array());
        array->push(1);
        array->push(2);
        array->push(3);
    }

    {
        auto array = top->setChild("arr8", toml::Array());
        array->push(1);
        array->push(2);
    }

    return root;
}

inline toml::Value build_array_table_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    auto widgets = top->setChild("widgets", toml::Array());
    {
        auto widget = widgets->push(toml::Table());
        widget->setChild("type", "image");
        widget->setChild("width", 1000);

        auto topics = widget->setChild("topics", toml::Array());
        {
            auto topic = topics->push(toml::Table());
            topic->setChild("topic", "some");
            topic->setChild("count", 3);
        }

        {
            auto topic = topics->push(toml::Table());
            topic->setChild("topic", "something");
            topic->setChild("count", 4);
        }
    }

    {
        auto widget = widgets->push(toml::Table());
        widget->setChild("type", "foo");
        widget->setChild("width", 2000);

        auto topics = widget->setChild("topics", toml::Array());
        {
            auto topic = topics->push(toml::Table());
            topic->setChild("topic", "bar");
            topic->setChild("count", 5);
        }
    }

    return root;
}

inline toml::Value build_array_table_02(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    auto widgets = top->setChild("widgets", toml::Table());
    widgets->setChild("type", "image");
    widgets->setChild("width", 1000);

    auto topics = widgets->setChild("topics", toml::Array());
    {
        auto topic = topics->push(toml::Table());
        topic->setChild("topic", "some");
        topic->setChild("count", 3);

    }
    {
        auto topic = topics->push(toml::Table());
        topic->setChild("topic", "something");
        topic->setChild("count", 4);
    }

    return root;
}

inline toml::Value build_boolean_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    top->setChild("bool1", true);
    top->setChild("bool2", false);
    top->setChild("true", true);
    top->setChild("false", false);

    return root;
}

inline toml::Value build_datetime_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    {
        std::tm t;
        t.tm_sec = 0;
        t.tm_min = 32;
        t.tm_hour = 7;
        t.tm_mday = 27;
        t.tm_mon = 5 - 1;
        t.tm_year = 1979 - 1900;
        auto tp = std::chrono::system_clock::from_time_t(timegm(&t));

        top->setChild("date1", tp);
    }

    {
        std::tm t;
        t.tm_sec = 0;
        t.tm_min = 32;
        t.tm_hour = 7;
        t.tm_mday = 27;
        t.tm_mon = 5 - 1;
        t.tm_year = 1979 - 1900;
        auto tp = std::chrono::system_clock::from_time_t(timegm(&t));

        top->setChild("date2", tp);
    }


    {
        std::tm t;
        t.tm_sec = 0;
        t.tm_min = 32;
        t.tm_hour = 7;
        t.tm_mday = 27;
        t.tm_mon = 5 - 1;
        t.tm_year = 1979 - 1900;
        auto tp = std::chrono::system_clock::from_time_t(timegm(&t));
        tp += std::chrono::microseconds(999999);

        top->setChild("date3", tp);
    }

    {
        std::tm t;
        t.tm_sec = 0;
        t.tm_min = 0;
        t.tm_hour = 0;
        t.tm_mday = 27;
        t.tm_mon = 5 - 1;
        t.tm_year = 1979 - 1900;
        auto tp = std::chrono::system_clock::from_time_t(timegm(&t));

        top->setChild("date4", tp);
    }

    return root;
}

inline toml::Value build_float_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    top->setChild("flt1", 1.0);
    top->setChild("flt2", 3.1415);
    top->setChild("flt3", -0.01);

    top->setChild("flt4", 5e+22);
    top->setChild("flt5", 1e6);
    top->setChild("flt6", -2e-2);

    // XXX: Too low? Both writer & parser sets -0 and 0 respectively.
    top->setChild("flt7", -6.626e-34);

    // XXX: Format: 9_224_617.445_991_228_313
    top->setChild("flt8", 9224617.445991228313);
    // XXX: Format: 1e1_000
    // TODO: Express 1e+1000 - sat to zero for now until someone uses this (no test does)
    // XXX: Too high? Results in inf for value type double (parser is able. Hmm)
    top->setChild("flt9", 0.0);

    return root;
}

inline toml::Value build_inlinetable_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    {
        // XXX: format: inline
        auto table = top->setChild("name", toml::Table());
        table->setChild("first", "Tom");
        table->setChild("last", "Preston-Werner");
    }

    {
        // XXX: format: inline
        auto table = top->setChild("point", toml::Table());
        table->setChild("x", 1);
        table->setChild("y", 2);
    }

    return root;
}

inline toml::Value build_integer_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    top->setChild("int1", 99);
    top->setChild("int2", 42);
    top->setChild("int3", 0);
    top->setChild("int4", -17);
    //XXX: format: 1_000
    top->setChild("int5", 1000);
    //XXX: format: 5_349_221
    top->setChild("int6", 5349221);
    //XXX: format: 1_2_3_4_5
    top->setChild("int7", 12345);

    return root;
}

inline toml::Value build_string_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    top->setChild("s1", u8"I'm a string. \"You can quote me\". Name\tJos\u00E9\nLocation\tSF.");
    top->setChild("s2", "Roses are red\nViolets are blue");

    // These three strings are equivalent values, only represented differently on disk (parser).
    top->setChild("str1", "The quick brown fox jumps over the lazy dog.");
    top->setChild("str2", "The quick brown fox jumps over the lazy dog.");
    top->setChild("str3", "The quick brown fox jumps over the lazy dog.");

    // XXX: format: single quote value
    top->setChild("winpath", "C:\\Users\\nodejs\\templates");
    // XXX: format: single quote value
    top->setChild("winpath2", "\\\\ServerX\\admin$\\system32\\");
    // XXX: format: single quote value
    top->setChild("quoted", "Tom \"Dubs\" Preston-Werner");
    // XXX: format: single quote value
    top->setChild("regex", "<\\i\\c*\\s*>");

    // XXX: format: tripple single quotes: one-line
    top->setChild("regex2", "I [dw]on't need \\d{2} apples");
    // XXX: format: tripple single quotes: separate lines
    top->setChild("lines", "The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n");

    return root;
}

inline toml::Value build_table_01(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    auto table = top->setChild("table", toml::Table());
    table->setChild("key", "value");
    table->setChild("bare_key", "value");
    table->setChild("bare-key", "value");
    table->setChild("1234", "bare integer");

    table->setChild("127.0.0.1", "value");
    table->setChild("character encoding", "value");
    table->setChild("ʎǝʞ", "value");

    return root;
}

inline toml::Value build_table_02(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;

    auto table = top->setChild("table", toml::Table());
    auto taterman = table->setChild("tater.man", toml::Table());

    taterman->setChild("type", "pug");

    auto a = top->setChild("a", toml::Table());
    auto b = a->setChild("b", toml::Table());
    auto c = b->setChild("c", toml::Table());
    c->setChild("x", 1);

    auto d = top->setChild("d", toml::Table());
    auto e = d->setChild("e", toml::Table());
    auto f = e->setChild("f", toml::Table());
    f->setChild("x", 1);

    auto g = top->setChild("g", toml::Table());
    auto h = g->setChild("h", toml::Table());
    auto i = h->setChild("i", toml::Table());
    i->setChild("x", 1);

    auto j = top->setChild("j", toml::Table());
    auto k = j->setChild("ʞ", toml::Table());
    auto l = k->setChild("l", toml::Table());
    l->setChild("x", 1);

    return root;
}

inline toml::Value build_table_03(void)
{
    toml::Value root((toml::Table()));
    toml::Value* top = &root;
    auto array = top->setChild("a", toml::Array());

    {
        auto a = array->push(toml::Table());

        auto b = a->setChild("b", toml::Table());
        b->setChild("x", 1);

    }

    {
        auto a = array->push(toml::Table());

        auto b = a->setChild("b", toml::Table());
        b->setChild("x", 2);

    }

    return root;
}

#endif // TOML_TEST_BUILD_H
