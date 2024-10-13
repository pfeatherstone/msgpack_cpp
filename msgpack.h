#pragma once

#include <cstdint>
#include <limits>
#include <utility>
#include <endian.h>

namespace msgpackcpp
{
    template<class Stream>
    inline void serialize(Stream&& out, bool v)
    {
        const uint8_t format = v ? 0xc3 : 0xc2;
        out((const char*)&format, 1);  
    }
    
    template<class Stream>
    inline void serialize(Stream&& out, uint8_t v)
    {
        if (v <= 0x7f)
        {
            // positive fixint (7-bit positive integer)
            out((const char*)&v, 1);
        }
        else
        {
            // unsigned 8
            constexpr uint8_t format = 0xcc;
            out((const char*)&format, 1);
            out((const char*)&v, 1);
        }       
    }

    template<class Stream>
    inline void serialize(Stream&& out, int8_t v)
    {
        if (v < -(1<<5))
        {
            // signed 8
            constexpr uint8_t format = 0xd0;
            out((const char*)&format, 1);
            out((const char*)&v, 1);
        }
        else
        {
            // negative fixing (5-bit negative integer)
            out((const char*)&v, 1);
        }
    }

    template<class Stream>
    inline void serialize(Stream&& out, uint16_t v)
    {
        if (v <= std::numeric_limits<uint8_t>::max())
        {
            serialize(std::forward<Stream>(out), (uint8_t)v);
        }
        else
        {
            constexpr uint8_t format = 0xcd;;
            out((const char*)&format, 1);
            v = htobe16(v);
            out((const char*)&v, 2);
        }
    }

    template<class Stream>
    inline void serialize(Stream&& out, int16_t v)
    {
        // Positive
        if (v >= 0)
        {
            // uint16_t
            serialize(std::forward<Stream>(out), (uint16_t)v);
        }
        // negative
        else
        {
            if (v >= std::numeric_limits<int8_t>::min())
            {
                // int8_t
                serialize(std::forward<Stream>(out), (int8_t)v);
            }
            else
            {
                constexpr uint8_t format = 0xd1;
                out((const char*)&format, 1);
                v = htobe16(v);
                out((const char*)&v, 2);
            }
        }
    }

    template<class Stream>
    inline void serialize(Stream&& out, uint32_t v)
    {
        if (v <= std::numeric_limits<uint16_t>::max())
        {
            serialize(std::forward<Stream>(out), (uint16_t)v);
        }
        else
        {
            constexpr uint8_t format = 0xce;
            out((const char*)&format, 1);
            v = htobe32(v);
            out((const char*)&v, 4);
        }
    }

    template<class Stream>
    inline void serialize(Stream&& out, int32_t v)
    {
        // Positive
        if (v >= 0)
        {
            // uint32_t
            serialize(std::forward<Stream>(out), (uint32_t)v);
        }
        // negative
        else
        {
            if (v >= std::numeric_limits<int16_t>::min())
            {
                // int16_t
                serialize(std::forward<Stream>(out), (int16_t)v);
            }
            else
            {
                constexpr uint8_t format = 0xd2;
                out((const char*)&format, 1);
                v = htobe32(v);
                out((const char*)&v, 4);
            }
        }
    }

    template<class Stream>
    inline void serialize(Stream&& out, uint64_t v)
    {
        if (v <= std::numeric_limits<uint32_t>::max())
        {
            serialize(std::forward<Stream>(out), (uint32_t)v);
        }
        else
        {
            constexpr uint8_t format = 0xcf;
            out((const char*)&format, 1);
            v = htobe64(v);
            out((const char*)&v, 8);
        }
    }

    template<class Stream>
    inline void serialize(Stream&& out, int64_t v)
    {
        // Positive
        if (v >= 0)
        {
            // uint64_t
            serialize(std::forward<Stream>(out), (uint64_t)v);
        }
        // negative
        else
        {
            if (v >= std::numeric_limits<int32_t>::min())
            {
                // int32_t
                serialize(std::forward<Stream>(out), (int32_t)v);
            }
            else
            {
                constexpr uint8_t format = 0xd3;
                out((const char*)&format, 1);
                v = htobe64(v);
                out((const char*)&v, 8);
            }
        }
    }

    template<class Stream>
    inline void serialize(Stream&& out, float v)
    {
        constexpr uint8_t format = 0xca;
        out((const char*)&format, 1);
        uint32_t tmp{};
        memcpy(&tmp, &v, 4);
        tmp = htobe32(tmp);
        out((const char*)&tmp, 4);
    }

    template<class Stream>
    inline void serialize(Stream&& out, double v)
    {
        constexpr uint8_t format = 0xcb;
        out((const char*)&format, 1);
        uint64_t tmp{};
        memcpy(&tmp, &v, 8);
        tmp = htobe64(tmp);
        out((const char*)&tmp, 8);
    }
}