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
    struct sink_vector final : sink_base
    {
        static_assert(is_byte<Byte>, "Byte needs to be a byte");
        std::vector<Byte,Alloc>& buf;

        sink_vector(std::vector<Byte,Alloc>& buf_) : buf{buf_} {}

        void write(const char* data, size_t nbytes) override final
        {
            buf.insert(end(buf), data, data + nbytes);
        }
    };

    template<class Byte, class Alloc>
    struct source_vector final : source_base
    {
        static_assert(is_byte<Byte>, "Byte needs to be a byte");
        const std::vector<Byte,Alloc>& data;
        size_t offset{0};

        source_vector(const std::vector<Byte,Alloc>& data_) : data{data_} {}

        void read(char* buf, size_t nbytes) override final
        {
            if ((data.size() - offset) < nbytes)
                throw std::system_error(OUT_OF_DATA);
            std::memcpy(buf, data.data() + offset, nbytes);
            offset += nbytes;
        }

        uint8_t peak() override final
        {
            if (offset >= data.size())
                throw std::system_error(OUT_OF_DATA);
            return static_cast<uint8_t>(data[offset]);
        }
    };

    template<class Byte, class Alloc, check_byte<Byte> = true>
    auto sink(std::vector<Byte, Alloc>& buf)
    {
        return sink_vector<Byte,Alloc>{buf};
    }

    template<class Byte, class Alloc, check_byte<Byte> = true>
    auto source(const std::vector<Byte, Alloc>& buf)
    {
        return source_vector<Byte,Alloc>{buf};
    }

//----------------------------------------------------------------------------------------------------------------

    struct ostream_sink final : sink_base
    {
        std::ostream& out;

        ostream_sink(std::ostream& out_) : out{out_}{}
        void write(const char* data, size_t nbytes) override final {out.write(data, nbytes);}
    };

    struct istream_source final : source_base
    {
        std::istream& in;

        istream_source(std::istream& in_) : in{in_}{}

        void read(char* buf, size_t nbytes) override final
        {
            in.read(buf, nbytes);
            if (in.gcount() != (long)nbytes)
                throw std::system_error(OUT_OF_DATA);
        }

        uint8_t peak() override final
        {
            uint8_t b = static_cast<uint8_t>(in.peek());
            if (!in || !in.good() || in.eof())
                throw std::system_error(OUT_OF_DATA);
            return b;
        }
    };

    inline auto sink(std::ostream& out)  { return ostream_sink{out}; }
    inline auto source(std::istream& in) { return istream_source{in}; }

//----------------------------------------------------------------------------------------------------------------

}