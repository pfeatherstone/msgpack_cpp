#include <chrono>
#include <random>
#include <boost/describe/class.hpp>
#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"
#include "msgpack.h"
#include "msgpack_sinks.h"
#include "msgpack_describe.h"

using namespace std::chrono_literals;
using msgpackcpp::serialize;
using msgpackcpp::deserialize;
using msgpackcpp::sink;

template<class T, class Generator>
T random(Generator& gen)
{
    if constexpr(std::is_same_v<T, char> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>)
        return std::uniform_int_distribution<int>{std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}(gen);
    else if constexpr(std::is_integral_v<T>)
        return std::uniform_int_distribution<T>{std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}(gen);
    else if constexpr (std::is_floating_point_v<T>)
        return std::uniform_real_distribution<T>{std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}(gen);
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
    };

    BOOST_DESCRIBE_STRUCT(custom_struct, (), (
        c, i8, u8, i16, u16, i32, u32, i64, u64, f32, f64,
        str
    ))
}

int main()
{
    std::mt19937_64 eng(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    custom_namespace::custom_struct data;
    data.c      = random<char>(eng);
    data.i8     = random<int8_t>(eng);
    data.u8     = random<uint8_t>(eng);
    data.i16    = random<int16_t>(eng);
    data.u16    = random<uint16_t>(eng);
    data.i32    = random<int32_t>(eng);
    data.u32    = random<uint32_t>(eng);
    data.i64    = random<int64_t>(eng);
    data.u64    = random<uint64_t>(eng);
    data.f32    = random<float>(eng);
    data.f64    = random<double>(eng);
    data.str    = make_string();
    std::vector<uint8_t> buf;

    ankerl::nanobench::Bench().minEpochTime(100ms).maxEpochTime(1s).run("serialize", [&] {
        auto out = sink(buf);
        serialize(out, data);
        // ankerl::nanobench::doNotOptimizeAway(d);
    });
}