#pragma once

#include <functional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Zep
{

inline auto CountUtf8BytesFromChar(unsigned int c) -> size_t
{
    if (c < 0x80)
    {
        return 1;
    }
    if (c < 0x800)
    {
        return 2;
    }
    if (c >= 0xdc00 && c < 0xe000)
    {
        return 0;
    }
    if (c >= 0xd800 && c < 0xdc00)
    {
        return 4;
    }
    return 3;
}

inline auto Utf8Length(const char* s) -> size_t
{
    size_t stringLength = 0;
    while (*s != 0)
    {
        size_t len = 1;
        while (len <= 4 && (*s != 0))
        {
            if ((*s++ & 0xc0) != 0x80)
            {
                break;
            }
            len++;
        }
        stringLength += len;
    }
    return stringLength;
}

auto string_replace(std::string subject, const std::string& search, const std::string& replace) -> std::string;
void string_replace_in_place(std::string& subject, const std::string& search, const std::string& replace);

// trim from beginning of string (left)
inline auto LTrim(std::string& s, const char* t = " \t\n\r\f\v") -> std::string&
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from end of string (right)
inline auto RTrim(std::string& s, const char* t = " \t\n\r\f\v") -> std::string&
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from both ends of string (left & right)
inline auto Trim(std::string& s, const char* t = " \t\n\r\f\v") -> std::string&
{
    return LTrim(RTrim(s, t), t);
}

template <typename T>
auto toString(const T& t) -> std::string
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

template <typename T>
auto fromString(const std::string& s) -> T
{
    std::istringstream stream(s);
    T t;
    stream >> t;
    return t;
}

inline auto makeWStr(const std::string& str) -> std::wstring
{
    return std::wstring(str.begin(), str.end());
}

auto string_from_wstring(const std::wstring& str) -> std::string;
auto string_tolower(const std::string& str) -> std::string;

struct StringId
{
    uint32_t id = 0;
    StringId() = default;
    StringId(const char* pszString);
    StringId(const std::string& str);
    StringId(uint32_t _id)
    {
        id = _id;
    }

    auto operator==(const StringId& rhs) const -> bool
    {
        return id == rhs.id;
    }
    auto operator=(const char* pszString) -> StringId&;
    auto operator=(const std::string& str) -> StringId&;
    auto operator<(const StringId& rhs) const -> bool
    {
        return id < rhs.id;
    }

    [[nodiscard]] auto ToString() const -> std::string
    {
        auto itr = stringLookup.find(id);
        if (itr == stringLookup.end())
        {
            return "murmur:" + std::to_string(id);
        }
        return itr->second;
    }

    static std::unordered_map<uint32_t, std::string> stringLookup;
};

inline auto operator<<(std::ostream& str, StringId id) -> std::ostream&
{
    str << id.ToString();
    return str;
}

void string_split(const std::string& text, const char* delims, std::vector<std::string>& tokens);
auto string_split(const std::string& text, const char* delims) -> std::vector<std::string>;
void string_split_lines(const std::string& text, std::vector<std::string>& tokens);
void string_split_each(const std::string& text, const char* delims, std::function<bool(size_t, size_t)> fn);
void string_split_each(char* text, size_t start, size_t end, const char* delims, std::function<bool(size_t, size_t)> fn);
auto string_first_of(const char* text, size_t start, size_t end, const char* delims) -> size_t;
auto string_first_not_of(const char* text, size_t start, size_t end, const char* delims) -> size_t;

inline auto string_equals(const std::string& str, const std::string& str2) -> bool
{
    return str == str2;
}
} // namespace Zep

namespace std
{
template <>
struct hash<Zep::StringId>
{
    auto operator()(const Zep::StringId& k) const -> std::size_t
    {
        // Compute individual hash values for first,
        // second and third and combine them using XOR
        // and bit shifting:

        return std::hash<uint32_t>()(k.id);
    }
};
} // namespace std

namespace Zep
{
inline auto string_equals(const StringId lhs, const StringId rhs) -> bool
{
    return lhs.id == rhs.id;
}
} // namespace Zep
