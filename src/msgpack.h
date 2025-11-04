#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <limits>
#include <utility>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <map>
#include <unordered_map>
#include <variant>
#include <system_error>
#if __cpp_lib_bit_cast
#include <bit>
#endif

namespace msgpackcpp
{

//----------------------------------------------------------------------------------------------------------------

    class value
    {
    private:
        std::variant<std::nullptr_t,
                     bool,
                     int64_t,
                     uint64_t,
                     double,
                     std::string,
                     std::vector<value>,
                     std::map<std::string, value>> val;

    public:
        value()                             = default;
        value(const value& ori)             = default;
        value(value&& ori)                  = default;
        value& operator=(const value& ori)  = default;
        value& operator=(value&& ori)       = default;

        value(std::nullptr_t);
        value(bool v);

        template<class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, bool> = true>
        value(Int v);

        template<class UInt, std::enable_if_t<std::is_integral_v<UInt> && std::is_unsigned_v<UInt>, bool> = true>
        value(UInt v);

        template<class Real, std::enable_if_t<std::is_floating_point_v<Real>, bool> = true>
        value(Real v);

        value(const char* v);
        value(std::string_view v);
        value(std::string v);
        value(std::vector<value> v);
        value(std::map<std::string, value> v);
        value(std::initializer_list<value> v);

        size_t size() const noexcept;

        bool is_null()      const noexcept;
        bool is_bool()      const noexcept;
        bool is_int()       const noexcept;
        bool is_real()      const noexcept;
        bool is_str()       const noexcept;
        bool is_array()     const noexcept;
        bool is_object()    const noexcept;

        auto as_bool()      const -> bool;
        auto as_bool()            -> bool&;
        auto as_int64()     const -> int64_t;
        auto as_int64()           -> int64_t&;
        auto as_uint64()    const -> uint64_t;
        auto as_uint64()          -> uint64_t&;
        auto as_real()      const -> double;
        auto as_real()            -> double&;
        auto as_str()       const -> const std::string&;
        auto as_str()             -> std::string&;
        auto as_array()     const -> const std::vector<value>&;
        auto as_array()           -> std::vector<value>&;
        auto as_object()    const -> const std::map<std::string, value>&;
        auto as_object()          -> std::map<std::string, value>&;

        const value& at(const std::string& key) const;
        value&       at(const std::string& key);
        value&       operator[](const std::string& key);

        const value& operator[](size_t array_index) const;
        value&       operator[](size_t array_index);
    };

//----------------------------------------------------------------------------------------------------------------

    enum deserialization_error
    {
        OUT_OF_DATA = 1,
        BAD_FORMAT  = 2,
        BAD_SIZE    = 3,
        BAD_NAME    = 4
    };

    std::error_code make_error_code(deserialization_error ec);

//----------------------------------------------------------------------------------------------------------------

}

//----------------------------------------------------------------------------------------------------------------

namespace std
{
    template <>
    struct is_error_code_enum<msgpackcpp::deserialization_error> : true_type {};
}

//----------------------------------------------------------------------------------------------------------------

namespace msgpackcpp
{

//----------------------------------------------------------------------------------------------------------------

    template<class Byte>
    constexpr bool is_byte = std::is_same_v<Byte, char>     || 
                             std::is_same_v<Byte, uint8_t>  ||
                             std::is_same_v<Byte, int8_t>;

    template<class T, typename = void>
    struct is_binary_array : std::false_type {};

    template<class T>
    struct is_binary_array<T, std::void_t<typename T::value_type,
                                          decltype(std::declval<T>().data()),
                                          decltype(std::declval<T>().size())>> : std::integral_constant<bool, is_byte<typename T::value_type>> {};

    template<class T>
    constexpr bool is_binary_array_v = is_binary_array<T>::value;

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    void serialize(Stream& out, bool v);

    template<class Source>
    void deserialize(Source& in, bool& v);
    
//----------------------------------------------------------------------------------------------------------------

    template<class Stream, class UInt, std::enable_if_t<std::is_integral_v<UInt> && std::is_unsigned_v<UInt>, bool> = true>
    void serialize(Stream& out, UInt v);

    template<class Stream, class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, bool> = true>
    void serialize(Stream& out, Int v);

    template<class Source, class Int, std::enable_if_t<std::is_integral_v<Int>, bool> = true>
    void deserialize(Source& in, Int& v);

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    void serialize(Stream& out, float v);

    template<class Stream>
    void serialize(Stream& out, double v);

    template<class Source, class Float, std::enable_if_t<std::is_floating_point_v<Float>, bool> = true>
    void deserialize(Source& in, Float& v);

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    void serialize_str_size(Stream& out, const uint32_t size);

    template<class Source>
    void deserialize_str_size(Source& in, uint32_t& size);

    template<class Stream>
    void serialize(Stream& out, std::string_view v);

    template<class Stream>
    void serialize(Stream& out, const char* c_str);

    template<class Source>
    void deserialize(Source& in, std::string& v);

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    void serialize_bin_size(Stream& out, const uint32_t len);

    template<class Source>
    void deserialize_bin_size(Source& in, uint32_t& size);

    template<class Stream>
    void serialize_bin_array(Stream& out, const char* data, const uint32_t len);

    template<class Stream, class Alloc>
    void serialize(Stream& out, const std::vector<char, Alloc>& v);

    template<class Stream, class Alloc>
    void serialize(Stream& out, const std::vector<uint8_t, Alloc>& v);

    template<class Source, class Alloc>
    void deserialize(Source& in, std::vector<char, Alloc>& v);

    template<class Source, class Alloc>
    void deserialize(Source& in, std::vector<uint8_t, Alloc>& v);

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    void serialize_array_size(Stream& out, const uint32_t size);

    template<class Source>
    void deserialize_array_size(Source& in, uint32_t& size);

    template<class Stream, class T, class Alloc>
    void serialize(Stream& out, const std::vector<T, Alloc>& v);

    template<class Source, class T, class Alloc>
    void deserialize(Source& in, std::vector<T, Alloc>& v);

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    void serialize_map_size(Stream& out, const uint32_t size);

    template<class Source>
    void deserialize_map_size(Source& in, uint32_t& size);

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

//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
// DEFINITIONS
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------

    constexpr uint16_t byte_swap16(uint16_t v)
    {
        return static_cast<uint16_t>(((v & 0x00FF) << 8) | ((v & 0xFF00) >> 8));
    }

    constexpr uint64_t byte_swap32(uint32_t v)
    {
        return static_cast<uint32_t>(((v & 0x000000FFu) << 24) |
                                        ((v & 0x0000FF00u) << 8)  |
                                        ((v & 0x00FF0000u) >> 8)  |
                                        ((v & 0xFF000000u) >> 24));
    }

    constexpr uint64_t byte_swap64(uint64_t v)
    {
        return static_cast<uint64_t>(((v & 0x00000000000000FFULL) << 56) |
                                    ((v & 0x000000000000FF00ULL) << 40) |
                                    ((v & 0x0000000000FF0000ULL) << 24) |
                                    ((v & 0x00000000FF000000ULL) << 8)  |
                                    ((v & 0x000000FF00000000ULL) >> 8)  |
                                    ((v & 0x0000FF0000000000ULL) >> 24) |
                                    ((v & 0x00FF000000000000ULL) >> 40) |
                                    ((v & 0xFF00000000000000ULL) >> 56));
    }

    static_assert(byte_swap16(0x1234)               == 0x3412,              "bad swap");
    static_assert(byte_swap32(0x12345678)           == 0x78563412,          "bad swap");
    static_assert(byte_swap64(0x123456789abcdef1)   == 0xf1debc9a78563412,  "bad swap");
    
    inline bool is_little_endian() 
    {
        constexpr uint32_t v{0x01020304};
        const auto*        ptr{reinterpret_cast<const unsigned char*>(&v)};
        return ptr[0] == 0x04;
    }

//----------------------------------------------------------------------------------------------------------------

    inline uint16_t host_to_b16(uint16_t v) { return is_little_endian() ? byte_swap16(v) : v; }
    inline uint32_t host_to_b32(uint32_t v) { return is_little_endian() ? byte_swap32(v) : v; }
    inline uint64_t host_to_b64(uint64_t v) { return is_little_endian() ? byte_swap64(v) : v; }
    
//----------------------------------------------------------------------------------------------------------------

    template<class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, bool>>
    inline value::value(Int v) : val{static_cast<int64_t>(v)} {}

    template<class UInt, std::enable_if_t<std::is_integral_v<UInt> && std::is_unsigned_v<UInt>, bool>>
    inline value::value(UInt v) : val{static_cast<uint64_t>(v)} {}

    template<class Real, std::enable_if_t<std::is_floating_point_v<Real>, bool>>
    inline value::value(Real v) : val{static_cast<double>(v)} {}

//----------------------------------------------------------------------------------------------------------------

    enum msgpack_identifier : uint8_t
    {
        MSGPACK_FALSE       = 0xc2,
        MSGPACK_TRUE        = 0xc3,
        MSGPACK_FIXINT_POS  = 0x7f,
        MSGPACK_U8          = 0xcc,
        MSGPACK_U16         = 0xcd
    };

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    inline void serialize(Stream& out, bool v)
    {
        const uint8_t format = v ? MSGPACK_TRUE : MSGPACK_FALSE;
        out((const char*)&format, 1);  
    }

    template<class Source>
    inline void deserialize(Source& in, bool& v)
    {
        uint8_t tmp{};
        in((char*)&tmp, 1);
        if      (tmp == MSGPACK_FALSE) v = false;
        else if (tmp == MSGPACK_TRUE)  v = true;
        else throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

    template<class Stream, class UInt, std::enable_if_t<std::is_integral_v<UInt> && std::is_unsigned_v<UInt>, bool>>
    inline void serialize(Stream& out, UInt v)
    {
        if (v <= MSGPACK_FIXINT_POS)
        {
            // positive fixint (7-bit positive integer)
            const uint8_t v8 = static_cast<uint8_t>(v);
            out((const char*)&v8, 1);
        }
        else if (v <= std::numeric_limits<uint8_t>::max())
        {
            // unsigned 8
            constexpr uint8_t format = MSGPACK_U8;
            const     uint8_t v8     = static_cast<uint8_t>(v);
            out((const char*)&format, 1);
            out((const char*)&v8, 1);
        }
        else if (v <= std::numeric_limits<uint16_t>::max())
        {
            // unsigned 16
            constexpr uint8_t format = MSGPACK_U16;
            const     uint16_t v16   = host_to_b16(static_cast<uint16_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v16, 2);
        }    
        else if (v <= std::numeric_limits<uint32_t>::max())
        {
            // unsigned 32
            constexpr uint8_t format = 0xce;
            const     uint32_t v32   = host_to_b32(static_cast<uint32_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v32, 4);
        }
        else
        {
            // unsigned 64
            constexpr uint8_t format = 0xcf;
            const     uint64_t v64   = host_to_b64(static_cast<uint64_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v64, 8);
        }
    }

    template<class Stream, class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, bool>>
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
            const     uint16_t v16   = host_to_b16(bit_cast<uint16_t>(static_cast<int16_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v16, 2);
        }    
        else if (v >= std::numeric_limits<int32_t>::min())
        {
            // negative - int32_t
            constexpr uint8_t format = 0xd2;
            const     uint32_t v32   = host_to_b32(bit_cast<uint32_t>(static_cast<int32_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v32, 4);
        }
        else
        {
            // negative - int64_T
            constexpr uint8_t format = 0xd3;
            const     uint64_t v64   = host_to_b64(bit_cast<uint64_t>(static_cast<int64_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v64, 8);
        }
    }

    template<class Source, class Int, std::enable_if_t<std::is_integral_v<Int>, bool>>
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
            v = host_to_b16(tmp);
        }
        else if (format == 0xce)
        {
            // unsigned 32
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = host_to_b32(tmp);
        }
        else if (format == 0xcf)
        {
            // unsigned 64
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = host_to_b64(tmp);
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
            v = bit_cast<int16_t>(host_to_b16(tmp));
        }
        else if (format == 0xd2)
        {
            // signed 32
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = bit_cast<int32_t>(host_to_b32(tmp));
        }
        else if (format == 0xd3)
        {
            // signed 64
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = bit_cast<int64_t>(host_to_b64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

    template<class Stream>
    inline void serialize(Stream& out, float v)
    {
        constexpr uint8_t  format   = 0xca;
        const     uint32_t tmp      = host_to_b32(bit_cast<uint32_t>(v)); 
        out((const char*)&format, 1);
        out((const char*)&tmp, 4);
    }

    template<class Stream>
    inline void serialize(Stream& out, double v)
    {
        constexpr uint8_t  format   = 0xcb;
        const     uint64_t tmp      = host_to_b64(bit_cast<uint64_t>(v)); 
        out((const char*)&format, 1);
        out((const char*)&tmp, 8);
    }

    template<class Source, class Float, std::enable_if_t<std::is_floating_point_v<Float>, bool>>
    inline void deserialize(Source& in, Float& v)
    {
        uint8_t format{};
        in((char*)&format, 1);

        if (format == 0xca)
        {
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = bit_cast<float>(host_to_b32(tmp));
        }
        else if (format == 0xcb)
        {
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = bit_cast<double>(host_to_b64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

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
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xdb;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
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
            size = host_to_b16(size16);
        }
        else if (format == 0xdb)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
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

//----------------------------------------------------------------------------------------------------------------

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
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(len));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xc6;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(len));
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
            size = host_to_b16(size16);
        }
        else if (format == 0xc6)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
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

//----------------------------------------------------------------------------------------------------------------

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
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xdd;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
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
            size = host_to_b16(size16);
        }
        else if (format == 0xdd)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
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

//----------------------------------------------------------------------------------------------------------------

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
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = 0xdf;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
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
            size = host_to_b16(size16);
        }
        else if (format == 0xdf)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

}
