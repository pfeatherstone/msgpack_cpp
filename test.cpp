#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <random>
#include <vector>
#include <limits>
#include <string>
#include <algorithm>
#include <msgpack.hpp>
#include <doctest.h>
#include "msgpack.h"
#include "msgpack_sinks.h"

template<class Byte, class Allocator>
struct vector_sink
{
    std::vector<Byte, Allocator>& buf;

    vector_sink(std::vector<Byte, Allocator>& buf_) : buf{buf_} {}

    void write(const char* data, size_t len)
    {
        buf.insert(end(buf), data, data + len);
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
    TEST_CASE("basic types")
    {
        std::mt19937 eng(std::random_device{}());
        std::vector<uint8_t> buf1, buf2;

        for (int repeat = 0 ; repeat < 100000 ; ++repeat)
        {
            const uint8_t       a = random_int<uint8_t>(eng);
            const int8_t        b = random_int<int8_t>(eng);
            const uint16_t      c = random_int<uint16_t>(eng);
            const int16_t       d = random_int<int16_t>(eng);
            const uint32_t      e = random_int<uint32_t>(eng);
            const int32_t       f = random_int<int32_t>(eng);
            const uint64_t      g = random_int<uint64_t>(eng);
            const int64_t       h = random_int<int64_t>(eng);
            const float         i = random_float<float>(eng);
            const double        j = random_float<double>(eng);

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
                using namespace msgpackcpp;
                serialize(sink(buf2), a);
                serialize(sink(buf2), b);
                serialize(sink(buf2), c);
                serialize(sink(buf2), d);
                serialize(sink(buf2), e);
                serialize(sink(buf2), f);
                serialize(sink(buf2), g);
                serialize(sink(buf2), h);
                serialize(sink(buf2), i);
                serialize(sink(buf2), j);
            }

            REQUIRE(num_errors(buf1, buf2) == 0);
            buf1.clear();
            buf2.clear();
        }
    }

    TEST_CASE("string and bin array")
    {
        std::mt19937 eng(std::random_device{}());
        std::vector<uint8_t> buf1, buf2;

        const std::string   k = "hello there!";
        const std::string   l = "Hi there, this string is designed to be greater than 32 bytes long";
        const std::string   m = "He sighed, and took up the volume again, and tried to forget. He read"
                                "of the swallows that fly in and out of the little _café_ at Smyrna"
                                "where the Hadjis sit counting their amber beads and the turbaned"
                                "merchants smoke their long tasselled pipes";
        const std::string   n = "and talk gravely to each"
                                "other; he read of the Obelisk in the Place de la Concorde that weeps"
                                "tears of granite in its lonely sunless exile and longs to be back by"
                                "the hot, lotus-covered Nile, where there are Sphinxes, and rose-red"
                                "ibises, and white vultures with gilded claws, and crocodiles with small"
                                "beryl eyes that crawl over the green steaming mud; he began to brood"
                                "over those verses which, drawing music from kiss-stained marble, tell"
                                "of that curious statue that Gautier compares to a contralto voice, the"
                                "_monstre charmant_ that couches in the porphyry-room of the Louvre."
                                "But after a time the book fell from his hand. He grew nervous, and a"
                                "horrible fit of terror came over him. What if Alan Campbell should be"
                                "out of England? Days would elapse before he could come back. Perhaps he"
                                "might refuse to come. What could he do then? Every moment was of vital"
                                "importance.";
        std::string o(70000, 0);
        std::generate(begin(o), end(o), [&]{return std::uniform_int_distribution<char>{}(eng);});

        std::vector<char>    p(255);
        std::vector<uint8_t> q(255);
        std::vector<int8_t>  r(255);
        std::vector<char>    s(1000);
        std::vector<uint8_t> t(1000);
        std::vector<int8_t>  u(1000);
        std::vector<char>    v(70000);
        std::vector<uint8_t> w(70000);
        std::vector<int8_t>  x(70000);
        std::generate(begin(p), end(p), [&]{return random_int<char>(eng);});
        std::generate(begin(q), end(q), [&]{return random_int<uint8_t>(eng);});
        std::generate(begin(r), end(r), [&]{return random_int<int8_t>(eng);});
        std::generate(begin(s), end(s), [&]{return random_int<char>(eng);});
        std::generate(begin(t), end(t), [&]{return random_int<uint8_t>(eng);});
        std::generate(begin(u), end(u), [&]{return random_int<int8_t>(eng);});
        std::generate(begin(v), end(v), [&]{return random_int<char>(eng);});
        std::generate(begin(w), end(w), [&]{return random_int<uint8_t>(eng);});
        std::generate(begin(x), end(x), [&]{return random_int<int8_t>(eng);});

        {
            // using msgpack-c library
            vector_sink sink{buf1};
            msgpack::packer pack{&sink};
            pack.pack(k);
            pack.pack(l);
            pack.pack(m);
            pack.pack(n);
            pack.pack(o);
            pack.pack(p);
            pack.pack(q);
            pack.pack(r);
            pack.pack(s);
            pack.pack(t);
            pack.pack(u);
            pack.pack(v);
            pack.pack(w);
            pack.pack(x);
        }

        {
            // using custom library
            using namespace msgpackcpp;
            serialize(sink(buf2), k);
            serialize(sink(buf2), l);
            serialize(sink(buf2), m);
            serialize(sink(buf2), n);
            serialize(sink(buf2), o);
            serialize(sink(buf2), p);
            serialize(sink(buf2), q);
            serialize(sink(buf2), r);
            serialize(sink(buf2), s);
            serialize(sink(buf2), t);
            serialize(sink(buf2), u);
            serialize(sink(buf2), v);
            serialize(sink(buf2), w);
            serialize(sink(buf2), x);
        }

        REQUIRE(num_errors(buf1, buf2) == 0);
    }

    TEST_CASE("map")
    {
        std::mt19937 eng(std::random_device{}());
        
        std::map<std::string, int> a;
        a["a"] = 1;
        a["b"] = 2;
        a["c"] = 70000;
        a["d"] = 1000000000;

        std::vector<uint8_t> buf1, buf2;

        {
            // using msgpack-c library
            vector_sink sink{buf1};
            msgpack::pack(sink, a);
        }

        {
            // using custom library
            using namespace msgpackcpp;
            serialize(sink(buf2), a);
        }

        REQUIRE(num_errors(buf1, buf2) == 0);
    }
}