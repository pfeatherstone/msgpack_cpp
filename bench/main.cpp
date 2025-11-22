#include <chrono>
#include <random>
#include <boost/describe/class.hpp>
#include <msgpack.hpp>
#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"
#include "msgpack.h"
#include "msgpack_sinks.h"
#include "msgpack_describe.h"

using namespace std::chrono_literals;
using msgpackcpp::serialize;
using msgpackcpp::deserialize;
using msgpackcpp::sink;
using msgpackcpp::source;

template<class Byte, class Allocator>
struct vector_sink
{
    std::vector<Byte, Allocator>& buf;
    vector_sink(std::vector<Byte, Allocator>& buf_) : buf{buf_} {}
    void write(const char* data, size_t len) {buf.insert(end(buf), data, data + len);}
};

namespace custom_namespace
{
    struct custom_struct
    {
        char        c;
        int8_t      i8;
        uint8_t     u8;
        int16_t     i16;
        uint16_t    u16;
        int32_t     i32;
        uint32_t    u32;
        int64_t     i64;
        uint64_t    u64;
        float       f32;
        double      f64;
        std::string str;

        MSGPACK_DEFINE(c, i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, str)
    };

    BOOST_DESCRIBE_STRUCT(custom_struct, (), (c, i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, str))

    struct custom_struct2
    {
        std::vector<custom_struct>      array;
        std::map<int, custom_struct>    map;
        std::vector<uint8_t>            binary;
        MSGPACK_DEFINE(array, map, binary)
    };

    BOOST_DESCRIBE_STRUCT(custom_struct2, (), (array, map, binary))
}

std::string make_string()
{
    return  "and talk gravely to each"
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
}

template<class T>
void random(T& obj, std::mt19937_64& gen)
{
    if constexpr(std::is_same_v<T, char> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>)
        obj = std::uniform_int_distribution<int>{std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}(gen);
    else if constexpr(std::is_integral_v<T>)
        obj = std::uniform_int_distribution<T>{std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}(gen);
    else if constexpr (std::is_floating_point_v<T>)
        obj = std::uniform_real_distribution<T>{std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}(gen);
}

void random(custom_namespace::custom_struct& data, std::mt19937_64& eng)
{
    random(data.c, eng);
    random(data.i8, eng);
    random(data.u8, eng);
    random(data.i16, eng);
    random(data.u16, eng);
    random(data.i32, eng);
    random(data.u32, eng);
    random(data.i64, eng);
    random(data.u64, eng);
    random(data.f32, eng);
    random(data.f64, eng);
    data.str = make_string();
}

int main()
{
    std::mt19937_64 eng(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    custom_namespace::custom_struct2 data;
    data.array.resize(10);
    for (auto& obj : data.array) random(obj, eng);
    random(data.map[0], eng);
    random(data.map[-42], eng);
    random(data.map[100000], eng);
    data.binary.resize(1024);
    std::generate(begin(data.binary), end(data.binary), [&]{uint8_t v; random(v, eng); return v;});

    // Warmup
    std::vector<uint8_t> buf0;
    auto out = sink(buf0);
    serialize(out, data);

    std::vector<uint8_t> buf1;
    vector_sink out1(buf1);
    msgpack::pack(out1, data);
    printf("buf0.size() %zu buf1.size() %zu\n", buf0.size(), buf1.size());

    ankerl::nanobench::Bench().minEpochTime(100ms).epochs(20).run("msgpackcpp::deserialize", [&] {
        custom_namespace::custom_struct2 obj;
        auto in = source(buf0);
        deserialize(in, obj);
        // ankerl::nanobench::doNotOptimizeAway(d);
    });

    ankerl::nanobench::Bench().minEpochTime(100ms).epochs(20).run("msgpackcpp::serialize", [&] {
        buf0.clear();
        auto out = sink(buf0);
        serialize(out, data);
        // ankerl::nanobench::doNotOptimizeAway(d);
    });

    ankerl::nanobench::Bench().minEpochTime(100ms).epochs(20).run("msgpack_c::deserialize", [&] {
        msgpack::object_handle oh = msgpack::unpack((const char*)buf1.data(), buf1.size());
        custom_namespace::custom_struct2 obj = oh.get().as<custom_namespace::custom_struct2>();
        // ankerl::nanobench::doNotOptimizeAway(d);
    });

    ankerl::nanobench::Bench().minEpochTime(100ms).epochs(20).run("msgpack_c::serialize", [&] {
        buf1.clear();
        vector_sink out(buf1);
        msgpack::pack(out, data);
        // ankerl::nanobench::doNotOptimizeAway(d);
    });    
}