#include <cstdio>
#include <sstream>
#include <map>
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

    const auto run = [&](auto& buf)
    {
        // Serialize
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
    };

    // (De)serialize to vector
    std::vector<char> buf0;
    run(buf0);

    // (De)serialize to istream/ostream
    std::stringstream buf1;
    run(buf1);
}