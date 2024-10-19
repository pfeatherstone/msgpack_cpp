#pragma once

#include <cstdint>
#include <limits>
#include <utility>
#include <type_traits>
#include <string_view>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <cstring>
#if __cpp_lib_bit_cast
#include <bit>
#endif
#include <endian.h>
#include "msgpack_error.h"


namespace msgpackcpp
{
    /////////////////////////////////////////////////////////////////////////////////
    /// Helpers
    /////////////////////////////////////////////////////////////////////////////////

#if __cpp_lib_bit_cast
    using std::bit_cast;
#else
    template<class To, class From>
    To bit_cast(const From& src) noexcept
    {
        To dst;
        std::memcpy(&dst, &src, sizeof(To));
        return dst;
    }
#endif

    /////////////////////////////////////////////////////////////////////////////////
    /// BOOL
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream>
    inline void serialize(Stream& out, bool v)
    {
        const uint8_t format = v ? 0xc3 : 0xc2;
        out((const char*)&format, 1);  
    }

    template<class Source>
    inline void deserialize(Source& in, bool& v)
    {
        uint8_t tmp{};
        in((char*)&tmp, 1);
        if (tmp == 0xc2)
            v = false;
        else if (tmp == 0xc3)
            v = true;
        else
            throw std::system_error(BAD_FORMAT);
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// Unsigned integers
    /////////////////////////////////////////////////////////////////////////////////
    
    template<class Stream, class UInt, std::enable_if_t<std::is_integral_v<UInt> && std::is_unsigned_v<UInt>, bool> = true>
    inline void serialize(Stream& out, UInt v)
    {
        if (v <= 0x7f)
        {
            // positive fixint (7-bit positive integer)
            const uint8_t v8 = static_cast<uint8_t>(v);
            out((const char*)&v8, 1);
        }
        else if (v <= std::numeric_limits<uint8_t>::max())
        {
            // unsigned 8
            constexpr uint8_t format = 0xcc;
            const     uint8_t v8     = static_cast<uint8_t>(v);
            out((const char*)&format, 1);
            out((const char*)&v8, 1);
        }
        else if (v <= std::numeric_limits<uint16_t>::max())
        {
            // unsigned 16
            constexpr uint8_t format = 0xcd;
            const     uint16_t v16   = htobe16(static_cast<uint16_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v16, 2);
        }    
        else if (v <= std::numeric_limits<uint32_t>::max())
        {
            // unsigned 32
            constexpr uint8_t format = 0xce;
            const     uint32_t v32   = htobe32(static_cast<uint32_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v32, 4);
        }
        else
        {
            // unsigned 64
            constexpr uint8_t format = 0xcf;
            const     uint64_t v64   = htobe64(static_cast<uint64_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v64, 8);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// Signed integers
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream, class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, bool> = true>
    inline void serialize(Stream& out, Int v)
    {
        if (v >= 0)
        {
            // Positive - convert to corresponding unsigned int
            const std::make_unsigned_t<Int> p = v;
            serialize(out, p);
        }
        else if (v >= -(1<<5))
        {
            // negative fixing (5-bit negative integer)
            const int8_t v8 = static_cast<int8_t>(v);
            out((const char*)&v8, 1);
        }
        else if (v >= std::numeric_limits<int8_t>::min())
        {
            // negative - int8
            constexpr uint8_t format = 0xd0;
            const     int8_t  v8     = static_cast<int8_t>(v);
            out((const char*)&format, 1);
            out((const char*)&v8, 1);
        }
        else if (v >= std::numeric_limits<int16_t>::min())
        {
            // negative - int16
            constexpr uint8_t format = 0xd1;
            const     uint16_t v16   = htobe16(bit_cast<uint16_t>(static_cast<int16_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v16, 2);
        }    
        else if (v >= std::numeric_limits<int32_t>::min())
        {
            // negative - int32_t
            constexpr uint8_t format = 0xd2;
            const     uint32_t v32   = htobe32(bit_cast<uint32_t>(static_cast<int32_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v32, 4);
        }
        else
        {
            // negative - int64_T
            constexpr uint8_t format = 0xd3;
            const     uint64_t v64   = htobe64(bit_cast<uint64_t>(static_cast<int64_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v64, 8);
        }
    }

    template<class Source, class Int, std::enable_if_t<std::is_integral_v<Int>, bool> = true>
    inline void deserialize(Source& in, Int& v)
    {
        uint8_t format{};
        in((char*)&format, 1);

        if (format <= 0x7f)
        {
            // positive fixint (7-bit positive integer)
            v = format;
        }
        else if ((format & 0b11100000) == 0b11100000)
        {
            // negative fixing (5-bit negative integer)
            v = bit_cast<int8_t>(format);
        }
        else if (format == 0xcc)
        {
            // unsigned 8
            uint8_t tmp{};
            in((char*)&tmp, 1);
            v = tmp;
        }
        else if (format == 0xcd)
        {
            // unsigned 16
            uint16_t tmp{};
            in((char*)&tmp, 2);
            v = htobe16(tmp);
        }
        else if (format == 0xce)
        {
            // unsigned 32
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = htobe32(tmp);
        }
        else if (format == 0xcf)
        {
            // unsigned 64
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = htobe64(tmp);
        }
        else if (format == 0xd0)
        {
            // signed 8
            int8_t tmp{};
            in((char*)&tmp, 1);
            v = tmp;
        }
        else if (format == 0xd1)
        {
            // signed 16
            uint16_t tmp{};
            in((char*)&tmp, 2);
            v = bit_cast<int16_t>(htobe16(tmp));
        }
        else if (format == 0xd2)
        {
            // signed 32
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = bit_cast<int32_t>(htobe32(tmp));
        }
        else if (format == 0xd3)
        {
            // signed 64
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = bit_cast<int64_t>(htobe64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// Floating point
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream>
    inline void serialize(Stream& out, float v)
    {
        constexpr uint8_t  format   = 0xca;
        const     uint32_t tmp      = htobe32(bit_cast<uint32_t>(v)); 
        out((const char*)&format, 1);
        out((const char*)&tmp, 4);
    }

    template<class Stream>
    inline void serialize(Stream& out, double v)
    {
        constexpr uint8_t  format   = 0xcb;
        const     uint64_t tmp      = htobe64(bit_cast<uint64_t>(v)); 
        out((const char*)&format, 1);
        out((const char*)&tmp, 8);
    }

    template<class Source, class Float, std::enable_if_t<std::is_floating_point_v<Float>, bool> = true>
    inline void deserialize(Source& in, Float& v)
    {
        uint8_t format{};
        in((char*)&format, 1);

        if (format == 0xca)
        {
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = bit_cast<float>(htobe32(tmp));
        }
        else if (format == 0xcb)
        {
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = bit_cast<double>(htobe64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// String
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream>
    inline void serialize_str_size(Stream& out, const uint32_t size)
    {
        if (size < 32)
        {
            const uint8_t format = 0xa0 | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 256)
        {
            constexpr uint8_t format = 0xd9;
            const     uint8_t size8  = static_cast<uint8_t>(size);
            out((const char*)&format, 1);
            out((const char*)&size8, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = 0xda;
            const     uint16_t size16 = htobe16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xdb;
            const     uint32_t size32 = htobe32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<class Source>
    inline void deserialize_str_size(Source& in, uint32_t& size)
    {
        uint8_t format{};
        in((char*)&format, 1);

        if ((format & 0b11100000) == 0b10100000)
        {
            size = format & 0b00011111;
        }
        else if (format == 0xd9)
        {
            uint8_t size8{};
            in((char*)&size8, 1);
            size = size8;
        }
        else if (format == 0xda)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = htobe16(size16);
        }
        else if (format == 0xdb)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = htobe32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<class Stream>
    inline void serialize(Stream& out, std::string_view v)
    {
        serialize_str_size(out, v.size());
        out(v.data(), v.size());
    }

    template<class Stream>
    inline void serialize(Stream& out, const char* c_str)
    {
        serialize(out, std::string_view(c_str));
    }

    template<class Source>
    inline void deserialize(Source& in, std::string& v)
    {
        uint32_t size{};
        deserialize_str_size(in, size);
        v.resize(size);
        in(v.data(), size);
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// Binary array
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream>
    inline void serialize_bin_size(Stream& out, const uint32_t len)
    {
        if (len < 256)
        {
            constexpr uint8_t format = 0xc4;
            const     uint8_t size8  = static_cast<uint8_t>(len);
            out((const char*)&format, 1);
            out((const char*)&size8, 1);
        }
        else if (len < 65536)
        {
            constexpr uint8_t  format = 0xc5;
            const     uint16_t size16 = htobe16(static_cast<uint16_t>(len));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xc6;
            const     uint32_t size32 = htobe32(static_cast<uint32_t>(len));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<class Source>
    inline void deserialize_bin_size(Source& in, uint32_t& size)
    {
        uint8_t format{};
        in((char*)&format, 1);

        if (format == 0xc4)
        {
            uint8_t size8{};
            in((char*)&size8, 1);
            size = size8;
        }
        else if (format == 0xc5)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = htobe16(size16);
        }
        else if (format == 0xc6)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = htobe32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<class Stream>
    inline void serialize_bin_array(Stream& out, const char* data, const uint32_t len)
    {
        serialize_bin_size(out, len);
        out(data, len);
    }

    template<class Stream, class Alloc>
    inline void serialize(Stream& out, const std::vector<char, Alloc>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<class Stream, class Alloc>
    inline void serialize(Stream& out, const std::vector<uint8_t, Alloc>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<class Source, class Alloc>
    inline void deserialize(Source& in, std::vector<char, Alloc>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        v.resize(size);
        in(v.data(), size);
    }

    template<class Source, class Alloc>
    inline void deserialize(Source& in, std::vector<uint8_t, Alloc>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        v.resize(size);
        in((char*)v.data(), size);
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// Array
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream>
    inline void serialize_array_size(Stream& out, const uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = 0b10010000 | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = 0xdc;
            const     uint16_t size16 = htobe16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xdd;
            const     uint32_t size32 = htobe32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<class Source>
    inline void deserialize_array_size(Source& in, uint32_t& size)
    {
        uint8_t format{};
        in((char*)&format, 1);

        if ((format & 0b11110000) == 0b10010000)
        {
            size = format & 0b00001111;
        }
        else if (format == 0xdc)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = htobe16(size16);
        }
        else if (format == 0xdd)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = htobe32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<class Stream, class T, class Alloc>
    inline void serialize(Stream& out, const std::vector<T, Alloc>& v)
    { 
        serialize_array_size(out, v.size());
        for (const auto& x : v)
            serialize(out, x);
    }

    template<class Source, class T, class Alloc>
    inline void deserialize(Source& in, std::vector<T, Alloc>& v)
    {
        uint32_t size{};
        deserialize_array_size(in, size);
        v.resize(size);
        for (auto& x : v)
            deserialize(in, x);
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// Maps
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream>
    inline void serialize_map_size(Stream& out, const uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = 0b10000000 | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = 0xde;
            const     uint16_t size16 = htobe16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xdf;
            const     uint32_t size32 = htobe32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<class Source>
    inline void deserialize_map_size(Source& in, uint32_t& size)
    {
        uint8_t format{};
        in((char*)&format, 1);

        if ((format & 0b11110000) == 0b10000000)
        {
            size = format & 0b00001111;
        }
        else if (format == 0xde)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = htobe16(size16);
        }
        else if (format == 0xdf)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = htobe32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template <
        class Stream, 
        class K, 
        class V, 
        class Compare = std::less<K>,
        class Alloc = std::allocator<std::pair<const K, V>>
    >
    inline void serialize(Stream& out, const std::map<K,V,Compare,Alloc>& map)
    {
        serialize_map_size(out, map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(out, k);
            serialize(out, v);
        }
    }

    template <
        class Source, 
        class K, 
        class V, 
        class Compare = std::less<K>,
        class Alloc = std::allocator<std::pair<const K, V>>
    >
    inline void deserialize(Source& in, std::map<K,V,Compare,Alloc>& map)
    {
        uint32_t size{};
        deserialize_map_size(in, size);
        
        for (uint32_t i = 0 ; i < size ; ++i)
        {
            K key{};
            V val{};
            deserialize(in, key);
            deserialize(in, val);
            map.emplace(std::make_pair(key, val));
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
    inline void serialize(Stream& out, const std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map)
    {
        serialize_map_size(out, map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(out, k);
            serialize(out, v);
        }
    }

    template <
        class Source, 
        class K,
        class V,
        class Hash      = std::hash<K>,
        class KeyEqual  = std::equal_to<K>,
        class Alloc     = std::allocator<std::pair<const K, V>>
    >
    inline void deserialize(Source& in, std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map)
    {
        uint32_t size{};
        deserialize_map_size(in, size);
        
        for (uint32_t i = 0 ; i < size ; ++i)
        {
            K key{};
            V val{};
            deserialize(in, key);
            deserialize(in, val);
            map.emplace(std::make_pair(key, val));
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    /// Tuple
    /////////////////////////////////////////////////////////////////////////////////

    template<class Stream, class... Args>
    inline void serialize(Stream& out, const std::tuple<Args...>& tpl)
    {
        serialize_array_size(out, sizeof...(Args));
        std::apply([&](auto&&... args) {
            (serialize(out, std::forward<decltype(args)>(args)),...);
        }, tpl);
    }

    template<class Source, class... Args>
    inline void deserialize(Source& in, std::tuple<Args...>& tpl)
    {
        uint32_t size{};
        deserialize_array_size(in, size);
        if (size != sizeof...(Args))
            throw std::system_error(BAD_SIZE);

        std::apply([&](auto&&... args) {
            (deserialize(in, std::forward<decltype(args)>(args)),...);
        }, tpl);
    }
}
