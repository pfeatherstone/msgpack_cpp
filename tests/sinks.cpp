#include <numeric>
#include <sstream>
#include "doctest.h"
#include "msgpack.h"
#include "msgpack_sinks.h"

using namespace std;
using namespace std::literals::string_view_literals;
using namespace msgpackcpp;

TEST_SUITE("[SINKS]") 
{
    TEST_CASE("vector and stringstream")
    {
        std::vector<int> a(10);
        std::iota(begin(a), end(a), 0);
        std::map<std::string, int> b = {{"a", 1}, {"b", 2}};
        std::tuple<int, float, std::string> c(1, 3.14, "Hello there!");

        std::vector<char> buf0;
        std::stringstream buf1;

        const auto run = [&](auto& buf)
        {
            auto out = sink(buf);
            serialize(out, a);
            serialize(out, b);
            serialize(out, c);
        };

        run(buf0);
        run(buf1);
        std::string buf1_str = buf1.str();
        REQUIRE(buf0.size() == buf1_str.size());
        REQUIRE(std::equal(begin(buf0), end(buf0), begin(buf1_str)));
    }
}