#pragma once

#include <vector>
#include <ostream>

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
}