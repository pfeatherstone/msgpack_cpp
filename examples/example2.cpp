#include <cstdint>
#include "msgpack.h"
#include "msgpack_sinks.h"

namespace mynamespace
{
    struct my_struct
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

    std::vector<char> buf;
    auto out = sink(buf);
    serialize(out, a);
    auto in = source(buf);
    deserialize(in, b);

    printf("Serialized buffer size %zu\n", buf.size());
    printf("%d %f %s [", b.my_int, b.my_float, b.my_string.c_str());
    for (auto s : b.my_audio) printf("%hd ", s);
    printf("]\n");
}