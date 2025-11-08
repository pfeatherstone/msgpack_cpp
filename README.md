![Ubuntu](https://github.com/pfeatherstone/msgpack_cpp/actions/workflows/ubuntu.yml/badge.svg)
![MacOS](https://github.com/pfeatherstone/msgpack_cpp/actions/workflows/macos.yml/badge.svg)
![Windows](https://github.com/pfeatherstone/msgpack_cpp/actions/workflows/windows.yml/badge.svg)

# msgpack_cpp
C++ header-only msgpack library that supports (de)serializing types directly, including custom types, as well as a dicitionary type `msgpackcpp::value` à la `nlohmann::json` or `boost::json::value`. 

## Examples

### Basic types

```cpp
#include "msgpack.h"
#include "msgpack_sinks.h"

int main()
{
    using msgpackcpp::serialize;
    using msgpackcpp::deserialize;
    using msgpackcpp::sink;
    using msgpackcpp::source;

    // Some data
    int                         a = 1, aa;
    double                      b = 3.15, bb;
    std::string                 c = "hello there", cc;
    std::vector<char>           d = {0,1,2,3,4,5,6,7,8,9}, dd;
    std::map<std::string, int>  e = {{"a", 1}, {"b", 2}}, ee;


    // Serialize
    std::vector<char> buf;
    auto out = sink(buf);
    serialize(out, a);
    serialize(out, b);
    serialize(out, c);
    serialize(out, d);
    serialize(out, e);

    // Deserialize
    auto in = source(buf);
    deserialize(in, aa);
    deserialize(in, bb);
    deserialize(in, cc);
    deserialize(in, dd);
    deserialize(in, ee);
}
```

### Custom types

Option 1 : define `serialize()` and `deserialize()` functions in the same namespace as your custom struct. 
Note the use of std::tie. std::tuple is serialized like a msgpack [array](https://github.com/msgpack/msgpack/blob/master/spec.md#array-format-family). 
This ensures your custom type is serialized into a single msgpack object.

```cpp
#include "msgpack.h"
#include "msgpack_sinks.h"

namespace mynamespace
{
    struct my_struct1
    {
        int                 my_int{};
        float               my_float{};
        std::string         my_string;
        std::vector<short>  my_audio;
    };

    template<SINK_TYPE Sink>
    void serialize(Sink& out, const my_struct& obj)
    {
        auto packed = std::tie(obj.my_int, obj.my_float, obj.my_string, obj.my_audio);
        serialize(out, packed);
    }

    template<SOURCE_TYPE Source>
    void deserialize(Source& in, my_struct& obj)
    {
        auto packed = std::tie(obj.my_int, obj.my_float, obj.my_string, obj.my_audio);
        deserialize(in, packed);
    }  
}

int main()
{
    using msgpackcpp::serialize;
    using msgpackcpp::deserialize;
    using msgpackcpp::sink;
    using msgpackcpp::source;

    mynamespace::my_struct a = {1, 3.14, "hello there", {0, 1, 2, 3, 4}};
    mynamespace::my_struct b;

    // Serialize
    std::stringstream buf;
    auto out = sink(buf);
    serialize(out, a);

    // Deserialize
    auto in = source(buf);
    deserialize(in, b);
}
```

Option 2 : use Boost.Describe to describe your struct. This automatically makes the following functions available:
* `serialize(Sink& out, const CustomType& obj, bool as_map = false)`
* `deserialize(Source& in, CustomType& obj, bool as_map = false)`

When `as_map == false` then you get exactly the same behaviour as above. When `as_map == true` your type is serialized like a msgpack [map](https://github.com/msgpack/msgpack/blob/master/spec.md#map-format-family) where the keys are the member variable names of your struct.


```cpp
#include <boost/describe/class.hpp>
#include "msgpack.h"
#include "msgpack_sinks.h"
#include "msgpack_describe.h"

namespace mynamespace
{
    struct my_struct1
    {
        int                 my_int{};
        float               my_float{};
        std::string         my_string;
        std::vector<short>  my_audio;
    };

    BOOST_DESCRIBE_STRUCT(my_struct, (), (my_int, my_float, my_string, my_audio))
}

int main()
{
    using msgpackcpp::serialize;
    using msgpackcpp::deserialize;
    using msgpackcpp::sink;
    using msgpackcpp::source;

    mynamespace::my_struct a = {1, 3.14, "hello there", {0, 1, 2, 3, 4}};
    mynamespace::my_struct b;

    // Serialize
    std::stringstream buf;
    auto out = sink(buf);
    serialize(out, a, /*as_map=*/false); // serialize to a msgpack array
    serialize(out, a, /*as_map=*/true);  // serialize to a msgpack map

    // Deserialize
    auto in = source(buf);
    deserialize(in, b, /*as_map=*/false);
    deserialize(in, b, /*as_map=*/true);
}
```

### Dictionary value

```cpp
#include "msgpack.h"
#include "msgpack_sinks.h"

const auto REQUIRE = [](auto res) {if (!res)throw std::runtime_error("bad");};

int main()
{
    using msgpackcpp::serialize;
    using msgpackcpp::deserialize;
    using msgpackcpp::sink;
    using msgpackcpp::source;

    // Data
    msgpackcpp::value jv0 = {
        {"pi", 3.141},
        {"happy", true},
        {"name", "Niels"},
        {"nothing", nullptr},
        {"answer", {
            {"everything", -42}
        }},
        {"list", {1, 0, 2}},
        {"object", {
            {"currency", "USD"},
            {"value", 42.99}
        }}
    };

    // Serialize (pack)
    std::vector<char> buf;
    auto out = sink(buf);
    jv0.pack(out);

    // Deserialize (unpack)
    msgpackcpp::value jv1;
    auto in = source(buf);
    jv1.unpack(in);

    // Check
    REQUIRE(jv1.is_object());
    REQUIRE(jv1.size() == 7);
    REQUIRE(jv1.at("pi").as_real() == 3.141);
    REQUIRE(jv1.at("happy").as_bool() == true);
    REQUIRE(jv1.at("name").as_str() == "Niels");
    REQUIRE(jv1.at("nothing").is_null());
    REQUIRE(jv1.at("answer").at("everything").as_int64() == -42);
    REQUIRE(jv1.at("list").as_array().size() == 3);
    REQUIRE(jv1.at("object").at("currency").as_str() == "USD");
    REQUIRE(jv1.at("object").at("value").as_real() == 42.99);
}
```

## Installation

Just copy the contents of the include folder in your project with `msgpack_describe.h` as an optional header.

## Dependencies

You just need a C++17 compiler. If you want to avail yourself of the convenient Boost.Describe integration in `msgpack_describe.h`, then you'll require that Boost library.

## API

The library provides the following functions for basic and STL types:

```cpp
namespace msgpackcpp
{
    template<SINK_TYPE Sink, class Type>
    void serialize(Sink& out, const Type& obj);

    template<SOURCE_TYPE Source, class Type>
    void deserialize(Source& in, Type& obj);
}
```

where:
* `out` is a templated function object with signature `void(const char* data, size_t len)` which writes serialized data.
* `in` is a templated function object with signature `void(char* data, size_t len)` which reads serialized data.

When c++20 is enabled `SINK_TYPE` and `SOURCE_TYPE` are concepts which check those signatures.

The library also provides functions `sink()` and `source()` which take a type and return function objects (c++ lambda) which satisfy the `SINK_TYPE` and `SOURCE_TYPE` concepts. Currently overloads for `std::vector<char>` and `std::iostream` are provided though users can write their own sink/source types.

This library also provides a dictionary type `msgpackcpp::value` very similar to [nlohmann::json](https://json.nlohmann.me/api/basic_json/) or `boost::json::value` which can be (de)serialized using member functions `.pack()` and `.unpack()`.
Conversions from `msgpackcpp::value` to and from custom types is not supported and discouraged. This library allows you to serialize and deserialized types directly without having to go through `msgpackcpp::value`.

## Documentation

There is none. Hopefully the code is readable.