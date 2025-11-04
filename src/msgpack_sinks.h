#pragma once

#include <cstring>
#include <vector>
#include <ostream>
#include <istream>
#include "msgpack.h"

namespace msgpackcpp
{

//----------------------------------------------------------------------------------------------------------------

    template<class Byte, class Alloc>
    struct sink_vector : sink_base
    {
        static_assert(is_byte<Byte>, "Byte needs to be a byte");
        std::vector<Byte,Alloc>& buf;

        sink_vector(std::vector<Byte,Alloc>& buf_) : buf{buf_} {}

        void write(const char* data, size_t nbytes) override
        {
            buf.insert(end(buf), data, data + nbytes);
        }
    };

    template<class Byte, class Alloc>
    struct source_vector : source_base
    {
        static_assert(is_byte<Byte>, "Byte needs to be a byte");
        const std::vector<Byte,Alloc>& data;
        size_t offset{0};

        source_vector(const std::vector<Byte,Alloc>& data_) : data{data_} {}

        void read(char* buf, size_t nbytes) override
        {
            if ((data.size() - offset) < nbytes)
                throw std::system_error(OUT_OF_DATA);
            std::memcpy(buf, data.data() + offset, nbytes);
            offset += nbytes;
        }

        uint8_t peak() override
        {
            return static_cast<uint8_t>(data[offset]);
        }
    };

    template<class Byte, class Alloc, std::enable_if_t<is_byte<Byte>, bool> = true>
    auto sink(std::vector<Byte, Alloc>& buf)
    {
        return sink_vector<Byte,Alloc>{buf};
    }

    template<class Byte, class Alloc, std::enable_if_t<is_byte<Byte>, bool> = true>
    auto source(const std::vector<Byte, Alloc>& buf)
    {
        return source_vector<Byte,Alloc>{buf};
    }

//----------------------------------------------------------------------------------------------------------------

    // inline auto sink(std::ostream& out)
    // {
    //     return [&](const char* bytes, size_t nbytes) {
    //         out.write(bytes, nbytes);
    //     };
    // }

    // inline auto source(std::istream& in)
    // {
    //     return [&](char* bytes, size_t nbytes) {
    //         in.read(bytes, nbytes);
    //         if (in.gcount() != (long)nbytes)
    //             throw std::system_error(OUT_OF_DATA);
    //     };
    // }
}