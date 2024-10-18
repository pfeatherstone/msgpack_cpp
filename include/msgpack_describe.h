#pragma once

#include <boost/describe/members.hpp>
#include "msgpack.h"

namespace msgpackcpp
{
    template <
        class Stream, 
        class T,
        class D1 = boost::describe::describe_members<T, boost::describe::mod_any_access>
    >
    inline void serialize(Stream& out, const T& obj, bool as_map = false)
    { 
        if (as_map)
        {
            serialize_map_size(out, boost::mp11::mp_size<D1>::value);

            boost::mp11::mp_for_each<D1>([&](auto D) {
                serialize(out, D.name);
                serialize(out, obj.*D.pointer);
            });
        }
        else
        {
            serialize_array_size(out, boost::mp11::mp_size<D1>::value);

            boost::mp11::mp_for_each<D1>([&](auto D) {
                serialize(out, obj.*D.pointer);
            });
        }
    }

    template <
        class Source, 
        class T,
        class D1 = boost::describe::describe_members<T, boost::describe::mod_any_access>
    >
    inline void deserialize(Source& in, T& obj, bool as_map = false)
    { 
        if (as_map)
        {
            uint32_t size{};
            deserialize_map_size(in, size);
            if (size != boost::mp11::mp_size<D1>::value)
                throw std::system_error(BAD_SIZE); 

            boost::mp11::mp_for_each<D1>([&](auto D) {
                std::string name;
                deserialize(in, name);
                if (name != D.name)
                    throw std::system_error(BAD_NAME);
                deserialize(in, obj.*D.pointer);
            });
        }
        else
        {
            uint32_t size{};
            deserialize_array_size(in, size);
            if (size != boost::mp11::mp_size<D1>::value)
                throw std::system_error(BAD_SIZE); 

            boost::mp11::mp_for_each<D1>([&](auto D) {
                deserialize(in, obj.*D.pointer);
            });
        }
    }
}
