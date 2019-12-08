#pragma once

#include "zep/buffer.hpp"

#include <atomic>
#include <future>
#include <memory>
#include <set>
#include <vector>

namespace Zep
{

enum class ThemeColor;

struct CommentEntry
{
    bool isStart;
    bool isMultiLine;
    uint32_t location;
    uint32_t entries;
};

namespace ZepSyntaxFlags
{
enum
{
    CaseInsensitive = (1 << 0)
};
} // namespace ZepSyntaxFlags

struct SyntaxData
{
    ThemeColor foreground = ThemeColor::Normal;
    ThemeColor background = ThemeColor::None;
    bool underline = false;
};

class ZepSyntaxAdorn;
class ZepSyntax : public ZepComponent
{
public:
    ZepSyntax(ZepBuffer& buffer,
        std::set<std::string> keywords = std::set<std::string>{},
        std::set<std::string> identifiers = std::set<std::string>{},
        uint32_t flags = 0);
    ~ZepSyntax() override;

    virtual auto GetSyntaxAt(int32_t offset) const -> SyntaxData;
    virtual void UpdateSyntax();
    virtual void Interrupt();
    virtual void Wait() const;

    virtual auto GetProcessedChar() const -> int32_t
    {
        return m_processedChar;
    }
    virtual auto GetText() const -> const std::vector<SyntaxData>&
    {
        return m_syntax;
    }
    void Notify(std::shared_ptr<ZepMessage> message) override;

private:
    virtual void QueueUpdateSyntax(BufferLocation startLocation, BufferLocation endLocation);

protected:
    ZepBuffer& m_buffer;
    std::vector<CommentEntry> m_commentEntries;
    std::vector<SyntaxData> m_syntax;
    std::future<void> m_syntaxResult;
    std::atomic<int32_t> m_targetChar = { 0 };
    std::atomic<int32_t> m_processedChar = { 0 };
    std::vector<uint32_t> m_multiCommentStarts;
    std::vector<uint32_t> m_multiCommentEnds;
    std::set<std::string> m_keywords;
    std::set<std::string> m_identifiers;
    std::atomic<bool> m_stop;
    std::vector<std::shared_ptr<ZepSyntaxAdorn>> m_adornments;
    uint32_t m_flags;
};

class ZepSyntaxAdorn : public ZepComponent
{
public:
    ZepSyntaxAdorn(ZepSyntax& syntax, ZepBuffer& buffer)
        : ZepComponent(syntax.GetEditor())
        , m_buffer(buffer)
        , m_syntax(syntax)
    {
    }

    virtual auto GetSyntaxAt(int32_t offset, bool& found) const -> SyntaxData = 0;

protected:
    ZepBuffer& m_buffer;
    ZepSyntax& m_syntax;
};

} // namespace Zep
