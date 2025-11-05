#include <algorithm>
#include "msgpack.h"

namespace msgpackcpp
{

//----------------------------------------------------------------------------------------------------------------

    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

//----------------------------------------------------------------------------------------------------------------

    value::value(std::nullptr_t)        : val{nullptr} {}
    value::value(bool v)                : val{v} {}
    value::value(const char* v)         : val(std::string(v)) {}
    value::value(std::string_view v)    : val(std::string(v)) {}
    value::value(std::string v)         : val{std::move(v)} {}
    value::value(std::vector<char> v)   : val{std::move(v)} {}
    value::value(std::vector<value> v)  : val{std::move(v)} {}
    value::value(std::map<std::string, value> v) : val{std::move(v)} {}

    value::value(std::initializer_list<value> v)
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

//----------------------------------------------------------------------------------------------------------------
    
    size_t value::size() const noexcept
    {
        return std::visit(overloaded{
            [&](const std::vector<char>& v)             {return v.size();},
            [&](const std::vector<value>& v)            {return v.size();},
            [&](const std::map<std::string, value>& v)  {return v.size();},
            [&](std::nullptr_t)                         {return (size_t)0;},
            [&](const auto&)                            {return (size_t)1;}
        }, val);
    }

//----------------------------------------------------------------------------------------------------------------
 
    bool value::is_null()   const noexcept {return std::holds_alternative<std::nullptr_t>(val);}
    bool value::is_bool()   const noexcept {return std::holds_alternative<bool>(val);}
    bool value::is_int()    const noexcept {return std::holds_alternative<int64_t>(val) || std::holds_alternative<uint64_t>(val);}
    bool value::is_real()   const noexcept {return std::holds_alternative<double>(val);}
    bool value::is_str()    const noexcept {return std::holds_alternative<std::string>(val);}
    bool value::is_binary() const noexcept {return std::holds_alternative<std::vector<char>>(val);}
    bool value::is_array()  const noexcept {return std::holds_alternative<std::vector<value>>(val);}
    bool value::is_object() const noexcept {return std::holds_alternative<std::map<std::string, value>>(val);}

//----------------------------------------------------------------------------------------------------------------

    auto value::as_bool()      const -> bool                        {return std::get<bool>(val);}
    auto value::as_bool()            -> bool&                       {return std::get<bool>(val);}
    auto value::as_int64()     const -> int64_t                     {return std::get<int64_t>(val);}
    auto value::as_int64()           -> int64_t&                    {return std::get<int64_t>(val);}
    auto value::as_uint64()    const -> uint64_t                    {return std::get<uint64_t>(val);}
    auto value::as_uint64()          -> uint64_t&                   {return std::get<uint64_t>(val);}
    auto value::as_real()      const -> double                      {return std::get<double>(val);}
    auto value::as_real()            -> double&                     {return std::get<double>(val);}
    auto value::as_str()       const -> const std::string&          {return std::get<std::string>(val);}
    auto value::as_str()             -> std::string&                {return std::get<std::string>(val);}
    auto value::as_bin()       const -> const std::vector<char>&    {return std::get<std::vector<char>>(val);}
    auto value::as_bin()             -> std::vector<char>&          {return std::get<std::vector<char>>(val);}
    auto value::as_array()     const -> const std::vector<value>&   {return std::get<std::vector<value>>(val);}
    auto value::as_array()           -> std::vector<value>&         {return std::get<std::vector<value>>(val);}
    auto value::as_object()    const -> const std::map<std::string, value>& {return std::get<std::map<std::string, value>>(val);}
    auto value::as_object()          -> std::map<std::string, value>&       {return std::get<std::map<std::string, value>>(val);}

//----------------------------------------------------------------------------------------------------------------

    const value& value::at(const std::string& key) const { return std::get<std::map<std::string, value>>(val).at(key); }
    value&       value::at(const std::string& key)       { return std::get<std::map<std::string, value>>(val).at(key); }  

    value& value::operator[](const std::string& key)
    {
        if (!std::holds_alternative<std::map<std::string, value>>(val))
            val.emplace<std::map<std::string, value>>();
        return std::get<std::map<std::string, value>>(val)[key];
    }

//----------------------------------------------------------------------------------------------------------------

    const value& value::operator[](size_t array_index) const { return std::get<std::vector<value>>(val)[array_index]; }
    value&       value::operator[](size_t array_index)       { return std::get<std::vector<value>>(val)[array_index]; }

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

    std::error_code make_error_code(deserialization_error ec)
    {
        static const deserialization_error_category singleton;
        return {static_cast<int>(ec), singleton};
    }

//----------------------------------------------------------------------------------------------------------------

    void value::pack(sink_base& out) const
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

    void value::unpack(source_base& in)
    {
        const uint8_t format = in.peak();
        
        if (format == MSGPACK_NIL)
        {
            deserialize(in, nullptr);
        }
        else if (format == MSGPACK_FALSE || format == MSGPACK_TRUE)
        {
            bool v{};
            deserialize(in, v);
            val = v;
        }
        else if (format == MSGPACK_F32 || format == MSGPACK_F64)
        {
            double v{};
            deserialize(in, v);
            val = v;
        }
        else if (format < MSGPACK_FIXINT_POS || format == MSGPACK_U8 || format == MSGPACK_U16 || format == MSGPACK_U32 || format == MSGPACK_U64)
        {
            uint64_t v{};
            deserialize(in, v);
            val = v;
        }
        else if ((format & 0b11100000) == MSGPACK_FIXINT_NEG || format == MSGPACK_I8 || format == MSGPACK_I16 || format == MSGPACK_I32 || format == MSGPACK_I64)
        {
            int64_t v{};
            deserialize(in, v);
            val = v;
        }
        else if ((format & 0b11100000) == MSGPACK_FIXSTR || format == MSGPACK_STR8 || format == MSGPACK_STR16 || format == MSGPACK_STR32)
        {
            std::string v;
            deserialize(in, v);
            val = std::move(v);
        }
        else if (format == MSGPACK_BIN8 || format == MSGPACK_BIN16 || format == MSGPACK_BIN32)
        {
            std::vector<char> v;
            deserialize(in, v);
            val = std::move(v);
        }
        else if ((format & 0b11110000) == MSGPACK_FIXARR || format == MSGPACK_ARR16 || format == MSGPACK_ARR32)
        {
            uint32_t size{};
            deserialize_array_size(in, size);
            std::vector<value> v(size);
            for (auto& el : v)
                el.unpack(in);
            val = std::move(v);
        }
        else if ((format & 0b11110000) == MSGPACK_FIXMAP || format == MSGPACK_MAP16 || format == MSGPACK_MAP32)
        {
            uint32_t size{};
            deserialize_map_size(in, size);
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

    value value::unpack_static(source_base& in)
    {
        value jv;
        jv.unpack(in);
        return jv;
    }

//----------------------------------------------------------------------------------------------------------------

}