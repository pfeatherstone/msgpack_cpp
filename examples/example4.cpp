#include "msgpack.h"
#include "msgpack_sinks.h"

const auto REQUIRE = [](auto res) {if (!res)throw std::runtime_error("bad");};

int main()
{
    using msgpackcpp::serialize;
    using msgpackcpp::deserialize;
    using msgpackcpp::sink;
    using msgpackcpp::source;

    // Data
    msgpackcpp::value jv0 = {
        {"pi", 3.141},
        {"happy", true},
        {"name", "Niels"},
        {"nothing", nullptr},
        {"answer", {
            {"everything", -42}
        }},
        {"list", {1, 0, 2}},
        {"object", {
            {"currency", "USD"},
            {"value", 42.99}
        }}
    };

    // Serialize (pack)
    std::vector<char> buf;
    auto out = sink(buf);
    jv0.pack(out);

    // Deserialize (unpack)
    msgpackcpp::value jv1;
    auto in = source(buf);
    jv1.unpack(in);

    // Check
    REQUIRE(jv1.is_object());
    REQUIRE(jv1.size() == 7);
    REQUIRE(jv1.at("pi").as_real() == 3.141);
    REQUIRE(jv1.at("happy").as_bool() == true);
    REQUIRE(jv1.at("name").as_str() == "Niels");
    REQUIRE(jv1.at("nothing").is_null());
    REQUIRE(jv1.at("answer").at("everything").as_int64() == -42);
    REQUIRE(jv1.at("list").as_array().size() == 3);
    REQUIRE(jv1.at("object").at("currency").as_str() == "USD");
    REQUIRE(jv1.at("object").at("value").as_real() == 42.99);
}