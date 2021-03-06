#pragma once

#include "zep/mode.hpp"

namespace Zep
{

namespace CommandFlags
{
enum
{
    GroupBoundary = (1 << 0),
};
} // namespace CommandFlags

class ZepCommand
{
public:
    explicit ZepCommand(ZepBuffer& mode, BufferLocation cursorBefore = -1, BufferLocation cursorAfter = -1)
        : m_buffer(mode)
        , m_cursorBefore(cursorBefore)
        , m_cursorAfter(cursorAfter)
    {
    }

    virtual ~ZepCommand() = default;

    virtual void Redo() = 0;
    virtual void Undo() = 0;

    virtual void SetFlags(uint32_t flags)
    {
        m_flags = flags;
    }
    [[nodiscard]] virtual auto GetFlags() const -> uint32_t
    {
        return m_flags;
    }
    [[nodiscard]] virtual auto GetCursorAfter() const -> BufferLocation
    {
        return m_cursorAfter;
    }
    [[nodiscard]] virtual auto GetCursorBefore() const -> BufferLocation
    {
        return m_cursorBefore;
    }

protected:
    ZepBuffer& m_buffer;
    uint32_t m_flags = 0;
    BufferLocation m_cursorBefore = -1;
    BufferLocation m_cursorAfter = -1;
};

class ZepCommand_DeleteRange : public ZepCommand
{
public:
    ZepCommand_DeleteRange(ZepBuffer& buffer, const BufferLocation& startOffset, const BufferLocation& endOffset, const BufferLocation& cursor = BufferLocation{ -1 }, const BufferLocation& cursorAfter = BufferLocation{ -1 });
    ~ZepCommand_DeleteRange() override = default;
    ;

    void Redo() override;
    void Undo() override;

    BufferLocation m_startOffset;
    BufferLocation m_endOffset;

    std::string m_deleted;
};

enum class ReplaceRangeMode
{
    Fill,
    Replace,
};

class ZepCommand_ReplaceRange : public ZepCommand
{
public:
    ZepCommand_ReplaceRange(ZepBuffer& buffer, ReplaceRangeMode mode, const BufferLocation& startOffset, const BufferLocation& endOffset, std::string strReplace, const BufferLocation& cursor = BufferLocation{ -1 }, const BufferLocation& cursorAfter = BufferLocation{ -1 });
    ~ZepCommand_ReplaceRange() override = default;
    ;

    void Redo() override;
    void Undo() override;

    BufferLocation m_startOffset;
    BufferLocation m_endOffset;

    std::string m_strDeleted;
    std::string m_strReplace;
    ReplaceRangeMode m_mode;
};

class ZepCommand_Insert : public ZepCommand
{
public:
    ZepCommand_Insert(ZepBuffer& buffer, const BufferLocation& startOffset, const std::string& str, const BufferLocation& cursor = BufferLocation{ -1 }, const BufferLocation& cursorAfter = BufferLocation{ -1 });
    ~ZepCommand_Insert() override = default;
    ;

    void Redo() override;
    void Undo() override;

    BufferLocation m_startOffset;
    std::string m_strInsert;

    BufferLocation m_endOffsetInserted = -1;
};

} // namespace Zep
