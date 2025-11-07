![Ubuntu](https://github.com/pfeatherstone/msgpack_cpp/actions/workflows/ubuntu.yml/badge.svg)
![MacOS](https://github.com/pfeatherstone/msgpack_cpp/actions/workflows/macos.yml/badge.svg)
![Windows](https://github.com/pfeatherstone/msgpack_cpp/actions/workflows/windows.yml/badge.svg)

# msgpack_cpp
C++ header-only msgpack library that supports (de)serializing types directly, including custom types, as well as a dicitionary type `msgpackcpp::value` a la `nlohmann::json` or `boost::json::value`. 

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

Option 1 : define `serialize()` and `deserialize()` functions in the same namespace as your custom struct

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

Option 2 : use Boost.Describe to describe your struct

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
    serialize(out, a);

    // Deserialize
    auto in = source(buf);
    deserialize(in, b);
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

## Documentation

There is none. Hopefully the code is readable.