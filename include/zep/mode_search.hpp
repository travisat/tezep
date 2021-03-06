#pragma once

#include "zep/mode.hpp"
#include <future>
#include <memory>
#include <regex>

namespace Zep
{

class ZepWindow;
class ZepMode_Search : public ZepMode
{
public:
    ZepMode_Search(ZepEditor& editor, ZepWindow& launchWindow, ZepWindow& window, ZepPath startPath);
    ~ZepMode_Search() override;

    void AddKeyPress(uint32_t key, uint32_t modifiers) override;
    void Begin() override;
    void Notify(std::shared_ptr<ZepMessage> message) override;

    static auto StaticName() -> const char*
    {
        return "Search";
    }
    auto Name() const -> const char* override
    {
        return StaticName();
    }

private:
    void GetSearchPaths(const ZepPath& path, std::vector<std::string>& ignore, std::vector<std::string>& include) const;
    void InitSearchTree();
    void ShowTreeResult();
    void UpdateTree();

    enum class OpenType
    {
        Replace,
        VSplit,
        HSplit,
        Tab
    };
    void OpenSelection(OpenType type);

private:
    // List of files found in the directory search
    struct FileSearchResult
    {
        ZepPath root;
        std::vector<ZepPath> paths;
        std::vector<std::string> lowerPaths;
    };

    // List of lines in the file result, with last found char
    struct SearchResult
    {
        uint32_t index = 0;
        uint32_t location = 0;
    };

    // A mapping from character distance to a list of lines
    struct IndexSet
    {
        std::multimap<uint32_t, SearchResult> indices;
    };

    bool fileSearchActive = false;
    bool treeSearchActive = false;

    // Results of the file search and the indexing threads
    std::future<std::shared_ptr<FileSearchResult>> m_indexResult;
    std::future<std::shared_ptr<IndexSet>> m_searchResult;

    // All files that can potentially match
    std::shared_ptr<FileSearchResult> m_spFilePaths;

    // A hierarchy of index results.
    // The 'top' of the tree is the most narrow finding from a set of 'n' characters
    // index a,b,c -> index b,c -> index c
    std::vector<std::shared_ptr<IndexSet>> m_indexTree;

    // What we are searching for
    std::string m_searchTerm;
    bool m_caseImportant = false;

    ZepWindow& m_launchWindow;
    ZepWindow& m_window;
    ZepPath m_startPath;
};

} // namespace Zep
