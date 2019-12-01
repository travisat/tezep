#pragma once

#include "zep/zep_config.hpp"

#include "zep/mcommon/file/path.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>


namespace Zep
{

// Zep's view of the outside world in terms of files
// Below there is a version of this that will work on most platforms using std's <filesystem> for file operations
// If you want to expose your app's view of the world, you need to implement this minimal set of functions
class IZepFileSystem
{
public:
    virtual ~IZepFileSystem() = default;
    ;
    virtual auto Read(const ZepPath& filePath) -> std::string = 0;
    virtual auto Write(const ZepPath& filePath, const void* pData, size_t size) -> bool = 0;

    // The rootpath is either the git working directory or the app current working directory
    [[nodiscard]] virtual auto GetSearchRoot(const ZepPath& start) const -> ZepPath = 0;

    // The working directory is set at start of day to the app's working parameter directory
    [[nodiscard]] virtual auto GetWorkingDirectory() const -> const ZepPath& = 0;
    virtual void SetWorkingDirectory(const ZepPath& path) = 0;

    [[nodiscard]] virtual auto IsDirectory(const ZepPath& path) const -> bool = 0;
    [[nodiscard]] virtual auto IsReadOnly(const ZepPath& path) const -> bool = 0;
    [[nodiscard]] virtual auto Exists(const ZepPath& path) const -> bool = 0;

    // A callback API for scaning
    virtual void ScanDirectory(const ZepPath& path, std::function<bool(const ZepPath& path, bool& dont_recurse)> fnScan) const = 0;

    // Equivalent means 'the same file'
    [[nodiscard]] virtual auto Equivalent(const ZepPath& path1, const ZepPath& path2) const -> bool = 0;
    [[nodiscard]] virtual auto Canonical(const ZepPath& path) const -> ZepPath = 0;
};

// CPP File system - part of the standard C++ libraries
#if defined(ZEP_FEATURE_CPP_FILE_SYSTEM)

// A generic file system using cross platform fs:: and tinydir for searches
// This is typically the only one that is used for normal desktop usage.
// But you could make your own if your files were stored in a compressed folder, or the target system didn't have a traditional file system...
class ZepFileSystemCPP : public IZepFileSystem
{
public:
    ~ZepFileSystemCPP();
    ZepFileSystemCPP();
    virtual std::string Read(const ZepPath& filePath) override;
    virtual bool Write(const ZepPath& filePath, const void* pData, size_t size) override;
    virtual void ScanDirectory(const ZepPath& path, std::function<bool(const ZepPath& path, bool& dont_recurse)> fnScan) const override;
    virtual void SetWorkingDirectory(const ZepPath& path) override;
    virtual const ZepPath& GetWorkingDirectory() const override;
    virtual ZepPath GetSearchRoot(const ZepPath& start) const override;
    virtual bool IsDirectory(const ZepPath& path) const override;
    virtual bool IsReadOnly(const ZepPath& path) const override;
    virtual bool Exists(const ZepPath& path) const override;
    virtual bool Equivalent(const ZepPath& path1, const ZepPath& path2) const override;
    virtual ZepPath Canonical(const ZepPath& path) const override;

private:
    ZepPath m_workingDirectory;
};
#endif // CPP File system

} // namespace Zep
