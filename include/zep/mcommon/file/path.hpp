#pragma once

#include "zep/mcommon/string/stringutils.hpp"

#include <system_error>
#include <chrono>
#include <sstream>
#include <fstream>
#include <utility>

namespace Zep
{

using file_time_type = std::chrono::system_clock::time_point;
class ZepPath
{
public:
    using const_iterator = std::vector<std::string>::const_iterator;
    ZepPath(std::string  strPath = std::string())
        : m_strPath(std::move(strPath))
    {
    }

    ZepPath(const char* pszZepPath)
        : m_strPath(pszZepPath)
    {
    }

    auto empty() const -> bool
    {
        return m_strPath.empty();
    }

    auto stem() const -> ZepPath
    {
        auto str = filename().string();
        size_t dot = str.find_last_of('.');
        if (dot != std::string::npos)
        {
            return str.substr(0, dot);
        }
        return str;
    }

    // TODO(unknown): Unit tests
    // (See SO solution for some examples)
    auto filename() const -> ZepPath
    {
        //https://stackoverflow.com/a/43283887/18942
        if (m_strPath.empty())
        {
            return {};
        }

        auto len = m_strPath.length();
        auto index = m_strPath.find_last_of("/\\");

        if (index == std::string::npos)
        {
            return m_strPath;
        }

        if (index + 1 >= len)
        {

            len--;
            index = m_strPath.substr(0, len).find_last_of("/\\");

            if (len == 0)
            {
                return m_strPath;
            }

            if (index == 0)
            {
                return m_strPath.substr(1, len - 1);
            }

            if (index == std::string::npos)
            {
                return m_strPath.substr(0, len);
            }

            return m_strPath.substr(index + 1, len - index - 1);
        }

        return m_strPath.substr(index + 1, len - index);
    }

    auto has_filename() const -> bool
    {
        return !filename().string().empty();
    }

    auto has_extension() const -> bool
    {
        return !extension().string().empty();
    }

    static auto is_absolute() -> bool 
    {
        return false;
    }

    auto extension() const -> ZepPath
    {
        if (!has_filename()) {
            return ZepPath();
}

        auto str = filename().string();
        size_t dot = str.find_last_of('.');
        if (dot != std::string::npos)
        {
            return str.substr(dot, str.length() - dot);
        }
        return ZepPath();
    }

    auto parent_path() const -> ZepPath
    {
        std::string strSplit;
        size_t sep = m_strPath.find_last_of("\\/");
        if (sep != std::string::npos)
        {
            return m_strPath.substr(0, sep);
        }
        return ZepPath("");
    }

    auto replace_extension(const std::string& extension) -> ZepPath&
    {
        size_t dot = m_strPath.find_last_of('.');
        if (dot != std::string::npos)
        {
            m_strPath = m_strPath.substr(0, dot - 1) + extension;
        }
        else
        {
            m_strPath += extension;
        }
        return *this;
    }

    auto c_str() const -> const char*
    {
        return m_strPath.c_str();
    }
    auto string() const -> std::string
    {
        return m_strPath;
    }

    auto operator==(const ZepPath& rhs) const -> bool
    {
        return m_strPath == rhs.string();
    }
    
    auto operator!=(const ZepPath& rhs) const -> bool
    {
        return m_strPath != rhs.string();
    }

    auto operator/(const ZepPath& rhs) const -> ZepPath
    {
        std::string temp = m_strPath;
        RTrim(temp, "\\/");
        if (temp.empty())
        {
            return ZepPath(rhs.string());
        }
        return ZepPath(temp + "/" + rhs.string());
    }

    operator std::string() const
    {
        return m_strPath;
    }

    auto operator<(const ZepPath& rhs) const -> bool
    {
        return m_strPath < rhs.string();
    }

    auto begin() const -> std::vector<std::string>::const_iterator
    {
        std::string can = string_replace(m_strPath, "\\", "/");
        m_components = string_split(can, "/");
        return m_components.begin();
    }

    auto end() const -> std::vector<std::string>::const_iterator
    {
        return m_components.end();
    }

private:
    mutable std::vector<std::string> m_components;
    std::string m_strPath;
};

auto path_get_relative(const ZepPath& from, const ZepPath& to) -> ZepPath;

/*
inline ZepPath absolute(const ZepPath& input)
{
    // Read the comments at the top of this file; this is certainly incorrect, and doesn't handle ../
    // It is sufficient for what we need though
    auto p = canonical(input);
    auto strAbs = string_replace(p.string(), "/.", "");
    return ZepPath(strAbs);
}

inline std::ostream& operator<<(std::ostream& rhs, const ZepPath& p)
{
    rhs << p.string();
    return rhs;
}

namespace copy_options
{
enum
{
    overwrite_existing = 1
};
};

inline bool copy_file(const ZepPath& source, const ZepPath& dest, uint32_t options)
{
    std::ifstream src(source.string(), std::ios::binary);
    std::ofstream dst(dest.string(), std::ios::binary);
    if (!src.is_open() || !dst.is_open())
    {
        return false;
    }
    dst << src.rdbuf();
    return true;
}

inline bool isDirExist(const std::string& ZepPath)
{
#if defined(_WIN32)
    struct _stat info;
    if (_stat(ZepPath.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info;
    if (stat(ZepPath.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
#endif
}

inline bool makeZepPath(const std::string& ZepPath)
{
#if defined(_WIN32)
    int ret = _mkdir(ZepPath.c_str());
#else
    mode_t mode = 0755;
    int ret = mkdir(ZepPath.c_str(), mode);
#endif
    if (ret == 0)
        return true;

    switch (errno)
    {
        case ENOENT:
            // parent didn't exist, try to create it
            {
                auto pos = ZepPath.find_last_of('/');
                if (pos == std::string::npos)
#if defined(_WIN32)
                    pos = ZepPath.find_last_of('\\');
                if (pos == std::string::npos)
#endif
                    return false;
                if (!makeZepPath(ZepPath.substr(0, pos)))
                    return false;
            }
            // now, try to create again
#if defined(_WIN32)
            return 0 == _mkdir(ZepPath.c_str());
#else
            return 0 == mkdir(ZepPath.c_str(), mode);
#endif

        case EEXIST:
            // done!
            return isDirExist(ZepPath);

        default:
            return false;
    }
}
inline bool create_directories(const ZepPath& source)
{
    return makeZepPath(source.string());
}

inline file_time_type last_write_time(const ZepPath& source)
{
    struct stat attr;
    std::string strSource = source.string();
    stat(strSource.c_str(), &attr);
    return std::chrono::system_clock::from_time_t(attr.st_mtime);
}
*/

} // namespace Zep
