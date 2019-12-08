#pragma once

#include <functional>
#include <set>

#include "zep/editor.hpp"
#include "zep/gap_buffer.hpp"
#include "zep/line_widgets.hpp"
#include "zep/theme.hpp"

#include "zep/mcommon/file/path.hpp"

namespace Zep
{

class ZepSyntax;
class ZepTheme;
class ZepMode;
enum class ThemeColor;

enum class SearchDirection
{
    Forward,
    Backward
};

namespace SearchType
{
enum : uint32_t
{
    WORD = (1 << 0),
    Begin = (1 << 1),
    End = (1 << 2),
    Word = (1 << 3),
    SingleLine = (1 << 4)
};
} // namespace SearchType

namespace FileFlags
{
enum : uint32_t
{
    StrippedCR = (1 << 0),
    TerminatedWithZero = (1 << 1),
    ReadOnly = (1 << 2),
    Locked = (1 << 3), // Can this file path ever be written to?
    Dirty = (1 << 4), // Has the file been changed?
    HasWarnings = (1 << 6),
    HasErrors = (1 << 7),
    DefaultBuffer = (1 << 8) // Default startup buffer
};
} // namespace FileFlags

// Ensure the character is >=0 and <=127 as in the ASCII standard,
// isalnum, for example will assert on debug build if not in this range.
inline auto ToASCII(const char ch) -> int
{
    auto ret = (unsigned int)ch;
    ret = std::min(ret, 127U);
    return ret;
}

enum class BufferType
{
    Normal,
    Search,
    Repl
};

enum class LineLocation
{
    None, // Not any specific location
    LineFirstGraphChar, // First non blank character
    LineLastGraphChar, // Last non blank character
    LineLastNonCR, // Last character before the carriage return
    LineBegin, // Beginning of line
    BeyondLineEnd, // The line end of the buffer line (for wrapped lines).
    LineCRBegin, // The first carriage return character
};

using BufferLocation = int32_t;
struct BufferRange
{
    BufferLocation first{};
    BufferLocation second{};

    BufferRange(BufferLocation a, BufferLocation b)
        : first(a)
        , second(b)
    {
    }

    explicit BufferRange() = default;

    [[nodiscard]] auto ContainsLocation(BufferLocation loc) const -> bool
    {
        return loc >= first && loc < second;
    }
};

namespace RangeMarkerDisplayType
{
enum
{
    Hidden = 0,
    Underline = (1 << 0), // Underline the range
    Background = (1 << 1), // Add a background to the range
    Tooltip = (1 << 2), // Show a tooltip using the name/description
    TooltipAtLine = (1 << 3), // Tooltip shown if the user hovers the line
    CursorTip = (1 << 4), // Tooltip shown if the user cursor is on the Mark
    CursorTipAtLine = (1 << 5), // Tooltip shown if the user cursor is on the Mark line
    Indicator = (1 << 6), // Show an indicator on the left side
    All = Underline | Tooltip | TooltipAtLine | CursorTip | CursorTipAtLine | Indicator | Background
};
} // namespace RangeMarkerDisplayType

enum class ToolTipPos
{
    AboveLine = 0,
    BelowLine = 1,
    RightLine = 2,
    Count = 3
};

namespace RangeMarkerType
{
enum
{
    Message = (1 << 0),
    Search = (1 << 1),
    All = (Message | Search)
};
} // namespace RangeMarkerType

struct RangeMarker
{
    BufferRange range;
    ThemeColor textColor = ThemeColor::Text;
    ThemeColor backgroundColor = ThemeColor::Background;
    ThemeColor highlightColor = ThemeColor::Background;
    uint32_t displayType = RangeMarkerDisplayType::All;
    uint32_t markerType = RangeMarkerType::Message;
    std::string name;
    std::string description;
    ToolTipPos tipPos = ToolTipPos::AboveLine;

    [[nodiscard]] auto ContainsLocation(int32_t loc) const -> bool
    {
        return range.ContainsLocation(loc);
    }
    [[nodiscard]] auto IntersectsRange(const BufferRange& i) const -> bool
    {
        return i.first < range.second && i.second > range.first;
    }
};

struct ZepRepl
{
    std::function<std::string(const std::string&)> fnParser;
    std::function<bool(const std::string&, int&)> fnIsFormComplete;
};

using tRangeMarkers = std::map<int32_t, std::set<std::shared_ptr<RangeMarker>>>;

const int32_t InvalidOffset = -1;

// A really big cursor move; which will likely clamp
static const int32_t MaxCursorMove = int32_t(0xFFFFFFF);

class ZepBuffer : public ZepComponent
{
public:
    ZepBuffer(ZepEditor& editor, std::string strName);
    ZepBuffer(ZepEditor& editor, const ZepPath& path);
    ~ZepBuffer() override;

    void Clear();
    void SetText(const std::string& strText, bool initFromFile = false);
    void Load(const ZepPath& path);
    auto Save(int64_t& size) -> bool;

    auto GetFilePath() const -> ZepPath;
    void SetFilePath(const ZepPath& path);

    static auto Search(const std::string& str, BufferLocation start, SearchDirection dir = SearchDirection::Forward, BufferLocation end = BufferLocation{ -1L }) -> BufferLocation;

    auto GetLinePos(BufferLocation bufferLocation, LineLocation lineLocation) const -> BufferLocation;
    auto GetLineOffsets(int32_t line, int32_t& lineStart, int32_t& lineEnd) const -> bool;
    auto Clamp(BufferLocation in) const -> BufferLocation;
    auto ClampToVisibleLine(BufferLocation in) const -> BufferLocation;
    auto GetBufferColumn(BufferLocation location) const -> int32_t;
    auto InsideBuffer(BufferLocation location) const -> bool;
    using fnMatch = std::function<bool(const char)>;

    static void Move(BufferLocation& loc, SearchDirection dir);
    auto Valid(BufferLocation location) const -> bool;
    auto MotionBegin(BufferLocation& start) const -> bool;
    auto Skip(const fnMatch& IsToken, BufferLocation& start, SearchDirection dir) const -> bool;
    auto SkipOne(const fnMatch& IsToken, BufferLocation& start, SearchDirection dir) const -> bool;
    auto SkipNot(const fnMatch& IsToken, BufferLocation& start, SearchDirection dir) const -> bool;

    auto Find(BufferLocation start, const utf8* pBegin, const utf8* pEnd) const -> BufferLocation;
    auto FindOnLineMotion(BufferLocation start, const utf8* pCh, SearchDirection dir) const -> BufferLocation;
    auto WordMotion(BufferLocation start, uint32_t searchType, SearchDirection dir) const -> BufferLocation;
    auto EndWordMotion(BufferLocation start, uint32_t searchType, SearchDirection dir) const -> BufferLocation;
    auto ChangeWordMotion(BufferLocation start, uint32_t searchType, SearchDirection dir) const -> BufferLocation;
    auto AWordMotion(BufferLocation start, uint32_t searchType) const -> BufferRange;
    auto InnerWordMotion(BufferLocation start, uint32_t searchType) const -> BufferRange;
    auto StandardCtrlMotion(BufferLocation cursor, SearchDirection searchDir) const -> BufferRange;

    auto Delete(const BufferLocation& startOffset, const BufferLocation& endOffset) -> bool;
    auto Insert(const BufferLocation& startOffset, const std::string& str) -> bool;
    auto Replace(const BufferLocation& startOffset, const BufferLocation& endOffset, const std::string& str) -> bool;

    auto GetLineCount() const -> int32_t
    {
        return int32_t(m_lineEnds.size());
    }
    auto GetBufferLine(BufferLocation location) const -> int32_t;
    static auto LocationFromOffset(const BufferLocation& location, int32_t offset) -> BufferLocation;
    static auto LocationFromOffset(int32_t offset) -> BufferLocation;
    auto LocationFromOffsetByChars(const BufferLocation& location, int32_t offset, LineLocation clampLimit = LineLocation::None) const -> BufferLocation;
    auto EndLocation() const -> BufferLocation;

    auto GetText() -> GapBuffer<utf8>&
    {
        return m_gapBuffer;
    }
    auto GetLineEnds() const -> std::vector<int32_t>
    {
        return m_lineEnds;
    }

    auto TestFlags(uint32_t flags) -> bool
    {
        return (m_fileFlags & flags) == flags;
    }

    void ClearFlags(uint32_t flag)
    {
        SetFlags(flag, false);
    }

    void SetFlags(uint32_t flag, bool set = true)
    {
        if (set)
        {
            m_fileFlags |= flag;
        }
        else
        {
            m_fileFlags &= ~flag;
        }
    }
    void SetSyntaxProvider(const SyntaxProvider& provider)
    {
        if (provider.syntaxID != m_syntaxProvider.syntaxID)
        {
            m_spSyntax = provider.factory(this);
            m_syntaxProvider = provider;
        }
    }

    auto GetSyntax() const -> ZepSyntax*
    {
        return m_spSyntax.get();
    }

    auto GetName() const -> const std::string&
    {
        return m_strName;
    }

    auto GetDisplayName() const -> std::string;
    void Notify(std::shared_ptr<ZepMessage> message) override;

    auto GetTheme() const -> ZepTheme&;
    void SetTheme(std::shared_ptr<ZepTheme> spTheme);

    void SetSelection(const BufferRange& sel);
    auto GetSelection() const -> BufferRange;
    auto HasSelection() const -> bool;
    void ClearSelection();

    void AddRangeMarker(const std::shared_ptr<RangeMarker>& spMarker);
    void ClearRangeMarkers(const std::set<std::shared_ptr<RangeMarker>>& markers);
    void ClearRangeMarkers(uint32_t markerType);
    auto GetRangeMarkers(uint32_t markerType) const -> tRangeMarkers;
    void HideMarkers(uint32_t markerType);
    void ShowMarkers(uint32_t markerType, uint32_t displayType);

    void ForEachMarker(uint32_t markerType, SearchDirection dir, BufferLocation begin, BufferLocation end, const std::function<bool(const std::shared_ptr<RangeMarker>&)>& fnCB) const;
    auto FindNextMarker(BufferLocation start, SearchDirection dir, uint32_t markerType) -> std::shared_ptr<RangeMarker>;

    void SetBufferType(BufferType type);
    auto GetBufferType() const -> BufferType;

    void SetLastEditLocation(BufferLocation loc);
    auto GetLastEditLocation() const -> BufferLocation;

    auto GetMode() const -> ZepMode*;
    void SetMode(const std::shared_ptr<ZepMode>& spMode);

    void SetReplProvider(ZepRepl* repl)
    {
        m_replProvider = repl;
    }
    auto GetReplProvider() const -> ZepRepl*
    {
        return m_replProvider;
    }

    using tLineWidgets = std::vector<std::shared_ptr<ILineWidget>>;
    void AddLineWidget(int32_t line, const std::shared_ptr<ILineWidget>& spWidget);
    void ClearLineWidgets(int32_t line);
    auto GetLineWidgets(int32_t line) const -> const tLineWidgets*;

    auto GetLastUpdateTime() const -> uint64_t
    {
        return m_lastUpdateTime;
    }
    auto GetUpdateCount() const -> uint64_t
    {
        return m_updateCount;
    }

    auto IsHidden() const -> bool;

private:
    // Internal
    auto SearchWord(uint32_t searchType, GapBuffer<utf8>::const_iterator itrBegin, GapBuffer<utf8>::const_iterator itrEnd, SearchDirection dir) const -> GapBuffer<utf8>::const_iterator;
    void ClearRangeMarker(const std::shared_ptr<RangeMarker>& spMarker);

    void MarkUpdate();

    void UpdateForInsert(const BufferLocation& startOffset, const BufferLocation& endOffset);
    void UpdateForDelete(const BufferLocation& startOffset, const BufferLocation& endOffset);

private:
    bool m_dirty = false; // Is the text modified?
    GapBuffer<utf8> m_gapBuffer; // Storage for the text - a gap buffer for efficiency
    std::vector<int32_t> m_lineEnds; // End of each line
    uint32_t m_fileFlags = 0;
    BufferType m_bufferType = BufferType::Normal;
    std::shared_ptr<ZepSyntax> m_spSyntax;
    std::string m_strName;
    ZepPath m_filePath;
    std::shared_ptr<ZepTheme> m_spOverrideTheme;
    std::map<BufferLocation, std::vector<std::shared_ptr<ILineWidget>>> m_lineWidgets;

    BufferRange m_selection;
    tRangeMarkers m_rangeMarkers;
    BufferLocation m_lastEditLocation{ 0 };
    std::shared_ptr<ZepMode> m_spMode;
    ZepRepl* m_replProvider = nullptr; // May not be set
    SyntaxProvider m_syntaxProvider;
    uint64_t m_updateCount = 0;
    uint64_t m_lastUpdateTime = 0;
};

// Notification payload
enum class BufferMessageType
{
    // Inform clients that we are about to mess with the buffer
    PreBufferChange = 0,
    TextChanged,
    TextDeleted,
    TextAdded,
    Loaded,
    MarkersChanged
};

struct BufferMessage : public ZepMessage
{
    BufferMessage(ZepBuffer* pBuff, BufferMessageType messageType, const BufferLocation& startLoc, const BufferLocation& endLoc)
        : ZepMessage(Msg::Buffer)
        , pBuffer(pBuff)
        , type(messageType)
        , startLocation(startLoc)
        , endLocation(endLoc)
    {
    }

    ZepBuffer* pBuffer;
    BufferMessageType type;
    BufferLocation startLocation;
    BufferLocation endLocation;
};

} // namespace Zep
