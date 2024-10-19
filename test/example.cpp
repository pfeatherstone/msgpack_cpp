#include <iostream>
#include <sstream>
#include <numeric>
#include <boost/describe/class.hpp>
#include <msgpack.h>
#include <msgpack_describe.h>
#include <msgpack_sinks.h>

void example1_tuple()
{
    using namespace msgpackcpp;

    std::tuple<int, float, std::string> a(1, 3.14, "Hello there!");

    std::vector<char> buf;

    // Creates a lambda which captures `buf` by reference and appends data to it when invoked
    auto out = sink(buf); 

    // The first argument can be any function object with signature void(const char* data, size_t len)
    serialize(out, a);

    // Creates a mutable lambda which captures `buf` by reference and reads data from it when invoked
    auto in = source(buf); 

    // The first argument can be any function object with signature void(char* data, size_t len)
    deserialize(in, a);
}

void example2_vector()
{
    using namespace msgpackcpp;

    std::vector<int> v1(10);
    std::iota(begin(v1), end(v1), 0);
    std::vector<int> v2;

    // You can also serialize into an ostream object
    std::ostringstream sout;
    auto out = sink(sout);
    serialize(out, v1);

    // Deserialize from an istream object
    std::istringstream sin(sout.str());
    auto in = source(sin);
    deserialize(in, v2);
}

void example3_map()
{
    using namespace msgpackcpp;

    std::map<std::string, int> a = {{"a", 1}, {"b", 2}};
    std::map<std::string, int> b;

    std::vector<char> buf;
    auto out = sink(buf);
    serialize(out, a);

    auto in = source(buf);
    deserialize(in, b);
}

namespace mynamespace
{
    struct my_struct1
    {
        int                 my_int{};
        float               my_float{};
        std::string         my_string;
        std::vector<short>  my_audio;
    };

    template<class Stream>
    void serialize(Stream& out, const my_struct1& obj)
    {
        using msgpackcpp::serialize;
        serialize(out, std::tie(obj.my_int, obj.my_float, obj.my_string, obj.my_audio));
    }

    template<class Source>
    void deserialize(Source& in, my_struct1& obj)
    {
        using msgpackcpp::deserialize;
        auto members = std::tie(obj.my_int, obj.my_float, obj.my_string, obj.my_audio);
        deserialize(in, members);
    }   
}

void example4_struct()
{
    using namespace msgpackcpp;

    mynamespace::my_struct1 a = {1, 3.14, "hello there", {0, 1, 2, 3, 4}};
    mynamespace::my_struct1 b;

    std::vector<char> buf;
    auto out = sink(buf);
    serialize(out, a);

    auto in = source(buf);
    deserialize(in, b);

    std::cout << b.my_int << ' ' << b.my_float << ' ' << b.my_string << ' ' << b.my_audio.size() << '\n';
}

namespace mynamespace2
{
    struct my_struct2
    {
        int                 my_int{};
        float               my_float{};
        std::string         my_string;
        std::vector<short>  my_audio;
    };

    BOOST_DESCRIBE_STRUCT(my_struct2, (), (my_int, my_float, my_string, my_audio)) 
}

void example5_struct()
{
    using namespace msgpackcpp;

    mynamespace2::my_struct2 a = {1, 3.14, "hello there", {0, 1, 2, 3, 4}};
    mynamespace2::my_struct2 b;

    std::vector<char> buf;
    auto out = sink(buf);
    serialize(out, a, /*as_map=*/true);

    auto in = source(buf);
    deserialize(in, b, /*as_map=*/true);

    std::cout << b.my_int << ' ' << b.my_float << ' ' << b.my_string << ' ' << b.my_audio.size() << '\n';
}

int main()
{
    example1_tuple();
    example2_vector();
    example3_map();
    example4_struct();
    example5_struct();
}