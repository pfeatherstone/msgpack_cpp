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
#include <array>
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

    struct sink_base
    {
        virtual void write(const char* data, size_t nbytes) = 0;
    };

    struct source_base
    {
        virtual void    read(char* buf, size_t nbytes)  = 0;
        virtual uint8_t peak()                          = 0;
        virtual size_t  remaining() const               = 0;
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

namespace std
{
    template <>
    struct is_error_code_enum<msgpackcpp::deserialization_error> : true_type {};
}

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

        void pack(sink_base& out) const;
        void unpack(source_base& in);
        static value unpack_static(source_base& in);
    };
    
//----------------------------------------------------------------------------------------------------------------

    template<class Byte>
    constexpr bool is_byte = std::is_same_v<Byte, char>     || 
                             std::is_same_v<Byte, uint8_t>  ||
                             std::is_same_v<Byte, int8_t>;

//----------------------------------------------------------------------------------------------------------------

    void serialize(sink_base& out, std::nullptr_t);
    void deserialize(source_base& in, std::nullptr_t);

//----------------------------------------------------------------------------------------------------------------

    void serialize(sink_base& out, bool v);
    void deserialize(source_base& in, bool& v);
    
//----------------------------------------------------------------------------------------------------------------

    template<class UInt, std::enable_if_t<std::is_integral_v<UInt> && std::is_unsigned_v<UInt>, bool> = true>
    void serialize(sink_base& out, UInt v);

    template<class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, bool> = true>
    void serialize(sink_base& out, Int v);

    template<class Int, std::enable_if_t<std::is_integral_v<Int>, bool> = true>
    void deserialize(source_base& in, Int& v);

//----------------------------------------------------------------------------------------------------------------

    void serialize(sink_base& out, float v);
    void serialize(sink_base& out, double v);

    template<class Float, std::enable_if_t<std::is_floating_point_v<Float>, bool> = true>
    void deserialize(source_base& in, Float& v);

//----------------------------------------------------------------------------------------------------------------

    void serialize_str_size(sink_base& out, const uint32_t size);
    void deserialize_str_size(source_base& in, uint32_t& size);
    void serialize(sink_base& out, std::string_view v);
    void serialize(sink_base& out, const char* c_str);
    void deserialize(source_base& in, std::string& v);

//----------------------------------------------------------------------------------------------------------------

    void serialize_bin_size(sink_base& out, const uint32_t len);
    void deserialize_bin_size(source_base& in, uint32_t& size);
    void serialize_bin_array(sink_base& out, const char* data, const uint32_t len);

    template<class Alloc>
    void serialize(sink_base& out, const std::vector<char, Alloc>& v);

    template<class Alloc>
    void serialize(sink_base& out, const std::vector<uint8_t, Alloc>& v);

    template<class Alloc>
    void deserialize(source_base& in, std::vector<char, Alloc>& v);

    template<class Alloc>
    void deserialize(source_base& in, std::vector<uint8_t, Alloc>& v);

    template<std::size_t N>
    void serialize(sink_base& out, const std::array<char, N>& v);

    template<std::size_t N>
    void serialize(sink_base& out, const std::array<uint8_t, N>& v);

    template<std::size_t N>
    void deserialize(source_base& in, std::array<char, N>& v);

    template<std::size_t N>
    void deserialize(source_base& in, std::array<uint8_t, N>& v);

//----------------------------------------------------------------------------------------------------------------

    void serialize_array_size(sink_base& out, const uint32_t size);
    void deserialize_array_size(source_base& in, uint32_t& size);

    template<class T, class Alloc>
    void serialize(sink_base& out, const std::vector<T, Alloc>& v);

    template<class T, class Alloc>
    void deserialize(source_base& in, std::vector<T, Alloc>& v);

    template<class T, std::size_t N>
    void serialize(sink_base& out, const std::array<T, N>& v);

    template<class T, std::size_t N>
    void deserialize(source_base& in, std::array<T, N>& v);

//----------------------------------------------------------------------------------------------------------------

    void serialize_map_size(sink_base& out, const uint32_t size);
    void deserialize_map_size(source_base& in, uint32_t& size);

    template <
        class K, 
        class V, 
        class Compare = std::less<K>,
        class Alloc = std::allocator<std::pair<const K, V>>
    >
    void serialize(sink_base& out, const std::map<K,V,Compare,Alloc>& map);

    template <
        class K, 
        class V, 
        class Compare = std::less<K>,
        class Alloc = std::allocator<std::pair<const K, V>>
    >
    void deserialize(source_base& in, std::map<K,V,Compare,Alloc>& map);

    template <
        class K,
        class V,
        class Hash      = std::hash<K>,
        class KeyEqual  = std::equal_to<K>,
        class Alloc     = std::allocator<std::pair<const K, V>>
    >
    void serialize(sink_base& out, const std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map);

    template <
        class K,
        class V,
        class Hash      = std::hash<K>,
        class KeyEqual  = std::equal_to<K>,
        class Alloc     = std::allocator<std::pair<const K, V>>
    >
    void deserialize(source_base& in, std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map);

//----------------------------------------------------------------------------------------------------------------

    template<class... Args>
    void serialize(sink_base& out, const std::tuple<Args...>& tpl);
    
    template<class... Args>
    void deserialize(source_base& in, std::tuple<Args...>& tpl);

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

    constexpr uint32_t byte_swap32(uint32_t v)
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
        MSGPACK_NIL         = 0xc0,
        MSGPACK_FALSE       = 0xc2,
        MSGPACK_TRUE        = 0xc3,
        MSGPACK_F32         = 0xca,
        MSGPACK_F64         = 0xcb,
        MSGPACK_FIXINT_POS  = 0x7f,
        MSGPACK_U8          = 0xcc,
        MSGPACK_U16         = 0xcd,
        MSGPACK_U32         = 0xce,
        MSGPACK_U64         = 0xcf,
        MSGPACK_FIXINT_NEG  = 0xe0,
        MSGPACK_I8          = 0xd0,
        MSGPACK_I16         = 0xd1,
        MSGPACK_I32         = 0xd2,
        MSGPACK_I64         = 0xd3,
        MSGPACK_FIXSTR      = 0xa0,
        MSGPACK_STR8        = 0xd9,
        MSGPACK_STR16       = 0xda,
        MSGPACK_STR32       = 0xdb,
        MSGPACK_BIN8        = 0xc4,
        MSGPACK_BIN16       = 0xc5,
        MSGPACK_BIN32       = 0xc6,
        MSGPACK_FIXARR      = 0X90,
        MSGPACK_ARR16       = 0xdc,
        MSGPACK_ARR32       = 0xdd,
        MSGPACK_FIXMAP      = 0x80,
        MSGPACK_MAP16       = 0xde,
        MSGPACK_MAP32       = 0xdf
    };
    
//----------------------------------------------------------------------------------------------------------------

    inline void serialize(sink_base& out, std::nullptr_t)
    {
        constexpr uint8_t format = MSGPACK_NIL;
        out.write((const char*)&format, 1);  
    }

    inline void deserialize(source_base& in, std::nullptr_t)
    {
        uint8_t format{};
        in.read((char*)&format, 1);
        if (format != MSGPACK_NIL) 
            throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

    inline void serialize(sink_base& out, bool v)
    {
        const uint8_t format = v ? MSGPACK_TRUE : MSGPACK_FALSE;
        out.write((const char*)&format, 1);  
    }

    inline void deserialize(source_base& in, bool& v)
    {
        uint8_t tmp{};
        in.read((char*)&tmp, 1);
        if      (tmp == MSGPACK_FALSE) v = false;
        else if (tmp == MSGPACK_TRUE)  v = true;
        else throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

    template<class UInt, std::enable_if_t<std::is_integral_v<UInt> && std::is_unsigned_v<UInt>, bool>>
    inline void serialize(sink_base& out, UInt v)
    {
        if (v <= MSGPACK_FIXINT_POS)
        {
            // positive fixint (7-bit positive integer)
            const uint8_t v8 = static_cast<uint8_t>(v);
            out.write((const char*)&v8, 1);
        }
        else if (v <= std::numeric_limits<uint8_t>::max())
        {
            // unsigned 8
            constexpr uint8_t format = MSGPACK_U8;
            const     uint8_t v8     = static_cast<uint8_t>(v);
            out.write((const char*)&format, 1);
            out.write((const char*)&v8, 1);
        }
        else if (v <= std::numeric_limits<uint16_t>::max())
        {
            // unsigned 16
            constexpr uint8_t format = MSGPACK_U16;
            const     uint16_t v16   = host_to_b16(static_cast<uint16_t>(v));
            out.write((const char*)&format, 1);
            out.write((const char*)&v16, 2);
        }    
        else if (v <= std::numeric_limits<uint32_t>::max())
        {
            // unsigned 32
            constexpr uint8_t format = MSGPACK_U32;
            const     uint32_t v32   = host_to_b32(static_cast<uint32_t>(v));
            out.write((const char*)&format, 1);
            out.write((const char*)&v32, 4);
        }
        else
        {
            // unsigned 64
            constexpr uint8_t format = MSGPACK_U64;
            const     uint64_t v64   = host_to_b64(static_cast<uint64_t>(v));
            out.write((const char*)&format, 1);
            out.write((const char*)&v64, 8);
        }
    }

    template<class Int, std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, bool>>
    inline void serialize(sink_base& out, Int v)
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
            out.write((const char*)&v8, 1);
        }
        else if (v >= std::numeric_limits<int8_t>::min())
        {
            // negative - int8
            constexpr uint8_t format = MSGPACK_I8;
            const     int8_t  v8     = static_cast<int8_t>(v);
            out.write((const char*)&format, 1);
            out.write((const char*)&v8, 1);
        }
        else if (v >= std::numeric_limits<int16_t>::min())
        {
            // negative - int16
            constexpr uint8_t format = MSGPACK_I16;
            const     uint16_t v16   = host_to_b16(bit_cast<uint16_t>(static_cast<int16_t>(v)));
            out.write((const char*)&format, 1);
            out.write((const char*)&v16, 2);
        }    
        else if (v >= std::numeric_limits<int32_t>::min())
        {
            // negative - int32_t
            constexpr uint8_t format = MSGPACK_I32;
            const     uint32_t v32   = host_to_b32(bit_cast<uint32_t>(static_cast<int32_t>(v)));
            out.write((const char*)&format, 1);
            out.write((const char*)&v32, 4);
        }
        else
        {
            // negative - int64_T
            constexpr uint8_t format = MSGPACK_I64;
            const     uint64_t v64   = host_to_b64(bit_cast<uint64_t>(static_cast<int64_t>(v)));
            out.write((const char*)&format, 1);
            out.write((const char*)&v64, 8);
        }
    }

    template<class Int, std::enable_if_t<std::is_integral_v<Int>, bool>>
    inline void deserialize(source_base& in, Int& v)
    {
        uint8_t format{};
        in.read((char*)&format, 1);

        if (format <= MSGPACK_FIXINT_POS)
        {
            // positive fixint (7-bit positive integer)
            v = format;
        }
        else if ((format & 0b11100000) == MSGPACK_FIXINT_NEG)
        {
            // negative fixing (5-bit negative integer)
            v = bit_cast<int8_t>(format);
        }
        else if (format == MSGPACK_U8)
        {
            // unsigned 8
            uint8_t tmp{};
            in.read((char*)&tmp, 1);
            v = tmp;
        }
        else if (format == MSGPACK_U16)
        {
            // unsigned 16
            uint16_t tmp{};
            in.read((char*)&tmp, 2);
            v = host_to_b16(tmp);
        }
        else if (format == MSGPACK_U32)
        {
            // unsigned 32
            uint32_t tmp{};
            in.read((char*)&tmp, 4);
            v = host_to_b32(tmp);
        }
        else if (format == MSGPACK_U64)
        {
            // unsigned 64
            uint64_t tmp{};
            in.read((char*)&tmp, 8);
            v = host_to_b64(tmp);
        }
        else if (format == MSGPACK_I8)
        {
            // signed 8
            int8_t tmp{};
            in.read((char*)&tmp, 1);
            v = tmp;
        }
        else if (format == MSGPACK_I16)
        {
            // signed 16
            uint16_t tmp{};
            in.read((char*)&tmp, 2);
            v = bit_cast<int16_t>(host_to_b16(tmp));
        }
        else if (format == MSGPACK_I32)
        {
            // signed 32
            uint32_t tmp{};
            in.read((char*)&tmp, 4);
            v = bit_cast<int32_t>(host_to_b32(tmp));
        }
        else if (format == MSGPACK_I64)
        {
            // signed 64
            uint64_t tmp{};
            in.read((char*)&tmp, 8);
            v = bit_cast<int64_t>(host_to_b64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

    inline void serialize(sink_base& out, float v)
    {
        constexpr uint8_t  format   = MSGPACK_F32;
        const     uint32_t tmp      = host_to_b32(bit_cast<uint32_t>(v)); 
        out.write((const char*)&format, 1);
        out.write((const char*)&tmp, 4);
    }

    inline void serialize(sink_base& out, double v)
    {
        constexpr uint8_t  format   = MSGPACK_F64;
        const     uint64_t tmp      = host_to_b64(bit_cast<uint64_t>(v)); 
        out.write((const char*)&format, 1);
        out.write((const char*)&tmp, 8);
    }

    template<class Float, std::enable_if_t<std::is_floating_point_v<Float>, bool>>
    inline void deserialize(source_base& in, Float& v)
    {
        uint8_t format{};
        in.read((char*)&format, 1);

        if (format == MSGPACK_F32)
        {
            uint32_t tmp{};
            in.read((char*)&tmp, 4);
            v = bit_cast<float>(host_to_b32(tmp));
        }
        else if (format == MSGPACK_F64)
        {
            uint64_t tmp{};
            in.read((char*)&tmp, 8);
            v = bit_cast<double>(host_to_b64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

    inline void serialize_str_size(sink_base& out, const uint32_t size)
    {
        if (size < 32)
        {
            const uint8_t format = MSGPACK_FIXSTR | static_cast<uint8_t>(size);
            out.write((const char*)&format, 1);
        }
        else if (size < 256)
        {
            constexpr uint8_t format = MSGPACK_STR8;
            const     uint8_t size8  = static_cast<uint8_t>(size);
            out.write((const char*)&format, 1);
            out.write((const char*)&size8, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = MSGPACK_STR16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out.write((const char*)&format, 1);
            out.write((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_STR32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
            out.write((const char*)&format, 1);
            out.write((const char*)&size32, 4);
        }
    }

    inline void deserialize_str_size(source_base& in, uint32_t& size)
    {
        uint8_t format{};
        in.read((char*)&format, 1);

        if ((format & 0b11100000) == MSGPACK_FIXSTR)
        {
            size = format & 0b00011111;
        }
        else if (format == MSGPACK_STR8)
        {
            uint8_t size8{};
            in.read((char*)&size8, 1);
            size = size8;
        }
        else if (format == MSGPACK_STR16)
        {
            uint16_t size16{};
            in.read((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_STR32)
        {
            uint32_t size32{};
            in.read((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    inline void serialize(sink_base& out, std::string_view v)
    {
        serialize_str_size(out, v.size());
        out.write(v.data(), v.size());
    }

    inline void serialize(sink_base& out, const char* c_str)
    {
        serialize(out, std::string_view(c_str));
    }

    inline void deserialize(source_base& in, std::string& v)
    {
        uint32_t size{};
        deserialize_str_size(in, size);
        v.resize(size);
        in.read(v.data(), size);
    }

//----------------------------------------------------------------------------------------------------------------

    inline void serialize_bin_size(sink_base& out, const uint32_t len)
    {
        if (len < 256)
        {
            constexpr uint8_t format = MSGPACK_BIN8;
            const     uint8_t size8  = static_cast<uint8_t>(len);
            out.write((const char*)&format, 1);
            out.write((const char*)&size8, 1);
        }
        else if (len < 65536)
        {
            constexpr uint8_t  format = MSGPACK_BIN16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(len));
            out.write((const char*)&format, 1);
            out.write((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_BIN32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(len));
            out.write((const char*)&format, 1);
            out.write((const char*)&size32, 4);
        }
    }

    inline void deserialize_bin_size(source_base& in, uint32_t& size)
    {
        uint8_t format{};
        in.read((char*)&format, 1);

        if (format == MSGPACK_BIN8)
        {
            uint8_t size8{};
            in.read((char*)&size8, 1);
            size = size8;
        }
        else if (format == MSGPACK_BIN16)
        {
            uint16_t size16{};
            in.read((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_BIN32)
        {
            uint32_t size32{};
            in.read((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    inline void serialize_bin_array(sink_base& out, const char* data, const uint32_t len)
    {
        serialize_bin_size(out, len);
        out.write(data, len);
    }

    template<class Alloc>
    inline void serialize(sink_base& out, const std::vector<char, Alloc>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<class Alloc>
    inline void serialize(sink_base& out, const std::vector<uint8_t, Alloc>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<class Alloc>
    inline void deserialize(source_base& in, std::vector<char, Alloc>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        v.resize(size);
        in.read(v.data(), size);
    }

    template<class Alloc>
    inline void deserialize(source_base& in, std::vector<uint8_t, Alloc>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        v.resize(size);
        in.read((char*)v.data(), size);
    }

    template<std::size_t N>
    inline void serialize(sink_base& out, const std::array<char, N>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<std::size_t N>
    inline void serialize(sink_base& out, const std::array<uint8_t, N>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<std::size_t N>
    inline void deserialize(source_base& in, std::array<char, N>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        if (size != N)
            throw std::system_error(BAD_SIZE);
        in.read((char*)v.data(), size);
    }

    template<std::size_t N>
    inline void deserialize(source_base& in, std::array<uint8_t, N>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        if (size != N)
            throw std::system_error(BAD_SIZE);
        in.read((char*)v.data(), size);
    }

//----------------------------------------------------------------------------------------------------------------

    inline void serialize_array_size(sink_base& out, const uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = MSGPACK_FIXARR | static_cast<uint8_t>(size);
            out.write((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = MSGPACK_ARR16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out.write((const char*)&format, 1);
            out.write((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_ARR32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
            out.write((const char*)&format, 1);
            out.write((const char*)&size32, 4);
        }
    }

    inline void deserialize_array_size(source_base& in, uint32_t& size)
    {
        uint8_t format{};
        in.read((char*)&format, 1);

        if ((format & 0b11110000) == MSGPACK_FIXARR)
        {
            size = format & 0b00001111;
        }
        else if (format == MSGPACK_ARR16)
        {
            uint16_t size16{};
            in.read((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_ARR32)
        {
            uint32_t size32{};
            in.read((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<class T, class Alloc>
    inline void serialize(sink_base& out, const std::vector<T, Alloc>& v)
    { 
        serialize_array_size(out, v.size());
        for (const auto& x : v)
            serialize(out, x);
    }

    template<class T, class Alloc>
    inline void deserialize(source_base& in, std::vector<T, Alloc>& v)
    {
        uint32_t size{};
        deserialize_array_size(in, size);
        v.resize(size);
        for (auto& x : v)
            deserialize(in, x);
    }

    template<class T, std::size_t N>
    inline void serialize(sink_base& out, const std::array<T, N>& v)
    {
        serialize_array_size(out, v.size());
        for (const auto& x : v)
            serialize(out, x);
    }

    template<class T, std::size_t N>
    inline void deserialize(source_base& in, std::array<T, N>& v)
    {
        uint32_t size{};
        deserialize_array_size(in, size);
        if (size != N)
            throw std::system_error(BAD_SIZE);
        for (auto& x : v)
            deserialize(in, x);
    }

//----------------------------------------------------------------------------------------------------------------

    inline void serialize_map_size(sink_base& out, const uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = MSGPACK_FIXMAP | static_cast<uint8_t>(size);
            out.write((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = MSGPACK_MAP16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out.write((const char*)&format, 1);
            out.write((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_MAP32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
            out.write((const char*)&format, 1);
            out.write((const char*)&size32, 4);
        }
    }

    inline void deserialize_map_size(source_base& in, uint32_t& size)
    {
        uint8_t format{};
        in.read((char*)&format, 1);

        if ((format & 0b11110000) == MSGPACK_FIXMAP)
        {
            size = format & 0b00001111;
        }
        else if (format == MSGPACK_MAP16)
        {
            uint16_t size16{};
            in.read((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_MAP32)
        {
            uint32_t size32{};
            in.read((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

     template <
        class K, 
        class V, 
        class Compare,
        class Alloc
    >
    inline void serialize(sink_base& out, const std::map<K,V,Compare,Alloc>& map)
    {
        serialize_map_size(out, map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(out, k);
            serialize(out, v);
        }
    }

    template <
        class K, 
        class V, 
        class Compare,
        class Alloc
    >
    inline void deserialize(source_base& in, std::map<K,V,Compare,Alloc>& map)
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
        class K,
        class V,
        class Hash,
        class KeyEqual,
        class Alloc
    >
    inline void serialize(sink_base& out, const std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map)
    {
        serialize_map_size(out, map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(out, k);
            serialize(out, v);
        }
    }

    template <
        class K,
        class V,
        class Hash,
        class KeyEqual,
        class Alloc
    >
    inline void deserialize(source_base& in, std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map)
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

//----------------------------------------------------------------------------------------------------------------

    template<class... Args>
    inline void serialize(sink_base& out, const std::tuple<Args...>& tpl)
    {
        serialize_array_size(out, sizeof...(Args));
        std::apply([&](auto&&... args) {
            (serialize(out, std::forward<decltype(args)>(args)),...);
        }, tpl);
    }

    template<class... Args>
    inline void deserialize(source_base& in, std::tuple<Args...>& tpl)
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

}
