# tinytoml

A header only C++11 library for parsing TOML.

[![Build Status](https://travis-ci.org/mayah/tinytoml.svg?branch=master)](https://travis-ci.org/mayah/tinytoml)

This parser is based on TOML [v0.2.0](https://github.com/mojombo/toml/blob/master/versions/toml-v0.2.0.md).
This library is distributed under simplified BSD License.

## Introduction

tinytoml is a tiny [TOML](https://github.com/toml-lang/toml) parser for C++11 with following properties:
- header file only
- C++11 library friendly (array is std::vector, table is std::map, time is std::chrono::system_clock::time_point).
- no external dependencies (note: we're using cmake for testing, but it's not required to use this library).

We'd like to keep this library as handy as possible.

## Prerequisite

- C++11 compiler
- C++11 libraries.

I've only checked this library works with recent clang++ (3.5) and g++ (4.7). I didn't check this with cl.exe.

## How to use

Copy include/toml/toml.h into your project, and include it from your source. That's all.

## Example code

```c++
std::ifstream ifs("foo.toml");
toml::Parser parser(ifs);

toml::Value v = parser.parse();
// If toml file is valid, v would be valid.
// Otherwise, you can get an error reason by calling Parser::errorReason().
if (!v.valid()) {
    cout << parser.errorReason() << endl;
    return;
}

// You can get a value using get().
// If type error occured, std::runtime_error is raised.
cout << v.get<string>("foo.bar") << endl;

// You can find a value by find().
// If found, non-null pointer will be returned.
// You can check the type of value with is().
// You can get the inner value by as().
const toml::Value* x = v.find("bar");
if (x && x->is<std::string>()) {
    cout << x->as<string>() << endl;
} else if (x && x->is<int>()) {
    cout << x->as<int>() << endl;
}

// Note: the inner value of integer value is actually int64_t,
// however, you can use 'int' for convenience.
const toml::Value z = ...;
int x = z->as<int>();
int y = z->as<int64_t>();
// toml::Array is actually std::vector<toml::Value>.
// So, you can use range based for, etc.
const toml::Array& ar = z->as<toml::Array>();
for (const toml::Value& v : ar) {
    ...
}
```

## How to test

The directory 'src' contains a few tests. We're using google testing framework, and cmake.

```sh
$ mkdir -p out/Debug; cd out/Debug
$ cmake ../../src
$ make
$ make test
```

'src' also contains a small example for how to use this.
