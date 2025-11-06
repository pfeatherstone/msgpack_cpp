#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <limits>
#include <utility>
#include <algorithm>
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
#if __cpp_concepts
#include <concepts>
#endif

//----------------------------------------------------------------------------------------------------------------

#if __cpp_concepts

namespace msgpackcpp
{
    template<class F> concept sink_type   = std::invocable<F, const char*, std::size_t>;
    template<class F> concept source_type = std::invocable<F, char*, std::size_t>;
}

#define SINK_TYPE   msgpackcpp::sink_type
#define SOURCE_TYPE msgpackcpp::source_type

#else
#define SINK_TYPE class
#define SOURCE_TYPE class
#endif

namespace msgpackcpp
{

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

    template<class Byte>
    constexpr bool is_byte = std::is_same_v<Byte, char>     || 
                             std::is_same_v<Byte, uint8_t>  ||
                             std::is_same_v<Byte, int8_t>;

    template<class T>
    using check_byte = std::enable_if_t<is_byte<T>, bool>;

    template<class T>
    using check_float = std::enable_if_t<std::is_floating_point_v<T>, bool>;

    template<class T>
    using check_sint = std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, bool>;

    template<class T>
    using check_uint = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool>;

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
                     std::vector<char>,
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

        template<class Int, check_sint<Int> = true>
        value(Int v);

        template<class UInt, check_uint<UInt> = true>
        value(UInt v);

        template<class Real, check_float<Real> = true>
        value(Real v);

        value(const char* v);
        value(std::string_view v);
        value(std::string v);
        value(std::vector<char> v);
        value(std::vector<value> v);
        value(std::map<std::string, value> v);
        value(std::initializer_list<value> v);

        size_t size() const noexcept;

        bool is_null()      const noexcept;
        bool is_bool()      const noexcept;
        bool is_int()       const noexcept;
        bool is_real()      const noexcept;
        bool is_str()       const noexcept;
        bool is_binary()    const noexcept;
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
        auto as_bin()       const -> const std::vector<char>&;
        auto as_bin()             -> std::vector<char>&;
        auto as_array()     const -> const std::vector<value>&;
        auto as_array()           -> std::vector<value>&;
        auto as_object()    const -> const std::map<std::string, value>&;
        auto as_object()          -> std::map<std::string, value>&;

        const value& at(const std::string& key) const;
        value&       at(const std::string& key);
        value&       operator[](const std::string& key);

        const value& operator[](size_t array_index) const;
        value&       operator[](size_t array_index);

        template<SINK_TYPE Sink>
        void pack(Sink& out) const;

        template<SOURCE_TYPE Source>
        void unpack(Source& in);
    };

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    void serialize(Sink& out, std::nullptr_t);

    template<SOURCE_TYPE Source>
    void deserialize(Source& in, std::nullptr_t);

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    void serialize(Sink& out, bool v);

    template<SOURCE_TYPE Source>
    void deserialize(Source& in, bool& v);
    
//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink, class UInt, check_uint<UInt> = true>
    void serialize(Sink& out, UInt v);

    template<SINK_TYPE Sink, class Int, check_sint<Int> = true>
    void serialize(Sink& out, Int v);

    template<SOURCE_TYPE Source, class Int, std::enable_if_t<std::is_integral_v<Int>, bool> = true>
    void deserialize(Source& in, Int& v);

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    void serialize(Sink& out, float v);

    template<SINK_TYPE Sink>
    void serialize(Sink& out, double v);

    template<SOURCE_TYPE Source, class Float, check_float<Float> = true>
    void deserialize(Source& in, Float& v);

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    void serialize_str_size(Sink& out, const uint32_t size);

    template<SOURCE_TYPE Source>
    void deserialize_str_size(Source& in, uint32_t& size);

    template<SINK_TYPE Sink>
    void serialize(Sink& out, std::string_view v);

    template<SINK_TYPE Sink>
    void serialize(Sink& out, const char* c_str);

    template<SOURCE_TYPE Source>
    void deserialize(Source& in, std::string& v);

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    void serialize_bin_size(Sink& out, const uint32_t len);

    template<SOURCE_TYPE Source>
    void deserialize_bin_size(Source& in, uint32_t& size);

    template<SINK_TYPE Sink>
    void serialize_bin_array(Sink& out, const char* data, const uint32_t len);

    template<SINK_TYPE Sink, class Alloc>
    void serialize(Sink& out, const std::vector<char, Alloc>& v);

    template<SINK_TYPE Sink, class Alloc>
    void serialize(Sink& out, const std::vector<uint8_t, Alloc>& v);

    template<SOURCE_TYPE Source, class Alloc>
    void deserialize(Source& in, std::vector<char, Alloc>& v);

    template<SOURCE_TYPE Source, class Alloc>
    void deserialize(Source& in, std::vector<uint8_t, Alloc>& v);

    template<SINK_TYPE Sink, std::size_t N>
    void serialize(Sink& out, const std::array<char, N>& v);

    template<SINK_TYPE Sink, std::size_t N>
    void serialize(Sink& out, const std::array<uint8_t, N>& v);

    template<SOURCE_TYPE Source, std::size_t N>
    void deserialize(Source& in, std::array<char, N>& v);

    template<SOURCE_TYPE Source, std::size_t N>
    void deserialize(Source& in, std::array<uint8_t, N>& v);

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    void serialize_array_size(Sink& out, const uint32_t size);

    template<SOURCE_TYPE Source>
    void deserialize_array_size(Source& in, uint32_t& size);

    template<SINK_TYPE Sink, class T, class Alloc>
    void serialize(Sink& out, const std::vector<T, Alloc>& v);

    template<SOURCE_TYPE Source, class T, class Alloc>
    void deserialize(Source& in, std::vector<T, Alloc>& v);

    template<SINK_TYPE Sink, class T, std::size_t N>
    void serialize(Sink& out, const std::array<T, N>& v);

    template<SOURCE_TYPE Source, class T, std::size_t N>
    void deserialize(Source& in, std::array<T, N>& v);

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    void serialize_map_size(Sink& out, const uint32_t size);

    template<SOURCE_TYPE Source>
    void deserialize_map_size(Source& in, uint32_t& size);

    template <
        SINK_TYPE Sink,
        class K, 
        class V, 
        class Compare = std::less<K>,
        class Alloc = std::allocator<std::pair<const K, V>>
    >
    void serialize(Sink& out, const std::map<K,V,Compare,Alloc>& map);

    template <
        SOURCE_TYPE Source, 
        class K, 
        class V, 
        class Compare = std::less<K>,
        class Alloc = std::allocator<std::pair<const K, V>>
    >
    void deserialize(Source& in, std::map<K,V,Compare,Alloc>& map);

    template <
        SINK_TYPE Sink,
        class K,
        class V,
        class Hash      = std::hash<K>,
        class KeyEqual  = std::equal_to<K>,
        class Alloc     = std::allocator<std::pair<const K, V>>
    >
    void serialize(Sink& out, const std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map);

    template <
        SOURCE_TYPE Source,
        class K,
        class V,
        class Hash      = std::hash<K>,
        class KeyEqual  = std::equal_to<K>,
        class Alloc     = std::allocator<std::pair<const K, V>>
    >
    void deserialize(Source& in, std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map);

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink, class... Args>
    void serialize(Sink& out, const std::tuple<Args...>& tpl);
    
    template<SOURCE_TYPE Source, class... Args>
    void deserialize(Source& in, std::tuple<Args...>& tpl);

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

#if __cpp_lib_endian
    constexpr bool is_little_endian()
    {
        return std::endian::native == std::endian::little;
    }
#else
    inline bool is_little_endian() 
    {
        constexpr uint32_t v{0x01020304};
        const auto*        ptr{reinterpret_cast<const unsigned char*>(&v)};
        return ptr[0] == 0x04;
    }
#endif

//----------------------------------------------------------------------------------------------------------------

    inline uint16_t host_to_b16(uint16_t v) { return is_little_endian() ? byte_swap16(v) : v; }
    inline uint32_t host_to_b32(uint32_t v) { return is_little_endian() ? byte_swap32(v) : v; }
    inline uint64_t host_to_b64(uint64_t v) { return is_little_endian() ? byte_swap64(v) : v; }
   
//----------------------------------------------------------------------------------------------------------------

    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

//----------------------------------------------------------------------------------------------------------------

    struct deserialization_error_category : std::error_category
    {
        const char* name() const noexcept override 
        {
            return "msgpack deserialization";
        }

        std::string message(int ev) const override
        {
            switch(static_cast<deserialization_error>(ev))
            {
            case OUT_OF_DATA: return "Ran out of data while deserializing";
            case BAD_FORMAT:  return "Found bad format";
            case BAD_SIZE:    return "Found bad size";
            case BAD_NAME:    return "Found bad name";
            default:          return "Unrecognised error";
            }
        }
    };

    inline std::error_code make_error_code(deserialization_error ec)
    {
        static const deserialization_error_category singleton;
        return {static_cast<int>(ec), singleton};
    }

//----------------------------------------------------------------------------------------------------------------

    inline value::value(std::nullptr_t)        : val{nullptr} {}
    inline value::value(bool v)                : val{v} {}
    inline value::value(const char* v)         : val(std::string(v)) {}
    inline value::value(std::string_view v)    : val(std::string(v)) {}
    inline value::value(std::string v)         : val{std::move(v)} {}
    inline value::value(std::vector<char> v)   : val{std::move(v)} {}
    inline value::value(std::vector<value> v)  : val{std::move(v)} {}
    inline value::value(std::map<std::string, value> v) : val{std::move(v)} {}

    template<class Int, check_sint<Int>>
    inline value::value(Int v) : val{static_cast<int64_t>(v)} {}

    template<class UInt, check_uint<UInt>>
    inline value::value(UInt v) : val{static_cast<uint64_t>(v)} {}

    template<class Real, check_float<Real>>
    inline value::value(Real v) : val{static_cast<double>(v)} {}

    inline value::value(std::initializer_list<value> v)
    {
        const bool is_object = std::all_of(begin(v), end(v), [](const auto& el) {
            return el.is_array() && el.size() == 2 && el[0].is_str();
        });

        if (is_object)
        {
            auto& map = val.emplace<std::map<std::string, value>>();
            for (const auto& el : v)
                map.emplace(el[0].as_str(), el[1]);
        }
        else
            val.emplace<std::vector<value>>(v);
    }

    inline size_t value::size() const noexcept
    {
        return std::visit(overloaded{
            [&](const std::vector<char>& v)             {return v.size();},
            [&](const std::vector<value>& v)            {return v.size();},
            [&](const std::map<std::string, value>& v)  {return v.size();},
            [&](std::nullptr_t)                         {return (size_t)0;},
            [&](const auto&)                            {return (size_t)1;}
        }, val);
    }

    inline bool value::is_null()   const noexcept {return std::holds_alternative<std::nullptr_t>(val);}
    inline bool value::is_bool()   const noexcept {return std::holds_alternative<bool>(val);}
    inline bool value::is_int()    const noexcept {return std::holds_alternative<int64_t>(val) || std::holds_alternative<uint64_t>(val);}
    inline bool value::is_real()   const noexcept {return std::holds_alternative<double>(val);}
    inline bool value::is_str()    const noexcept {return std::holds_alternative<std::string>(val);}
    inline bool value::is_binary() const noexcept {return std::holds_alternative<std::vector<char>>(val);}
    inline bool value::is_array()  const noexcept {return std::holds_alternative<std::vector<value>>(val);}
    inline bool value::is_object() const noexcept {return std::holds_alternative<std::map<std::string, value>>(val);}

    inline auto value::as_bool()      const -> bool                        {return std::get<bool>(val);}
    inline auto value::as_bool()            -> bool&                       {return std::get<bool>(val);}
    inline auto value::as_int64()     const -> int64_t                     {return std::get<int64_t>(val);}
    inline auto value::as_int64()           -> int64_t&                    {return std::get<int64_t>(val);}
    inline auto value::as_uint64()    const -> uint64_t                    {return std::get<uint64_t>(val);}
    inline auto value::as_uint64()          -> uint64_t&                   {return std::get<uint64_t>(val);}
    inline auto value::as_real()      const -> double                      {return std::get<double>(val);}
    inline auto value::as_real()            -> double&                     {return std::get<double>(val);}
    inline auto value::as_str()       const -> const std::string&          {return std::get<std::string>(val);}
    inline auto value::as_str()             -> std::string&                {return std::get<std::string>(val);}
    inline auto value::as_bin()       const -> const std::vector<char>&    {return std::get<std::vector<char>>(val);}
    inline auto value::as_bin()             -> std::vector<char>&          {return std::get<std::vector<char>>(val);}
    inline auto value::as_array()     const -> const std::vector<value>&   {return std::get<std::vector<value>>(val);}
    inline auto value::as_array()           -> std::vector<value>&         {return std::get<std::vector<value>>(val);}
    inline auto value::as_object()    const -> const std::map<std::string, value>& {return std::get<std::map<std::string, value>>(val);}
    inline auto value::as_object()          -> std::map<std::string, value>&       {return std::get<std::map<std::string, value>>(val);}

    inline const value& value::at(const std::string& key) const { return std::get<std::map<std::string, value>>(val).at(key); }
    inline value&       value::at(const std::string& key)       { return std::get<std::map<std::string, value>>(val).at(key); }  

    inline value& value::operator[](const std::string& key)
    {
        if (!std::holds_alternative<std::map<std::string, value>>(val))
            val.emplace<std::map<std::string, value>>();
        return std::get<std::map<std::string, value>>(val)[key];
    }

    inline const value& value::operator[](size_t array_index) const { return std::get<std::vector<value>>(val)[array_index]; }
    inline value&       value::operator[](size_t array_index)       { return std::get<std::vector<value>>(val)[array_index]; }

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

    constexpr bool format_is_bool(uint8_t f)        {return f == MSGPACK_FALSE || f == MSGPACK_TRUE;}
    constexpr bool format_is_float(uint8_t f)       {return f == MSGPACK_F32 || f == MSGPACK_F64;}
    constexpr bool format_is_fixint_pos(uint8_t f)  {return f <= MSGPACK_FIXINT_POS;}
    constexpr bool format_is_uint(uint8_t f)        {return format_is_fixint_pos(f) || f == MSGPACK_U8 || f == MSGPACK_U16 || f == MSGPACK_U32 || f == MSGPACK_U64;}
    constexpr bool format_is_fixint_neg(uint8_t f)  {return (f & 0b11100000) == MSGPACK_FIXINT_NEG;}
    constexpr bool format_is_sint(uint8_t f)        {return format_is_fixint_neg(f) || f == MSGPACK_I8 || f == MSGPACK_I16 || f == MSGPACK_I32 || f == MSGPACK_I64;}
    constexpr bool format_is_fixstr(uint8_t f)      {return (f & 0b11100000) == MSGPACK_FIXSTR;}
    constexpr bool format_is_string(uint8_t f)      {return format_is_fixstr(f) || f == MSGPACK_STR8 || f == MSGPACK_STR16 || f == MSGPACK_STR32;}
    constexpr bool format_is_binary(uint8_t f)      {return f == MSGPACK_BIN8 || f == MSGPACK_BIN16 || f == MSGPACK_BIN32;}
    constexpr bool format_is_fixarr(uint8_t f)      {return (f & 0b11110000) == MSGPACK_FIXARR;}
    constexpr bool format_is_array(uint8_t f)       {return format_is_fixarr(f) || f == MSGPACK_ARR16 || f == MSGPACK_ARR32;}
    constexpr bool format_is_fixmap(uint8_t f)      {return (f & 0b11110000) == MSGPACK_FIXMAP;}
    constexpr bool format_is_map(uint8_t f)         {return format_is_fixmap(f) || f == MSGPACK_MAP16 || f == MSGPACK_MAP32;}

//----------------------------------------------------------------------------------------------------------------

    template<SOURCE_TYPE Source>
    uint8_t read_format(Source& in)
    {
        uint8_t format{};
        in((char*)&format, 1);
        return format;
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    inline void serialize(Sink& out, std::nullptr_t)
    {
        constexpr uint8_t format = MSGPACK_NIL;
        out((const char*)&format, 1);
    }

    template<SOURCE_TYPE Source>
    inline void deserialize(Source& in, std::nullptr_t)
    {
        if (read_format(in) != MSGPACK_NIL) 
            throw std::system_error(BAD_FORMAT);
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    inline void serialize(Sink& out, bool v)
    {
        const uint8_t format = v ? MSGPACK_TRUE : MSGPACK_FALSE;
        out((const char*)&format, 1);  
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_(Source& /*in*/, uint8_t format, bool& v)
    {
        if      (format == MSGPACK_FALSE) v = false;
        else if (format == MSGPACK_TRUE)  v = true;
        else throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source>
    inline void deserialize(Source& in, bool& v)
    {
        deserialize_(in, read_format(in), v);
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink, class UInt, check_uint<UInt>>
    inline void serialize(Sink& out, UInt v)
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
            constexpr uint8_t format = MSGPACK_U32;
            const     uint32_t v32   = host_to_b32(static_cast<uint32_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v32, 4);
        }
        else
        {
            // unsigned 64
            constexpr uint8_t format = MSGPACK_U64;
            const     uint64_t v64   = host_to_b64(static_cast<uint64_t>(v));
            out((const char*)&format, 1);
            out((const char*)&v64, 8);
        }
    }

    template<SINK_TYPE Sink, class Int, check_sint<Int>>
    inline void serialize(Sink& out, Int v)
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
            constexpr uint8_t format = MSGPACK_I8;
            const     int8_t  v8     = static_cast<int8_t>(v);
            out((const char*)&format, 1);
            out((const char*)&v8, 1);
        }
        else if (v >= std::numeric_limits<int16_t>::min())
        {
            // negative - int16
            constexpr uint8_t format = MSGPACK_I16;
            const     uint16_t v16   = host_to_b16(bit_cast<uint16_t>(static_cast<int16_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v16, 2);
        }    
        else if (v >= std::numeric_limits<int32_t>::min())
        {
            // negative - int32_t
            constexpr uint8_t format = MSGPACK_I32;
            const     uint32_t v32   = host_to_b32(bit_cast<uint32_t>(static_cast<int32_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v32, 4);
        }
        else
        {
            // negative - int64_t
            constexpr uint8_t format = MSGPACK_I64;
            const     uint64_t v64   = host_to_b64(bit_cast<uint64_t>(static_cast<int64_t>(v)));
            out((const char*)&format, 1);
            out((const char*)&v64, 8);
        }
    }

    template<SOURCE_TYPE Source, class Int, std::enable_if_t<std::is_integral_v<Int>, bool> = true>
    inline void deserialize_(Source& in, uint8_t format, Int& v)
    {
        if (format_is_fixint_pos(format))
        {
            // positive fixint (7-bit positive integer)
            v = format;
        }
        else if (format_is_fixint_neg(format))
        {
            // negative fixing (5-bit negative integer)
            v = bit_cast<int8_t>(format);
        }
        else if (format == MSGPACK_U8)
        {
            // unsigned 8
            uint8_t tmp{};
            in((char*)&tmp, 1);
            v = tmp;
        }
        else if (format == MSGPACK_U16)
        {
            // unsigned 16
            uint16_t tmp{};
            in((char*)&tmp, 2);
            v = host_to_b16(tmp);
        }
        else if (format == MSGPACK_U32)
        {
            // unsigned 32
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = host_to_b32(tmp);
        }
        else if (format == MSGPACK_U64)
        {
            // unsigned 64
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = host_to_b64(tmp);
        }
        else if (format == MSGPACK_I8)
        {
            // signed 8
            int8_t tmp{};
            in((char*)&tmp, 1);
            v = tmp;
        }
        else if (format == MSGPACK_I16)
        {
            // signed 16
            uint16_t tmp{};
            in((char*)&tmp, 2);
            v = bit_cast<int16_t>(host_to_b16(tmp));
        }
        else if (format == MSGPACK_I32)
        {
            // signed 32
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = bit_cast<int32_t>(host_to_b32(tmp));
        }
        else if (format == MSGPACK_I64)
        {
            // signed 64
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = bit_cast<int64_t>(host_to_b64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source, class Int, std::enable_if_t<std::is_integral_v<Int>, bool>>
    inline void deserialize(Source& in, Int& v)
    {
        deserialize_(in, read_format(in), v);
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    inline void serialize(Sink& out, float v)
    {
        constexpr uint8_t  format   = MSGPACK_F32;
        const     uint32_t tmp      = host_to_b32(bit_cast<uint32_t>(v)); 
        out((const char*)&format, 1);
        out((const char*)&tmp, 4);
    }

    template<SINK_TYPE Sink>
    inline void serialize(Sink& out, double v)
    {
        constexpr uint8_t  format   = MSGPACK_F64;
        const     uint64_t tmp      = host_to_b64(bit_cast<uint64_t>(v)); 
        out((const char*)&format, 1);
        out((const char*)&tmp, 8);
    }

    template<SOURCE_TYPE Source, class Float, check_float<Float> = true>
    inline void deserialize_(Source& in, uint8_t format, Float& v)
    {
        if (format == MSGPACK_F32)
        {
            uint32_t tmp{};
            in((char*)&tmp, 4);
            v = bit_cast<float>(host_to_b32(tmp));
        }
        else if (format == MSGPACK_F64)
        {
            uint64_t tmp{};
            in((char*)&tmp, 8);
            v = bit_cast<double>(host_to_b64(tmp));
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source, class Float, check_float<Float>>
    inline void deserialize(Source& in, Float& v)
    {
        deserialize_(in, read_format(in), v);
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    inline void serialize_str_size(Sink& out, const uint32_t size)
    {
        if (size < 32)
        {
            const uint8_t format = MSGPACK_FIXSTR | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 256)
        {
            constexpr uint8_t format = MSGPACK_STR8;
            const     uint8_t size8  = static_cast<uint8_t>(size);
            out((const char*)&format, 1);
            out((const char*)&size8, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = MSGPACK_STR16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_STR32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_str_size_(Source& in, uint8_t format, uint32_t& size)
    {
        if (format_is_fixstr(format))
        {
            size = format & 0b00011111;
        }
        else if (format == MSGPACK_STR8)
        {
            uint8_t size8{};
            in((char*)&size8, 1);
            size = size8;
        }
        else if (format == MSGPACK_STR16)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_STR32)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_str_size(Source& in, uint32_t& size)
    {
        deserialize_str_size_(in, read_format(in), size);
    }

    template<SINK_TYPE Sink>
    inline void serialize(Sink& out, std::string_view v)
    {
        serialize_str_size(out, v.size());
        out(v.data(), v.size());
    }

    template<SINK_TYPE Sink>
    inline void serialize(Sink& out, const char* c_str)
    {
        serialize(out, std::string_view(c_str));
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_(Source& in, uint8_t format, std::string& v)
    {
        uint32_t size{};
        deserialize_str_size_(in, format, size);
        v.resize(size);
        in(v.data(), size);
    }

    template<SOURCE_TYPE Source>
    inline void deserialize(Source& in, std::string& v)
    {
        deserialize_(in, read_format(in), v);
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    inline void serialize_bin_size(Sink& out, const uint32_t len)
    {
        if (len < 256)
        {
            constexpr uint8_t format = MSGPACK_BIN8;
            const     uint8_t size8  = static_cast<uint8_t>(len);
            out((const char*)&format, 1);
            out((const char*)&size8, 1);
        }
        else if (len < 65536)
        {
            constexpr uint8_t  format = MSGPACK_BIN16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(len));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_BIN32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(len));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_bin_size_(Source& in, uint8_t format, uint32_t& size)
    {
        if (format == MSGPACK_BIN8)
        {
            uint8_t size8{};
            in((char*)&size8, 1);
            size = size8;
        }
        else if (format == MSGPACK_BIN16)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_BIN32)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_bin_size(Source& in, uint32_t& size)
    {
        deserialize_bin_size_(in, read_format(in), size);
    }

    template<SINK_TYPE Sink>
    inline void serialize_bin_array(Sink& out, const char* data, const uint32_t len)
    {
        serialize_bin_size(out, len);
        out(data, len);
    }

    template<SINK_TYPE Sink, class Alloc>
    inline void serialize(Sink& out, const std::vector<char, Alloc>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<SINK_TYPE Sink, class Alloc>
    inline void serialize(Sink& out, const std::vector<uint8_t, Alloc>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<SOURCE_TYPE Source, class Alloc>
    inline void deserialize_(Source& in, uint8_t format, std::vector<char, Alloc>& v)
    {
        uint32_t size{};
        deserialize_bin_size_(in, format, size);
        v.resize(size);
        in(v.data(), size);
    }

    template<SOURCE_TYPE Source, class Alloc>
    inline void deserialize(Source& in, std::vector<char, Alloc>& v)
    {
        deserialize_(in, read_format(in), v);
    }

    template<SOURCE_TYPE Source, class Alloc>
    inline void deserialize(Source& in, std::vector<uint8_t, Alloc>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        v.resize(size);
        in((char*)v.data(), size);
    }

    template<SINK_TYPE Sink, std::size_t N>
    inline void serialize(Sink& out, const std::array<char, N>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<SINK_TYPE Sink, std::size_t N>
    inline void serialize(Sink& out, const std::array<uint8_t, N>& v)
    {
        serialize_bin_array(out, (const char*)v.data(), v.size());
    }

    template<SOURCE_TYPE Source, std::size_t N>
    inline void deserialize(Source& in, std::array<char, N>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        if (size != N)
            throw std::system_error(BAD_SIZE);
        in((char*)v.data(), size);
    }

    template<SOURCE_TYPE Source, std::size_t N>
    inline void deserialize(Source& in, std::array<uint8_t, N>& v)
    {
        uint32_t size{};
        deserialize_bin_size(in, size);
        if (size != N)
            throw std::system_error(BAD_SIZE);
        in((char*)v.data(), size);
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    inline void serialize_array_size(Sink& out, const uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = MSGPACK_FIXARR | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = MSGPACK_ARR16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_ARR32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_array_size_(Source& in, uint8_t format, uint32_t& size)
    {
        if (format_is_fixarr(format))
        {
            size = format & 0b00001111;
        }
        else if (format == MSGPACK_ARR16)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_ARR32)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_array_size(Source& in, uint32_t& size)
    {
        deserialize_array_size_(in, read_format(in), size);
    }

    template<SINK_TYPE Sink, class T, class Alloc>
    inline void serialize(Sink& out, const std::vector<T, Alloc>& v)
    { 
        serialize_array_size(out, v.size());
        for (const auto& x : v)
            serialize(out, x);
    }

    template<SOURCE_TYPE Source, class T, class Alloc>
    inline void deserialize(Source& in, std::vector<T, Alloc>& v)
    {
        uint32_t size{};
        deserialize_array_size(in, size);
        v.resize(size);
        for (auto& x : v)
            deserialize(in, x);
    }

    template<SINK_TYPE Sink, class T, std::size_t N>
    inline void serialize(Sink& out, const std::array<T, N>& v)
    {
        serialize_array_size(out, v.size());
        for (const auto& x : v)
            serialize(out, x);
    }

    template<SOURCE_TYPE Source, class T, std::size_t N>
    inline void deserialize(Source& in, std::array<T, N>& v)
    {
        uint32_t size{};
        deserialize_array_size(in, size);
        if (size != N)
            throw std::system_error(BAD_SIZE);
        for (auto& x : v)
            deserialize(in, x);
    }

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink>
    inline void serialize_map_size(Sink& out, const uint32_t size)
    {
        if (size < 16)
        {
            const uint8_t format = MSGPACK_FIXMAP | static_cast<uint8_t>(size);
            out((const char*)&format, 1);
        }
        else if (size < 65536)
        {
            constexpr uint8_t  format = MSGPACK_MAP16;
            const     uint16_t size16 = host_to_b16(static_cast<uint16_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size16, 2);
        }
        else 
        {
            constexpr uint8_t  format = MSGPACK_MAP32;
            const     uint32_t size32 = host_to_b32(static_cast<uint32_t>(size));
            out((const char*)&format, 1);
            out((const char*)&size32, 4);
        }
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_map_size_(Source& in, uint8_t format, uint32_t& size)
    {
        if (format_is_fixmap(format))
        {
            size = format & 0b00001111;
        }
        else if (format == MSGPACK_MAP16)
        {
            uint16_t size16{};
            in((char*)&size16, 2);
            size = host_to_b16(size16);
        }
        else if (format == MSGPACK_MAP32)
        {
            uint32_t size32{};
            in((char*)&size32, 4);
            size = host_to_b32(size32);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source>
    inline void deserialize_map_size(Source& in, uint32_t& size)
    {
        deserialize_map_size_(in, read_format(in), size);
    }

    template <
        SINK_TYPE Sink,
        class K, 
        class V, 
        class Compare,
        class Alloc
    >
    inline void serialize(Sink& out, const std::map<K,V,Compare,Alloc>& map)
    {
        serialize_map_size(out, map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(out, k);
            serialize(out, v);
        }
    }

    template <
        SOURCE_TYPE Source,
        class K, 
        class V, 
        class Compare,
        class Alloc
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
        SINK_TYPE Sink,
        class K,
        class V,
        class Hash,
        class KeyEqual,
        class Alloc
    >
    inline void serialize(Sink& out, const std::unordered_map<K,V,Hash,KeyEqual,Alloc>& map)
    {
        serialize_map_size(out, map.size());
        
        for (const auto& [k,v] : map)
        {
            serialize(out, k);
            serialize(out, v);
        }
    }

    template <
        SOURCE_TYPE Source,
        class K,
        class V,
        class Hash,
        class KeyEqual,
        class Alloc
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

//----------------------------------------------------------------------------------------------------------------

    template<SINK_TYPE Sink, class... Args>
    inline void serialize(Sink& out, const std::tuple<Args...>& tpl)
    {
        serialize_array_size(out, sizeof...(Args));
        std::apply([&](auto&&... args) {
            (serialize(out, std::forward<decltype(args)>(args)),...);
        }, tpl);
    }

    template<SOURCE_TYPE Source, class... Args>
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

    template<SINK_TYPE Sink>
    inline void value::pack(Sink& out) const
    {
        std::visit(overloaded{
            [&](std::nullptr_t) {
                serialize(out, nullptr);
            },
            [&](const std::vector<value>& v) {
                serialize_array_size(out, v.size());
                for (const auto& el : v)
                    el.pack(out);
            },
            [&](const std::map<std::string, value>& m) {
                serialize_map_size(out, m.size());
                for (const auto& [k,v] : m)
                {
                    serialize(out, k);
                    v.pack(out);
                }
            },
            [&](const auto& v) {
                serialize(out, v);
            }
        }, val);
    }

    template<SOURCE_TYPE Source>
    inline void value::unpack(Source& in)
    {
        const uint8_t format = read_format(in);
        
        if (format == MSGPACK_NIL)
        {
            /*no-op*/
        }
        else if (format_is_bool(format))
        {
            bool v{};
            deserialize_(in, format, v);
            val = v;
        }
        else if (format_is_float(format))
        {
            double v{};
            deserialize_(in, format, v);
            val = v;
        }
        else if (format_is_uint(format))
        {
            uint64_t v{};
            deserialize_(in, format, v);
            val = v;
        }
        else if (format_is_sint(format))
        {
            int64_t v{};
            deserialize_(in, format, v);
            val = v;
        }
        else if (format_is_string(format))
        {
            std::string v;
            deserialize_(in, format, v);
            val = std::move(v);
        }
        else if (format_is_binary(format))
        {
            std::vector<char> v;
            deserialize_(in, format, v);
            val = std::move(v);
        }
        else if (format_is_array(format))
        {
            uint32_t size{};
            deserialize_array_size_(in, format, size);
            std::vector<value> v(size);
            for (auto& el : v)
                el.unpack(in);
            val = std::move(v);
        }
        else if (format_is_map(format))
        {
            uint32_t size{};
            deserialize_map_size_(in, format, size);
            std::map<std::string, value> m;
            for (size_t i{0} ; i < size ; ++i)
            {
                std::string k;
                value       v;
                deserialize(in, k);
                v.unpack(in);
                m.emplace(std::make_pair(std::move(k), std::move(v)));
            }
            val = std::move(m);
        }
        else
            throw std::system_error(BAD_FORMAT);
    }

    template<SOURCE_TYPE Source>
    inline value unpack(Source& in)
    {
        value jv;
        jv.unpack(in);
        return jv;
    }

//----------------------------------------------------------------------------------------------------------------

}
