#pragma once

#include <cstdint>
#include <limits>
#include <utility>
#include <type_traits>
#include <string_view>
#include <vector>
#include <map>
#include <unordered_map>
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
    inline void serialize(Stream&& out, uint16_t v)
    {
        if (v <= std::numeric_limits<uint8_t>::max())
        {
            serialize(std::forward<Stream>(out), (uint8_t)v);
        }
        else
        {
            constexpr uint8_t format = 0xcd;
            v = htobe16(v);
            out((const char*)&format, 1);
            out((const char*)&v, 2);
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

    template<class Stream>
    inline void serialize(Stream&& out, std::string_view v)
    {
        if (v.size() < 32)
        {
            const uint8_t format = 0xa0 | static_cast<uint8_t>(v.size());
            out((const char*)&format, 1);
        }
        else if (v.size() < 256)
        {
            const uint8_t format = 0xd9;
            const uint8_t size8  = static_cast<uint8_t>(v.size());
            out((const char*)&format, 1);
            out((const char*)&size8, 1);
        }
        else if (v.size() < 65536)
        {
            const uint8_t  format = 0xda;
            const uint16_t size16 = htobe16(static_cast<uint16_t>(v.size()));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            const uint8_t  format = 0xdb;
            const uint32_t size32 = htobe32(static_cast<uint32_t>(v.size()));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
        out(v.data(), v.size());
    }

    template<class Stream>
    inline void serialize(Stream&& out, const char* c_str)
    {
        serialize(std::forward<Stream>(out), std::string_view(c_str));
    }
    
    template<class Stream>
    inline void serialize_bin_array(Stream&& out, const char* data, const size_t len)
    {
        if (len < 256)
        {
            const uint8_t format = 0xc4;
            const uint8_t size8  = static_cast<uint8_t>(len);
            out((const char*)&format, 1);
            out((const char*)&size8, 1);
        }
        else if (len < 65536)
        {
            const uint8_t  format = 0xc5;
            const uint16_t size16 = htobe16(static_cast<uint16_t>(len));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            const uint8_t  format = 0xc6;
            const uint32_t size32 = htobe32(static_cast<uint32_t>(len));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
        out(data, len);
    }

    template<class Stream, class Alloc>
    inline void serialize(Stream&& out, const std::vector<char, Alloc>& v)
    {
        serialize_bin_array(std::forward<Stream>(out), (const char*)v.data(), v.size());
    }

    template<class Stream, class Alloc>
    inline void serialize(Stream&& out, const std::vector<uint8_t, Alloc>& v)
    {
        serialize_bin_array(std::forward<Stream>(out), (const char*)v.data(), v.size());
    }

    template<class Stream>
    inline void serialize_array_size(Stream&& out, uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = 0x90 | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            const uint8_t  format = 0xdc;
            const uint16_t size16 = htobe16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            const uint8_t  format = 0xdd;
            const uint32_t size32 = htobe32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<class Stream, class T, class Alloc>
    inline void serialize(Stream&& out, const std::vector<T, Alloc>& v)
    { 
        serialize_array_size(std::forward<Stream>(out), v.size());
        for (const auto& x : v)
            serialize(std::forward<Stream>(out), x);
    }

    template<class Stream>
    inline void serialize_map_size(Stream&& out, uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = 0x80 | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            const uint8_t  format = 0xde;
            const uint16_t size16 = htobe16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            const uint8_t  format = 0xdf;
            const uint32_t size32 = htobe32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template <
        class Stream, 
        class K, 
        class V, 
        class Compare = std::less<K>,
        class Alloc = std::allocator<std::pair<const K, V>>
    >
    inline void serialize(Stream&& out, const std::map<K,V,Compare,Alloc>& map)
    {
        serialize_map_size(std::forward<Stream>(out), map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(std::forward<Stream>(out), k);
            serialize(std::forward<Stream>(out), v);
        }
    }

    template <
        class Stream, 
        class K,
        class V,
        class Hash      = std::hash<K>,
        class KeyEqual  = std::equal_to<K>,
        class Alloc     = std::allocator<std::pair<const K, V>>
    >
    inline void serialize(Stream&& out, const std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map)
    {
        serialize_map_size(std::forward<Stream>(out), map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(std::forward<Stream>(out), k);
            serialize(std::forward<Stream>(out), v);
        }
    }
}