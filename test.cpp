#define BOOST_TEST_MODULE Msgpack
#include <random>
#include <vector>
#include <limits>
#include <string>
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <boost/describe/class.hpp>
#include <msgpack.hpp>
#include "msgpack.h"
#include "msgpack_sinks.h"
#include "msgpack_describe.h"

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

namespace custom_namespace
{
    struct custom_struct1
    {
        int64_t             my_int{};
        double              my_float{};
        std::string         my_str;
        std::vector<float>  my_vec;
    };

    BOOST_DESCRIBE_STRUCT(custom_struct1, (), (
        my_int,
        my_float,
        my_str,
        my_vec
    ))

    struct custom_struct2
    {
        std::vector<custom_struct1> my_vec;
    };

    BOOST_DESCRIBE_STRUCT(custom_struct2, (), (my_vec))

    struct custom_struct3
    {
        int64_t             my_int{};
        double              my_float{};
        std::string         my_str;
        std::vector<float>  my_vec;
        custom_struct2      my_struct;
    };

    template<class Stream>
    void serialize(Stream&& out, const custom_struct3& obj)
    {
        serialize_all(std::forward<Stream>(out), obj.my_int, obj.my_float, obj.my_str, obj.my_vec, obj.my_struct);
    }
}

BOOST_AUTO_TEST_CASE(test_serialize_basic_types)
{
    std::mt19937 eng(std::random_device{}());
    std::vector<uint8_t> buf1, buf2, buf3;

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

            serialize_all(sink(buf3), a, b, c, d, e, f, g, h, i, j);
        }

        BOOST_TEST_REQUIRE(num_errors(buf1, buf2) == 0);
        BOOST_TEST_REQUIRE(num_errors(buf1, buf3) == 0);
        buf1.clear();
        buf2.clear();
        buf3.clear();
    }
}

BOOST_AUTO_TEST_CASE(test_serialize_string_and_binary_arrays)
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
        using msgpackcpp::serialize;
        using msgpackcpp::sink;
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

    BOOST_TEST_REQUIRE(num_errors(buf1, buf2) == 0);
}

BOOST_AUTO_TEST_CASE(test_serialize_maps)
{
    std::mt19937 eng(std::random_device{}());
    std::vector<uint8_t> buf1, buf2;

    std::map<std::string, int> a;
    a["a"] = 1;
    a["b"] = 2;
    a["c"] = 70000;
    a["d"] = 1000000000;

    std::unordered_map<uint64_t, std::string> b;
    b[1]                = "small int";
    b[257]              = "medium int";
    b[70000]            = "big int";
    b[5000000000ull]    = "very big int";

    {
        // using msgpack-c library
        vector_sink sink{buf1};
        msgpack::pack(sink, a);
        msgpack::pack(sink, b);
    }

    {
        // using custom library
        using namespace msgpackcpp;
        serialize(sink(buf2), a);
        serialize(sink(buf2), b);
    }

    BOOST_TEST_REQUIRE(num_errors(buf1, buf2) == 0);
}

BOOST_AUTO_TEST_CASE(test_serialize_custom_struct)
{
    std::mt19937 eng(std::random_device{}());
    std::vector<uint8_t> buf1, buf2;

    custom_namespace::custom_struct1 a;
    a.my_int      = random_int<int64_t>(eng);
    a.my_float    = random_float<float>(eng);
    a.my_str      = "Hello there!";
    a.my_vec.resize(1024);
    std::generate(begin(a.my_vec), end(a.my_vec), [&]{return random_float<float>(eng);});

    {
        // using msgpack-c library
        vector_sink sink{buf1};
        msgpack::packer pack{&sink};
        pack.pack_array(4);
        pack.pack(a.my_int);
        pack.pack(a.my_float);
        pack.pack(a.my_str);
        pack.pack(a.my_vec);
    }

    {
        // using custom library
        using namespace msgpackcpp;
        serialize(sink(buf2), a);
    }

    BOOST_TEST_REQUIRE(num_errors(buf1, buf2) == 0);

    buf1.clear();
    buf2.clear();

    {
        // using msgpack-c library
        vector_sink sink{buf1};
        msgpack::packer pack{&sink};
        pack.pack_map(4);
        pack.pack("my_int");
        pack.pack(a.my_int);
        pack.pack("my_float");
        pack.pack(a.my_float);
        pack.pack("my_str");
        pack.pack(a.my_str);
        pack.pack("my_vec");
        pack.pack(a.my_vec);
    }

    {
        // using custom library
        using namespace msgpackcpp;
        serialize(sink(buf2), a, true);
    }

    BOOST_TEST_REQUIRE(num_errors(buf1, buf2) == 0);

    custom_namespace::custom_struct2 b;
    b.my_vec.push_back(a);
    b.my_vec.push_back(a);

    custom_namespace::custom_struct3 c;
    c.my_int    = random_int<int64_t>(eng);
    c.my_float  = random_float<float>(eng);
    c.my_str    = "I have the high ground";
    c.my_vec.resize(10, 2);
    c.my_struct = b;

    buf2.clear();

    {
        // using custom library
        using namespace msgpackcpp;
        serialize(sink(buf2), c);
    }

    BOOST_TEST_REQUIRE(buf2.size() > 0lu);
}

BOOST_AUTO_TEST_CASE(test_deserialize_basic_types)
{
    std::mt19937 eng(std::random_device{}());
    std::vector<uint8_t> buf;

    for (int repeat = 0 ; repeat < 100000 ; ++repeat)
    {
        const bool          a = std::uniform_int_distribution<int>{0,1}(eng) == 1;
        const uint8_t       b = random_int<uint8_t>(eng);
        // const int8_t        b = random_int<int8_t>(eng);
        // const uint16_t      c = random_int<uint16_t>(eng);
        // const int16_t       d = random_int<int16_t>(eng);
        // const uint32_t      e = random_int<uint32_t>(eng);
        // const int32_t       f = random_int<int32_t>(eng);
        // const uint64_t      g = random_int<uint64_t>(eng);
        // const int64_t       h = random_int<int64_t>(eng);
        // const float         i = random_float<float>(eng);
        // const double        j = random_float<double>(eng);

        using namespace msgpackcpp;
        auto out = sink(buf);
        serialize(out, a);
        serialize(out, b);

        bool    aa{};
        uint8_t bb{};
        auto in = source(buf);
        deserialize(in, aa);
        deserialize(in, bb);

        BOOST_TEST_REQUIRE(a == aa);
        BOOST_TEST_REQUIRE(b == bb);
        buf.clear();
    }
}