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
    inline void serialize(Stream&& out, const T& obj, bool as_map = false)
    { 
        if (as_map)
        {
            serialize_map_size(std::forward<Stream>(out), boost::mp11::mp_size<D1>::value);

            boost::mp11::mp_for_each<D1>([&](auto D) {
                serialize(std::forward<Stream>(out), D.name);
                serialize(std::forward<Stream>(out), obj.*D.pointer);
            });
        }
        else
        {
            serialize_array_size(std::forward<Stream>(out), boost::mp11::mp_size<D1>::value);

            boost::mp11::mp_for_each<D1>([&](auto D) {
                serialize(std::forward<Stream>(out), obj.*D.pointer);
            });
        }
    }
}