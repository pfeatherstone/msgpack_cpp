#pragma once

#include <string>
#include <system_error>

namespace msgpackcpp
{
    enum deserialization_error
    {
        OUT_OF_DATA = 1,
        BAD_FORMAT  = 2,
        BAD_SIZE    = 3,
        BAD_NAME    = 4
    };

    struct deserialization_error_category : std::error_category
    {
        const char* name() const noexcept override 
        {
            return "msgpack deserialization";
        }

        std::string message(int ev) const override
        {
            switch(static_cast<deserialization_error>(ev))
            {
            case OUT_OF_DATA: return "Ran out of data while deserializing";
            case BAD_FORMAT:  return "Found bad format";
            case BAD_SIZE:    return "Found bad size";
            case BAD_NAME:    return "Found bad name";
            default:          return "Unrecognised error";
            }
        }
    };

    const deserialization_error_category& get_deserialization_error_category_singleton()
    {
        static deserialization_error_category singleton;
        return singleton;
    }

    inline std::error_code make_error_code(deserialization_error ec)
    {
        return {static_cast<int>(ec), get_deserialization_error_category_singleton()};
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<msgpackcpp::deserialization_error> : true_type {};
}
