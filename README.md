# tinytoml

A header only C++11 library for parsing TOML

## Prerequisite

- C++11 compiler
- C++11 libraries.

I've only checked this library works with clang++ and g++. I didn't check this with cl.exe.

## How to use

Copy include/toml/toml.h into your project, and include it from your source.

Example code

    std::ifstream ifs("foo.toml");
    toml::Parser parser(ifs);

    toml::Value v = parser.parse();
    // If toml file is valid, v would be valid.
    if (!v.valid()) {
        cout << "some error occured";
        return;
    }

    // you get a value using get().
    // If type error occured, your program will die.
    cout << v.get<string>("foo.bar") << endl;

    // you can find a value by find().
    // If found, non-null pointer will be returned.
    const toml::Value* x = v.find("bar");

    // you can check the type of value with is().
    // you can get the inner value by as().
    if (x && x->is<string>()) {
        cout << x->as<string>() << endl;
    }


## How to test

The directory 'src' contains a few tests. We're using google testing framework, and cmake.

    $ mkdir -p out/Debug; cd out/Debug
    $ cmake ../../src
    $ make
    $ make test

'src' also contains a small example for how to use this.

