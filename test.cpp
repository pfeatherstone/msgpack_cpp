#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <random>
#include <vector>
#include <limits>
#include <msgpack.hpp>
#include <doctest.h>
#include "msgpack.h"

template<class Byte, class Allocator>
struct vector_sink
{
    std::vector<Byte, Allocator>& buf;

    vector_sink(std::vector<Byte, Allocator>& buf_) : buf{buf_} {}

    void write(const char* data, size_t len)
    {
        buf.insert(end(buf), data, data + len);
    }

    void operator()(const char* data, size_t len)
    {
        write(data, len);
    }
};

const auto num_errors = [](const auto& buf1, const auto& buf2)
{
    int errors{0};
    size_t i{0};
    for (; i < std::min(buf1.size(), buf2.size()) ; ++i)
        if (buf1[i] != buf2[i])
            ++errors;
    for(; i < std::max(buf1.size(), buf2.size()); ++i)
        ++errors;
    return errors;
};

template<class Int, class Generator>
Int random_int(Generator& gen)
{
    return std::uniform_int_distribution<Int>{std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max()}(gen);
}

template<class Float, class Generator>
Float random_float(Generator& gen)
{
    return std::uniform_real_distribution<Float>{std::numeric_limits<Float>::min(), std::numeric_limits<Float>::max()}(gen);
}

TEST_SUITE("[MSGPACK]")
{
    TEST_CASE("arithmetic types")
    {
        std::mt19937 eng(std::random_device{}());

        std::vector<uint8_t> buf1, buf2;

        for (int repeat = 0 ; repeat < 1000 ; ++repeat)
        {
            const uint8_t  a = random_int<uint8_t>(eng);
            const int8_t   b = random_int<int8_t>(eng);
            const uint16_t c = random_int<uint16_t>(eng);
            const int16_t  d = random_int<int16_t>(eng);
            const uint32_t e = random_int<uint32_t>(eng);
            const int32_t  f = random_int<int32_t>(eng);
            const uint64_t g = random_int<uint64_t>(eng);
            const int64_t  h = random_int<int64_t>(eng);
            const float    i = random_float<float>(eng);
            const double   j = random_float<double>(eng);

            {
                // using msgpack-c library
                vector_sink sink{buf1};
                msgpack::packer pack{&sink};
                pack.pack(a);
                pack.pack(b);
                pack.pack(c);
                pack.pack(d);
                pack.pack(e);
                pack.pack(f);
                pack.pack(g);
                pack.pack(h);
                pack.pack(i);
                pack.pack(j);
            }

            {
                // using custom library
                vector_sink sink{buf2};
                msgpackcpp::serialize(sink, a);
                msgpackcpp::serialize(sink, b);
                msgpackcpp::serialize(sink, c);
                msgpackcpp::serialize(sink, d);
                msgpackcpp::serialize(sink, e);
                msgpackcpp::serialize(sink, f);
                msgpackcpp::serialize(sink, g);
                msgpackcpp::serialize(sink, h);
                msgpackcpp::serialize(sink, i);
                msgpackcpp::serialize(sink, j);
            }

            REQUIRE(num_errors(buf1, buf2) == 0);
            buf1.clear();
            buf2.clear();
        }
    }
}