#pragma once

#include <cstring>
#include <vector>
#include <ostream>
#include <istream>
#include "msgpack_error.h"

namespace msgpackcpp
{
    template<class Byte, class Alloc, std::enable_if_t<sizeof(Byte) == 1, bool> = true>
    auto sink(std::vector<Byte, Alloc>& buf)
    {
        return [&](const char* bytes, size_t nbytes) {
            buf.insert(end(buf), bytes, bytes + nbytes);
        };
    } 

    inline auto sink(std::ostream& out)
    {
        return [&](const char* bytes, size_t nbytes) {
            out.write(bytes, nbytes);
        };
    }

    template<class Byte, class Alloc, std::enable_if_t<sizeof(Byte) == 1, bool> = true>
    auto source(const std::vector<Byte, Alloc>& buf)
    {
        size_t offset{0};
        return [&buf, offset](char* bytes, size_t nbytes) mutable {
            if ((buf.size() - offset) < nbytes)
                throw std::system_error(OUT_OF_DATA);
            std::memcpy(bytes, buf.data() + offset, nbytes);
            offset += nbytes;
        };
    } 

    inline auto source(std::istream& in)
    {
        return [&](char* bytes, size_t nbytes) {
            in.read(bytes, nbytes);
            if (in.gcount() != (long)nbytes)
                throw std::system_error(OUT_OF_DATA);
        };
    }
}