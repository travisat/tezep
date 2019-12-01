#include "zep/filesystem.hpp"

#include <fstream>

#include "zep/mcommon/logger.hpp"
#include "zep/mcommon/string/stringutils.hpp"

#undef ERROR

#if defined(ZEP_FEATURE_CPP_FILE_SYSTEM)

#if !defined(__APPLE__)
#include <experimental/filesystem>
namespace cpp_fs = std::experimental::filesystem::v1;
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif

namespace Zep
{

ZepFileSystemCPP::ZepFileSystemCPP()
{
#if defined(__APPLE__)
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        m_workingDirectory = ZepPath(std::string(cwd));
    }
#else
    m_workingDirectory = ZepPath(cpp_fs::current_path().string());
#endif
}

ZepFileSystemCPP::~ZepFileSystemCPP()
{
}

void ZepFileSystemCPP::SetWorkingDirectory(const ZepPath& path)
{
    m_workingDirectory = path;
}

const ZepPath& ZepFileSystemCPP::GetWorkingDirectory() const
{
    return m_workingDirectory;
}

bool ZepFileSystemCPP::IsDirectory(const ZepPath& path) const
{
#if defined(__APPLE__)
    struct stat s;
    auto strPath = path.string();
    if (stat(strPath.c_str(), &s) == 0)
    {
        if (s.st_mode & S_IFDIR)
        {
            //it's a directory
            return true;
        }
    }
    return false;
#else
    return cpp_fs::is_directory(path.string());
#endif
}

bool ZepFileSystemCPP::IsReadOnly(const ZepPath& path) const
{
#if defined(__APPLE__)
    struct stat s;
    auto strPath = path.string();
    if (stat(strPath.c_str(), &s) == 0)
    {
        if (s.st_mode & S_IWRITE)
        {
            // Can write, so not read only!
            return false;
        }
    }
    return true;
#else
    auto perms = cpp_fs::status(path.string()).permissions();
    if ((perms & cpp_fs::perms::owner_write) == cpp_fs::perms::owner_write)
    {
        return false;
    }
    return true;
#endif
}

std::string ZepFileSystemCPP::Read(const ZepPath& fileName)
{
    std::ifstream in(fileName, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(size_t(in.tellg()));
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return (contents);
    }
    else
    {
        LOG(typelog::ERROR) << "File Not Found: " << fileName.string();
    }
    return std::string();
}

bool ZepFileSystemCPP::Write(const ZepPath& fileName, const void* pData, size_t size)
{
    FILE* pFile;
    pFile = fopen(fileName.string().c_str(), "wb");
    if (!pFile)
    {
        return false;
    }
    fwrite(pData, sizeof(uint8_t), size, pFile);
    fclose(pFile);
    return true;
}

void ZepFileSystemCPP::ScanDirectory(const ZepPath& path, std::function<bool(const ZepPath& path, bool& dont_recurse)> fnScan) const
{
    // Not on apple yet!
#ifndef __APPLE__
    for (auto itr = cpp_fs::recursive_directory_iterator(path.string());
         itr != cpp_fs::recursive_directory_iterator();
         itr++)
    {
        auto p = ZepPath(itr->path().string());

        bool recurse = true;
        if (!fnScan(p, recurse))
            return;

        if (!recurse && itr.recursion_pending())
        {
            itr.disable_recursion_pending();
        }
    }
#else
    (void)path;
    (void)fnScan;
#endif
}

bool ZepFileSystemCPP::Exists(const ZepPath& path) const
{
#if defined(__APPLE__)
    try
    {
        std::ifstream ifile(path.string());
        return (bool)ifile;
    }
    catch (std::exception&)
    {
    }
    return false;
#else
    try
    {
        return cpp_fs::exists(path.string());
    }
    catch (cpp_fs::filesystem_error& err)
    {
        throw std::runtime_error(err.what());
    }
#endif
}

bool ZepFileSystemCPP::Equivalent(const ZepPath& path1, const ZepPath& path2) const
{
#if defined(__APPLE__)
    return Canonical(path1).string() == Canonical(path2).string();
#else
    try
    {
        // The below API expects existing files!  Best we can do is direct compare of paths
        if (!cpp_fs::exists(path1.string()) || !cpp_fs::exists(path2.string()))
        {
            return Canonical(path1).string() == Canonical(path2).string();
        }
        return cpp_fs::equivalent(path1.string(), path2.string());
    }
    catch (cpp_fs::filesystem_error& err)
    {
        throw std::runtime_error(err.what());
    }
#endif
}

ZepPath ZepFileSystemCPP::Canonical(const ZepPath& path) const
{
#if defined(__APPLE__)
    return ZepPath(string_replace(path.string(), "\\", "/"));
#else
    try
    {
        return ZepPath(cpp_fs::canonical(path.string()).string());
    }
    catch (cpp_fs::filesystem_error& err)
    {
        throw std::runtime_error(err.what());
    }
#endif
}

ZepPath ZepFileSystemCPP::GetSearchRoot(const ZepPath& start) const
{
    auto findStartPath = [&](const ZepPath& startPath) {
        if (!startPath.empty())
        {
            auto testPath = startPath;
            if (!IsDirectory(testPath))
            {
                testPath = testPath.parent_path();
            }

            while (!testPath.empty() && IsDirectory(testPath))
            {
                bool foundDir = false;

                // Look in this dir
                ScanDirectory(testPath, [&](const ZepPath& p, bool& recurse) -> bool {
                    // Not looking at sub folders
                    recurse = false;

                    // Found the .git repo
                    if (p.extension() == ".git" && IsDirectory(p))
                    {
                        foundDir = true;

                        // Quit search
                        return false;
                    }
                    return true;
                });

                // If found,  return it as the path we need
                if (foundDir)
                {
                    return testPath;
                }

                testPath = testPath.parent_path();
            }
        }
        return startPath;
    };

    ZepPath workingDir = GetWorkingDirectory();
    auto startPath = findStartPath(start);
    if (startPath.empty())
    {
        startPath = findStartPath(workingDir);
        if (startPath.empty())
        {
            startPath = GetWorkingDirectory();
        }
    }

    // Failure case, just use current path
    if (startPath.empty())
    {
        startPath = start;
    }
    return startPath;
}

} // namespace Zep

#endif // CPP_FILESYSTEM
