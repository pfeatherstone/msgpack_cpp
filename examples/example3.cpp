#include <cstdint>
#include <boost/describe/class.hpp>
#include "msgpack.h"
#include "msgpack_sinks.h"
#include "msgpack_describe.h"

namespace mynamespace
{
    struct my_struct
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

    std::vector<char> buf;
    auto out = sink(buf);
    serialize(out, a);
    auto in = source(buf);
    deserialize(in, b);

    printf("%d %f %s [", b.my_int, b.my_float, b.my_string.c_str());
    for (auto s : b.my_audio) printf("%hd ", s);
    printf("]\n");
}