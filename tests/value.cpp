#include "doctest.h"
#include "msgpack.h"

using namespace std;
using namespace std::literals::string_view_literals;
using namespace msgpackcpp;

TEST_SUITE("[VALUE]") 
{
    TEST_CASE("basic") 
    {
        value jv;

        jv = nullptr;
        REQUIRE(jv.is_null());
        REQUIRE(jv.size() == 0);

        jv = true;
        REQUIRE(jv.is_bool());
        REQUIRE(jv.size() == 1);
        REQUIRE(jv.as_bool() == true);

        jv = 42;
        REQUIRE(jv.is_int());
        REQUIRE(jv.as_int64() == 42);
        REQUIRE(jv.size() == 1);

        jv = 3.141595;
        REQUIRE(jv.is_real());
        REQUIRE(jv.as_real() == 3.141595);
        REQUIRE(jv.size() == 1);

        jv = "hello there"; // const char*
        REQUIRE(jv.is_str());
        REQUIRE(jv.as_str() == "hello there");
        REQUIRE(jv.size() == 1);

        jv = "from a certain point of view"sv; // std::string_view
        REQUIRE(jv.is_str());
        REQUIRE(jv.as_str() == "from a certain point of view");
        REQUIRE(jv.size() == 1);

        jv = std::string("peace is a lie"); // std::string
        REQUIRE(jv.is_str());
        REQUIRE(jv.as_str() == "peace is a lie");
        REQUIRE(jv.size() == 1);

        jv = {0,1,2,3,4,5,6,7,8,9};
        REQUIRE(jv.is_array());
        REQUIRE(jv.size() == 10);
        size_t i{0};
        for (auto v : jv.as_array())
        {
            REQUIRE(v.is_int());
            REQUIRE(v.as_int64() == i++);
        }

        jv = {33, "age", true};
        REQUIRE(jv.is_array());
        REQUIRE(jv.size() == 3);
        REQUIRE(jv[0].is_int());
        REQUIRE(jv[0].as_int64() == 33);
        REQUIRE(jv[1].is_str());
        REQUIRE(jv[1].as_str() == "age");
        REQUIRE(jv[2].is_bool());
        REQUIRE(jv[2].as_bool() == true);  

        jv = {{1,1.52}, {2,3.141592}};
        REQUIRE(jv.is_array());
        REQUIRE(jv.size() == 2);
        for (const auto& el : jv.as_array())
        {
            REQUIRE(el.is_array());
            REQUIRE(el.size() == 2);
        }

        jv = {
            {"pi", 3.141},
            {"happy", true},
            {"name", "Niels"},
            {"nothing", nullptr},
            {"answer", {
                {"everything", 42}
            }},
            {"list", {1, 0, 2}},
            {"object", {
                {"currency", "USD"},
                {"value", 42.99}
            }}
        };

        const auto check_niels = [](value& jv)
        {
            REQUIRE(jv.is_object());
            REQUIRE(jv.size() == 7);
            REQUIRE(jv.at("pi").as_real() == 3.141);
            REQUIRE(jv.at("happy").as_bool() == true);
            REQUIRE(jv.at("name").as_str() == "Niels");
            REQUIRE(jv.at("nothing").is_null());
            REQUIRE(jv.at("answer").at("everything").as_int64() == 42);
            REQUIRE(jv.at("list").as_array().size() == 3);
            REQUIRE(jv.at("object").at("currency").as_str() == "USD");
            REQUIRE(jv.at("object").at("value").as_real() == 42.99);
        };

        check_niels(jv);
    }
}