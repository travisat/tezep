#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <regex>
#include <utility>

#include "zep/buffer.hpp"
#include "zep/editor.hpp"
#include "zep/filesystem.hpp"

#include "zep/mcommon/file/path.hpp"
#include "zep/mcommon/string/stringutils.hpp"

#include "zep/mcommon/logger.hpp"

namespace Zep
{

namespace
{

// A VIM-like definition of a word.  Actually, in Vim this can be changed, but this editor
// assumes a word is alphanumeric or underscore for consistency
inline auto IsWordChar(const char c) -> bool
{
    auto ch = ToASCII(c);
    return (std::isalnum(ch) != 0) || ch == '_';
}
inline auto IsWordOrSepChar(const char c) -> bool
{
    auto ch = ToASCII(c);
    return (std::isalnum(ch) != 0) || ch == '_' || ch == ' ' || ch == '\n' || ch == 0;
}
inline auto IsWORDChar(const char c) -> bool
{
    auto ch = ToASCII(c);
    return std::isgraph(ch) != 0;
}
inline auto IsWORDOrSepChar(const char c) -> bool
{
    auto ch = ToASCII(c);
    return (std::isgraph(ch) != 0) || ch == ' ' || ch == '\n' || ch == 0;
}
inline auto IsSpace(const char c) -> bool
{
    auto ch = ToASCII(c);
    return ch == ' ';
}
inline auto IsSpaceOrNewline(const char c) -> bool
{
    auto ch = ToASCII(c);
    return ch == ' ' || ch == '\n';
}
inline auto IsSpaceOrTerminal(const char c) -> bool
{
    auto ch = ToASCII(c);
    return ch == ' ' || ch == 0 || ch == '\n';
}
/*
inline auto IsNewlineOrEnd(const char c) -> bool
{
    auto ch = ToASCII(c);
    return ch == '\n' || ch == 0;
}*/

using fnMatch = std::function<bool>(const char);

} // namespace
ZepBuffer::ZepBuffer(ZepEditor& editor, std::string strName)
    : ZepComponent(editor)
    , m_strName(std::move(strName))
{
    Clear();
}

ZepBuffer::ZepBuffer(ZepEditor& editor, const ZepPath& path)
    : ZepComponent(editor)
{
    Load(path);
}

ZepBuffer::~ZepBuffer() = default;

void ZepBuffer::Notify(std::shared_ptr<ZepMessage> message)
{
}

auto ZepBuffer::GetBufferColumn(BufferLocation location) const -> int32_t
{
    auto lineStart = GetLinePos(location, LineLocation::LineBegin);
    return location - lineStart;
}

auto ZepBuffer::GetBufferLine(BufferLocation location) const -> int32_t
{
    auto itrLine = std::lower_bound(m_lineEnds.begin(), m_lineEnds.end(), location);
    if (itrLine != m_lineEnds.end() && location >= *itrLine)
    {
        itrLine++;
    }
    int32_t line = int32_t(itrLine - m_lineEnds.begin());
    line = std::min(std::max(0, line), int32_t(m_lineEnds.size() - 1));
    return line;
}

auto ZepBuffer::LocationFromOffsetByChars(const BufferLocation& location, int32_t offset, LineLocation clampLimit) const -> BufferLocation
{
    // Walk and find.
    int32_t dir = offset > 0 ? 1 : -1;

    auto clampLocation = GetLinePos(location, clampLimit);

    // TODO(unknown): This can be cleaner(?)
    int32_t current = location;
    for (int32_t i = 0; i < std::abs(offset); i++)
    {
        // If walking back, move back before looking at char
        if (dir == -1)
        {
            current += dir;
        }

        if (current >= m_gapBuffer.size())
        {
            break;
        }

        current = std::max(0, current);

        if (m_gapBuffer[current] == '\n')
        {
            if ((current + dir) >= m_gapBuffer.size())
            {
                break;
            }
        }

        // If walking forward, post append
        if (dir == 1)
        {
            current += dir;
        }
    }

    if (clampLocation != InvalidOffset)
    {
        current = std::min(clampLocation, current);
    }
    return LocationFromOffset(current);
}

auto ZepBuffer::LocationFromOffset(const BufferLocation& location, int32_t offset) -> BufferLocation
{
    return LocationFromOffset(location + offset);
}

auto ZepBuffer::LocationFromOffset(int32_t offset) -> BufferLocation
{
    return BufferLocation{ offset };
}

auto ZepBuffer::Search(const std::string& str, BufferLocation start, SearchDirection dir, BufferLocation end) -> BufferLocation
{
    (void)end;
    (void)dir;
    (void)start;
    (void)str;
    return BufferLocation{ 0 };
}

auto ZepBuffer::Valid(BufferLocation location) const -> bool
{
    return !(location < 0 || location >= (BufferLocation)m_gapBuffer.size());
}

// Prepare for a motion
auto ZepBuffer::MotionBegin(BufferLocation& start) const -> bool
{
    BufferLocation newStart = start;

    // Clamp to sensible, begin
    newStart = std::min(newStart, BufferLocation(m_gapBuffer.size() - 1));
    newStart = std::max(0, newStart);

    bool change = newStart != start;
    if (change)
    {
        start = newStart;
        return true;
    }
    return start != 0;
}

void ZepBuffer::Move(BufferLocation& loc, SearchDirection dir)
{
    if (dir == SearchDirection::Backward)
    {
        loc--;
    }
    else
    {
        loc++;
    }
}

auto ZepBuffer::Skip(const fnMatch& IsToken, BufferLocation& start, SearchDirection dir) const -> bool
{
    if (!Valid(start))
    {
        return false;
    }

    bool moved = false;
    while (Valid(start) && IsToken(m_gapBuffer[start]))
    {
        Move(start, dir);
        moved = true;
    }
    return moved;
}

auto ZepBuffer::SkipOne(const fnMatch& IsToken, BufferLocation& start, SearchDirection dir) const -> bool
{
    if (!Valid(start))
    {
        return false;
    }

    bool moved = false;
    if (Valid(start) && IsToken(m_gapBuffer[start]))
    {
        Move(start, dir);
        moved = true;
    }
    return moved;
}

auto ZepBuffer::SkipNot(const fnMatch& IsToken, BufferLocation& start, SearchDirection dir) const -> bool
{
    if (!Valid(start))
    {
        return false;
    }

    bool moved = false;
    while (Valid(start) && !IsToken(m_gapBuffer[start]))
    {
        Move(start, dir);
        moved = true;
    }
    return moved;
}

// This is the vim-like 'caw' rule; The motions and behaviour are based on how vim behaves.
// This is still quite complex behavior for this particular motion.  I'm open to better ways to express it!
auto ZepBuffer::AWordMotion(BufferLocation start, uint32_t searchType) const -> BufferRange
{
    auto IsWord = searchType == SearchType::Word ? IsWordChar : IsWORDChar;

    BufferRange r;
    r.first = start;

    MotionBegin(start);

    // Already on a word; find the limits, and include the space
    if (Skip(IsWord, start, SearchDirection::Backward))
    {
        start += 1;
        r.first = start;
        Skip(IsWord, start, SearchDirection::Forward);
        Skip(IsSpace, start, SearchDirection::Forward);
        r.second = start;
    }
    // ... or skip space
    else if (Skip(IsSpace, start, SearchDirection::Forward))
    {
        Skip(IsWord, start, SearchDirection::Forward);
        r.second = start;
    }
    // On a non-word, find the beginning, remove including following spaces
    else if (SkipNot(IsWord, start, SearchDirection::Backward))
    {
        Skip(IsSpace, start, SearchDirection::Forward);
        start += 1;
        r.first = start;
        SkipNot(IsWord, start, SearchDirection::Forward);
        Skip(IsSpace, start, SearchDirection::Forward);
        r.second = start;
    }

    return r;
}

// Implements the ctrl + motion of a standard editor.
// This is a little convoluted; perhaps the logic can be simpler!
// Playing around with CTRL+ arrows and shift in an app like notepad will teach you that the rules for how far to jump
// depend on what you are over, and which direction you are going.....
// The unit tests are designed to enforce the behavior here
auto ZepBuffer::StandardCtrlMotion(BufferLocation cursor, SearchDirection searchDir) const -> BufferRange
{
    MotionBegin(cursor);

    auto lineEnd = GetLinePos(cursor, LineLocation::LineLastNonCR);
    auto current = std::min(lineEnd, Clamp(cursor));

    BufferRange r;
    r.first = current;
    r.second = current;

    if (searchDir == SearchDirection::Forward)
    {
        // Skip space
        Skip(IsSpaceOrTerminal, current, searchDir);
        if (Skip(IsWORDChar, current, searchDir))
        {
            Skip(IsSpace, current, searchDir);
        }
    }
    else
    {
        // If on the first char of a new word, skip back
        if (current > 0 && IsWORDChar(m_gapBuffer[current]) && !IsWORDChar(m_gapBuffer[current - 1]))
        {
            current--;
        }

        // Skip a space
        Skip(IsSpaceOrTerminal, current, searchDir);

        // Back to the beginning of the next word
        if (Skip(IsWORDChar, current, searchDir))
        {
            current++;
        }
    }
    r.second = Clamp(current);

    return r;
}

auto ZepBuffer::InnerWordMotion(BufferLocation start, uint32_t searchType) const -> BufferRange
{
    auto IsWordOrSpace = searchType == SearchType::Word ? IsWordOrSepChar : IsWORDOrSepChar;
    auto IsWord = searchType == SearchType::Word ? IsWordChar : IsWORDChar;
    MotionBegin(start);

    BufferRange r;

    if (SkipNot(IsWordOrSpace, start, SearchDirection::Forward))
    {
        r.second = start;
        start--;
        SkipNot(IsWordOrSpace, start, SearchDirection::Backward);
        r.first = start + 1;
    }
    else if (Skip(IsSpace, start, SearchDirection::Forward))
    {
        r.second = start;
        start--;
        Skip(IsSpace, start, SearchDirection::Backward);
        r.first = start + 1;
    }
    else
    {
        Skip(IsWord, start, SearchDirection::Forward);
        r.second = start;
        start--;
        Skip(IsWord, start, SearchDirection::Backward);
        r.first = start + 1;
    }
    return r;
}

auto ZepBuffer::Find(BufferLocation start, const utf8* pBegin, const utf8* pEnd) const -> BufferLocation
{
    if (start > EndLocation())
    {
        return InvalidOffset;
    }

    if (pEnd == nullptr)
    {
        pEnd = pBegin;
        while (*pEnd != 0)
        {
            pEnd++;
        }
    }

    auto itrBuffer = m_gapBuffer.begin() + start;
    auto itrEnd = m_gapBuffer.end();
    while (itrBuffer != itrEnd)
    {
        auto itrNext = itrBuffer;

        // Loop the string
        auto pCurrent = pBegin;
        while (pCurrent != pEnd && itrNext != itrEnd)
        {
            if (*pCurrent != *itrNext)
            {
                break;
            }
            pCurrent++;
            itrNext++;
        };

        // We sucesfully got to the end
        if (pCurrent == pEnd)
        {
            return (BufferLocation)(itrBuffer - m_gapBuffer.begin());
        }

        itrBuffer++;
    };

    return InvalidOffset;
}

auto ZepBuffer::FindOnLineMotion(BufferLocation start, const utf8* pCh, SearchDirection dir) const -> BufferLocation
{
    auto entry = start;
    auto IsMatch = [pCh](const char ch) {
        return *pCh == ch;
    };
    auto NotMatchNotEnd = [pCh](const char ch) {
        return *pCh != ch && ch != '\n';
    };

    SkipOne(IsMatch, start, dir);
    Skip(NotMatchNotEnd, start, dir);

    if (Valid(start) && *pCh == m_gapBuffer[start])
    {
        return start;
    }
    return entry;
}

auto ZepBuffer::WordMotion(BufferLocation start, uint32_t searchType, SearchDirection dir) const -> BufferLocation
{
    auto IsWord = searchType == SearchType::Word ? IsWordChar : IsWORDChar;

    MotionBegin(start);

    if (dir == SearchDirection::Forward)
    {
        if (Skip(IsWord, start, dir))
        {
            // Skipped a word, skip spaces then done
            Skip(IsSpaceOrTerminal, start, dir);
        }
        else
        {
            SkipNot(IsWord, start, dir);
        }
    }
    else // Backward
    {
        auto startSearch = start;

        // Jump back to the beginning of a word if on it
        if (Skip(IsWord, start, dir))
        {
            // If we weren't already on the first char of the word, then we have gone back a word!
            if (startSearch != (start + 1))
            {
                SkipNot(IsWord, start, SearchDirection::Forward);
                return start;
            }
        }
        else
        {
            SkipNot(IsWord, start, dir);
        }

        // Skip any spaces
        Skip(IsSpace, start, dir);

        // Go back to the beginning of the word
        if (Skip(IsWord, start, dir))
        {
            SkipNot(IsWord, start, SearchDirection::Forward);
        }
    }
    return start;
}

auto ZepBuffer::EndWordMotion(BufferLocation start, uint32_t searchType, SearchDirection dir) const -> BufferLocation
{
    auto IsWord = searchType == SearchType::Word ? IsWordChar : IsWORDChar;

    MotionBegin(start);

    if (dir == SearchDirection::Forward)
    {
        auto startSearch = start;

        // Skip to the end
        if (Skip(IsWord, start, dir))
        {
            // We moved a bit, so we found the end of the current word
            if (startSearch != start - 1)
            {
                SkipNot(IsWord, start, SearchDirection::Backward);
                return start;
            }
        }
        else
        {
            SkipNot(IsWord, start, dir);
        }

        // Skip any spaces
        Skip(IsSpaceOrNewline, start, dir);

        // Go back to the beginning of the word
        if (Skip(IsWord, start, dir))
        {
            SkipNot(IsWord, start, SearchDirection::Backward);
        }
    }
    else // Backward
    {
        // Note this is the same as the Next word code, in 'forward' mode
        if (Skip(IsWord, start, dir))
        {
            // Skipped a word, skip spaces then done
            Skip(IsSpace, start, dir);
        }
        else
        {
            SkipNot(IsWord, start, dir);
        }
    }
    return start;
}

auto ZepBuffer::ChangeWordMotion(BufferLocation start, uint32_t searchType, SearchDirection dir) const -> BufferLocation
{
    // Change word is different to work skipping; it will change a string of spaces, for example.
    // Essentially it changes 'what you are over', based on the word rule
    auto IsWord = searchType == SearchType::Word ? IsWordChar : IsWORDChar;
    MotionBegin(start);
    if (Skip(IsWord, start, dir))
    {
        return start;
    }
    SkipNot(IsWord, start, dir);
    return start;
}

auto ZepBuffer::InsideBuffer(BufferLocation loc) const -> bool
{
    return loc >= 0 && loc < BufferLocation(m_gapBuffer.size());
}

auto ZepBuffer::Clamp(BufferLocation in) const -> BufferLocation
{
    in = std::min(in, BufferLocation(m_gapBuffer.size() - 1));
    in = std::max(in, BufferLocation(0));
    return in;
}

auto ZepBuffer::ClampToVisibleLine(BufferLocation in) const -> BufferLocation
{
    in = Clamp(in);
    auto loc = GetLinePos(in, LineLocation::LineLastNonCR);
    in = std::min(loc, in);
    return in;
}

// Method for querying the beginning and end of a line
auto ZepBuffer::GetLineOffsets(const int32_t line, int32_t& lineStart, int32_t& lineEnd) const -> bool
{
    // Not valid
    if (m_lineEnds.size() <= line)
    {
        lineStart = 0;
        lineEnd = 0;
        return false;
    }

    // Find the line bounds - we know the end, find the start from the previous
    lineEnd = m_lineEnds[line];
    lineStart = line == 0 ? 0 : m_lineEnds[line - 1];
    return true;
}

// Basic load suppot; read a file if it's present, but keep
// the file path in case you want to write later
void ZepBuffer::Load(const ZepPath& path)
{
    // Set the name from the path
    if (path.has_filename())
    {
        m_strName = path.filename().string();
    }
    else
    {
        m_strName = m_filePath.string();
    }

    // Must set the syntax before the first buffer change messages
    GetEditor().SetBufferSyntax(*this);

    if (GetEditor().GetFileSystem().Exists(path))
    {
        m_filePath = GetEditor().GetFileSystem().Canonical(path);
        auto read = GetEditor().GetFileSystem().Read(path);
        if (!read.empty())
        {
            SetText(read, true);
        }
    }
    else
    {
        // Can't canonicalize a non-existent path.
        // But we may have a path we haven't save to yet!
        Clear();
        m_filePath = path;
    }
}

auto ZepBuffer::Save(int64_t& size) -> bool
{
    if (TestFlags(FileFlags::Locked))
    {
        return false;
    }

    if (TestFlags(FileFlags::ReadOnly))
    {
        return false;
    }

    auto str = GetText().string();

    // Put back /r/n if necessary while writing the file
    // At the moment, Zep removes /r/n and just uses /n while modifying text.
    // It replaces the /r on files that had it afterwards
    // Alternatively we could manage them 'in place', but that would make parsing more complex.
    // And then what do you do if there are 2 different styles in the file.
    if ((m_fileFlags & FileFlags::StrippedCR) != 0)
    {
        // TODO(unknown): faster way to replace newlines
        string_replace_in_place(str, "\n", "\r\n");
    }

    // Remove the appended 0 if necessary
    size = (int64_t)str.size();
    if ((m_fileFlags & FileFlags::TerminatedWithZero) != 0)
    {
        size--;
    }

    if (size <= 0)
    {
        return true;
    }

    if (GetEditor().GetFileSystem().Write(m_filePath, &str[0], (size_t)size))
    {
        ClearFlags(FileFlags::Dirty);
        return true;
    }
    return false;
}

auto ZepBuffer::GetDisplayName() const -> std::string
{
    if (m_filePath.empty())
    {
        return m_strName;
    }
    return m_filePath.string();
}

auto ZepBuffer::GetFilePath() const -> ZepPath
{
    return m_filePath;
}

void ZepBuffer::SetFilePath(const ZepPath& path)
{
    auto testPath = path;
    if (GetEditor().GetFileSystem().Exists(testPath))
    {
        testPath = GetEditor().GetFileSystem().Canonical(testPath);
    }

    if (!GetEditor().GetFileSystem().Equivalent(testPath, m_filePath))
    {
        m_filePath = testPath;
    }
    GetEditor().SetBufferSyntax(*this);
}

// Remember that we updated the buffer and dirty the state
// Clients can use these values to figure out update times and dirty state
void ZepBuffer::MarkUpdate()
{
    m_updateCount++;
    m_lastUpdateTime = timer_get_time_now();

    SetFlags(FileFlags::Dirty);
}

// Clear this buffer.  If it was previously not clear, it has been updated.
// Otherwise it is just reset to default state.  A new buffer is always initially cleared.
void ZepBuffer::Clear()
{
    bool changed = false;
    if (m_gapBuffer.size() > 1)
    {
        // Inform clients we are about to change the buffer
        GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::PreBufferChange, 0, BufferLocation(m_gapBuffer.size() - 1)));
        changed = true;
    }

    m_gapBuffer.clear();
    m_gapBuffer.push_back(0);
    m_lineEnds.clear();
    SetFlags(FileFlags::TerminatedWithZero);

    m_lineEnds.push_back(m_gapBuffer.size());

    if (changed)
    {
        MarkUpdate();
        GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::TextDeleted, 0, BufferLocation(m_gapBuffer.size() - 1)));
    }
}

// Replace the buffer buffer with the text
void ZepBuffer::SetText(const std::string& text, bool initFromFile)
{
    // First, clear it
    Clear();

    if (!text.empty())
    {
        // Since incremental insertion of a big file into a gap buffer gives us worst case performance,
        // We build the buffer in a seperate array and assign it.  Much faster.
        // This is because we remove \r and convert tabs. Tabs are considered 'always evil' and should be
        // 4 spaces.  Take it up with your local code police if you feel aggrieved.
        std::vector<utf8> input;

        m_lineEnds.clear();

        // Update the gap buffer with the text
        // We remove \r, we only care about \n
        for (auto& ch : text)
        {
            if (ch == '\r')
            {
                m_fileFlags |= FileFlags::StrippedCR;
            }
            else if (ch == '\t')
            {
                input.push_back(' ');
                input.push_back(' ');
                input.push_back(' ');
                input.push_back(' ');
            }
            else
            {
                input.push_back(ch);
                if (ch == '\n')
                {
                    m_lineEnds.push_back(input.size());
                }
            }
        }
        m_gapBuffer.assign(input.begin(), input.end());
    }

    if (m_gapBuffer[m_gapBuffer.size() - 1] != 0)
    {
        m_fileFlags |= FileFlags::TerminatedWithZero;
        m_gapBuffer.push_back(0);
    }

    // TODO(unknown): Why is a line end needed always?
    m_lineEnds.push_back(m_gapBuffer.size());

    MarkUpdate();

    // When loading a file, send the Loaded message to distinguish it from adding to a buffer, and remember that the buffer is not dirty in this case
    if (initFromFile)
    {
        GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::Loaded, BufferLocation{ 0 }, BufferLocation{ static_cast<BufferLocation>(m_gapBuffer.size()) }));

        // Doc is not dirty
        ClearFlags(FileFlags::Dirty);
    }
    else
    {
        GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::TextAdded, BufferLocation{ 0 }, BufferLocation{ static_cast<BufferLocation>(m_gapBuffer.size()) }));
    }
}

// TODO(unknown): This can be cleaner
// The function needs to find the point on the line which bufferLocation is on.
// It needs to account for empty lines or the last line, zero terminated.
// It shouldn't walk away to another line!
auto ZepBuffer::GetLinePos(BufferLocation bufferLocation, LineLocation lineLocation) const -> BufferLocation
{
    if (lineLocation == LineLocation::None)
    {
        return InvalidOffset;
    }

    bufferLocation = Clamp(bufferLocation);
    if (m_gapBuffer.empty())
    {
        return bufferLocation;
    }

    // If we are on the CR, move back 1, unless the \n is all that is on the line
    if (m_gapBuffer[bufferLocation] == '\n')
    {
        bufferLocation--;
    }

    // Find the end of the previous line
    while (bufferLocation >= 0 && m_gapBuffer[bufferLocation] != '\n')
    {
        bufferLocation--;
    }

    // Step back to the start of the line
    bufferLocation++;

    switch (lineLocation)
    {
    default:
    case LineLocation::LineBegin: {
        return Clamp(bufferLocation);
    }
    break;

    // The point just after the line end
    case LineLocation::BeyondLineEnd: {
        while (bufferLocation < m_gapBuffer.size() && m_gapBuffer[bufferLocation] != '\n' && m_gapBuffer[bufferLocation] != 0)
        {
            bufferLocation++;
        }
        bufferLocation++;
        return Clamp(bufferLocation);
    }
    break;

    case LineLocation::LineCRBegin: {
        while (bufferLocation < m_gapBuffer.size()
            && m_gapBuffer[bufferLocation] != '\n'
            && m_gapBuffer[bufferLocation] != 0)
        {
            bufferLocation++;
        }
        return bufferLocation;
    }
    break;

    case LineLocation::LineFirstGraphChar: {
        while (bufferLocation < m_gapBuffer.size() && (std::isgraph(ToASCII(m_gapBuffer[bufferLocation])) == 0) && m_gapBuffer[bufferLocation] != '\n')
        {
            bufferLocation++;
        }
        return Clamp(bufferLocation);
    }
    break;

    case LineLocation::LineLastNonCR: {
        auto start = bufferLocation;

        while (bufferLocation < m_gapBuffer.size()
            && m_gapBuffer[bufferLocation] != '\n'
            && m_gapBuffer[bufferLocation] != 0)
        {
            bufferLocation++;
        }

        if (start != bufferLocation)
        {
            bufferLocation--;
        }

        return Clamp(bufferLocation);
    }
    break;

    case LineLocation::LineLastGraphChar: {
        while (bufferLocation < m_gapBuffer.size()
            && m_gapBuffer[bufferLocation] != '\n'
            && m_gapBuffer[bufferLocation] != 0)
        {
            bufferLocation++;
        }

        while (bufferLocation > 0 && bufferLocation < m_gapBuffer.size() && (std::isgraph(ToASCII(m_gapBuffer[bufferLocation])) == 0))
        {
            bufferLocation--;
        }
        return Clamp(bufferLocation);
    }
    break;
    }
}

void ZepBuffer::UpdateForDelete(const BufferLocation& startOffset, const BufferLocation& endOffset)
{
    auto distance = endOffset - startOffset;
    ForEachMarker(RangeMarkerType::All, Zep::SearchDirection::Forward, startOffset, EndLocation(), [&](const std::shared_ptr<RangeMarker>& marker) {
        if (startOffset >= marker->range.second)
        {
            return true;
        }
        if (endOffset <= marker->range.first)
        {
            marker->range.first -= distance;
            marker->range.second -= distance;
        }
        else
        {
            auto overlapStart = std::max(startOffset, marker->range.first);
            auto overlapEnd = std::min(endOffset, marker->range.second);
            auto dist = overlapEnd - overlapStart;
            marker->range.second -= dist;
        }
        return true;
    });

    if (!m_lineWidgets.empty())
    {
        std::map<int32_t, int32_t> lineMoves;
        auto itr = m_lineWidgets.begin();
        while (itr != m_lineWidgets.end())
        {
            auto pWidget = itr->second;
            if (startOffset >= itr->first)
            {
                // Nothing to do, the widgets are behind the area removed
                break;
            }
            if (startOffset < itr->first)
            {
                // Removed before, jump back by this many
                lineMoves[itr->first] = itr->first - distance;
            }
            else
            {
                auto overlapStart = std::max(startOffset, itr->first);
                auto overlapEnd = std::min(endOffset, itr->first);
                auto dist = overlapEnd - overlapStart;
                lineMoves[itr->first] = itr->first - dist;
            }
            itr++;
        }

        for (auto& replace : lineMoves)
        {
            auto pWidgets = m_lineWidgets[replace.first];
            m_lineWidgets.erase(replace.first);

            if (replace.second >= 0 && replace.second < (m_gapBuffer.size() - 1))
            {
                m_lineWidgets[replace.second] = pWidgets;
            }
        }
    }
}

void ZepBuffer::UpdateForInsert(const BufferLocation& startOffset, const BufferLocation& endOffset)
{
    // Move the markers after the insert point forwards, or
    // expand the marker range if inserting inside it (that's a guess!)
    auto distance = endOffset - startOffset;

    ForEachMarker(RangeMarkerType::All, SearchDirection::Forward, startOffset, EndLocation(), [&](const std::shared_ptr<RangeMarker>& marker) {
        if (marker->range.second <= startOffset)
        {
            return true;
        }

        if (marker->range.first >= startOffset)
        {
            marker->range.first += distance;
            marker->range.second += distance;
        }
        return true;
    });

    if (!m_lineWidgets.empty())
    {
        std::map<int32_t, int32_t> lineMoves;
        auto itr = m_lineWidgets.begin();
        while (itr != m_lineWidgets.end())
        {
            auto pWidget = itr->second;
            if (startOffset > itr->first)
            {
                // Nothing to do, the widgets are behind the area inserted
                break;
            }
            if (startOffset <= itr->first)
            {
                // Add
                lineMoves[itr->first] = itr->first + distance;
            }
            itr++;
        }

        for (auto& replace : lineMoves)
        {
            auto pWidgets = m_lineWidgets[replace.first];
            m_lineWidgets.erase(replace.first);
            if (replace.second >= 0 && replace.second < (m_gapBuffer.size() - 1))
            {
                m_lineWidgets[replace.second] = pWidgets;
            }
        }
    }
}

auto ZepBuffer::Insert(const BufferLocation& startOffset, const std::string& str) -> bool
{
    if (startOffset > m_gapBuffer.size())
    {
        return false;
    }

    BufferLocation changeRange{ static_cast<BufferLocation>(str.length()) };

    // We are about to modify this range
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::PreBufferChange, startOffset, startOffset + changeRange));

    UpdateForInsert(startOffset, startOffset + changeRange);

    // abcdef\r\nabc<insert>dfdf\r\n
    auto itrLine = std::lower_bound(m_lineEnds.begin(), m_lineEnds.end(), startOffset);
    ;
    if (itrLine != m_lineEnds.end() && *itrLine <= startOffset)
    {
        itrLine++;
    }

    auto itrEnd = str.end();
    auto itrBegin = str.begin();
    auto itr = str.begin();

    // Make a list of lines to 'insert'
    std::vector<int32_t> lines;
    std::string lineEndSymbols("\n");
    while (itr != itrEnd)
    {
        // Get to first point after "\n"
        // That's the point just after the end of the current line
        itr = std::find_first_of(itr, itrEnd, lineEndSymbols.begin(), lineEndSymbols.end());
        if (itr != itrEnd)
        {
            if (itr != itrEnd && *itr == '\n')
            {
                itr++;
            }
            lines.push_back((itr - itrBegin) + startOffset);
        }
    }

    // Increment the rest of the line ends
    // We make all the remaning line ends bigger by the fixed_size of the insertion
    auto itrAdd = itrLine;
    while (itrAdd != m_lineEnds.end())
    {
        *itrAdd += str.length();
        itrAdd++;
    }

    if (!lines.empty())
    {
        // Update the atomic line counter so clients can see where we are up to.
        m_lineEnds.insert(itrLine, lines.begin(), lines.end());
    }

    m_gapBuffer.insert(m_gapBuffer.begin() + startOffset, str.begin(), str.end());

    MarkUpdate();

    // This is the range we added (not valid any more in the buffer)
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::TextAdded, startOffset, startOffset + changeRange));

    return true;
}

auto ZepBuffer::Replace(const BufferLocation& startOffset, const BufferLocation& endOffset, const std::string& str) -> bool
{
    if (startOffset > m_gapBuffer.size() || endOffset > m_gapBuffer.size())
    {
        return false;
    }

    // We are about to modify this range
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::PreBufferChange, startOffset, endOffset));

    // Perform a straight replace
    for (auto loc = startOffset; loc < endOffset; loc++)
    {
        // Note we don't support utf8 yet
        m_gapBuffer[loc] = str[0];
    }

    MarkUpdate();

    // This is the range we added (not valid any more in the buffer)
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::TextChanged, startOffset, endOffset));

    return true;
}
// A fundamental operation - delete a range of characters
// Need to update:
// - m_lineEnds
// - m_processedLine
// - m_pBuffer (i.e remove chars)
// We also need to inform clients before we change the buffer, and after we delete text with the range we removed.
// This helps them to fix up their data structures without rebuilding.
// Assumption: The buffer always is at least a single line/character of '0', representing file end.
// This makes a few things fall out more easily
auto ZepBuffer::Delete(const BufferLocation& startOffset, const BufferLocation& endOffset) -> bool
{
    assert(startOffset >= 0 && endOffset <= (BufferLocation)(m_gapBuffer.size() - 1));

    // We are about to modify this range
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::PreBufferChange, startOffset, endOffset));

    UpdateForDelete(startOffset, endOffset);

    auto itrLine = std::lower_bound(m_lineEnds.begin(), m_lineEnds.end(), startOffset);
    if (itrLine == m_lineEnds.end())
    {
        return false;
    }

    auto itrLastLine = std::upper_bound(itrLine, m_lineEnds.end(), endOffset);
    auto offsetDiff = endOffset - startOffset;

    if (*itrLine <= startOffset)
    {
        itrLine++;
    }

    // Adjust all line offsets beyond us
    for (auto itr = itrLastLine; itr != m_lineEnds.end(); itr++)
    {
        *itr -= offsetDiff;
    }

    if (itrLine != itrLastLine)
    {
        m_lineEnds.erase(itrLine, itrLastLine);
    }

    m_gapBuffer.erase(m_gapBuffer.begin() + startOffset, m_gapBuffer.begin() + endOffset);
    assert(m_gapBuffer.size() > 0 && m_gapBuffer[m_gapBuffer.size() - 1] == 0);

    MarkUpdate();

    // This is the range we deleted (not valid any more in the buffer)
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::TextDeleted, startOffset, endOffset));

    return true;
}

auto ZepBuffer::EndLocation() const -> BufferLocation
{
    // TODO(unknown): This isn't safe? What if the buffer is empty
    // I've clamped it for now
    auto end = std::max((BufferLocation)0, (BufferLocation)m_gapBuffer.size() - 1);
    return LocationFromOffset(end);
}

auto ZepBuffer::GetTheme() const -> ZepTheme&
{
    if (m_spOverrideTheme)
    {
        return *m_spOverrideTheme;
    }
    return GetEditor().GetTheme();
}

void ZepBuffer::SetTheme(std::shared_ptr<ZepTheme> spTheme)
{
    m_spOverrideTheme = std::move(spTheme);
}

auto ZepBuffer::HasSelection() const -> bool
{
    return m_selection.first != m_selection.second;
}

void ZepBuffer::ClearSelection()
{
    m_selection.first = m_selection.second = 0;
}

auto ZepBuffer::GetSelection() const -> BufferRange
{
    return m_selection;
}

void ZepBuffer::SetSelection(const BufferRange& selection)
{
    m_selection = selection;
    if (m_selection.first > m_selection.second)
    {
        std::swap(m_selection.first, m_selection.second);
    }
}

void ZepBuffer::AddRangeMarker(const std::shared_ptr<RangeMarker>& spMarker)
{
    m_rangeMarkers[spMarker->range.first].insert(spMarker);
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::MarkersChanged, 0, BufferLocation(m_gapBuffer.size() - 1)));
}

void ZepBuffer::ClearRangeMarker(const std::shared_ptr<RangeMarker>& spMarker)
{
    std::set<BufferLocation> emptyLocations;
    for (auto& markerPair : m_rangeMarkers)
    {
        markerPair.second.erase(spMarker);
        if (markerPair.second.empty())
        {
            emptyLocations.insert(markerPair.first);
        }
    }

    for (auto& victim : emptyLocations)
    {
        m_rangeMarkers.erase(victim);
    }
}

void ZepBuffer::ClearRangeMarkers(const std::set<std::shared_ptr<RangeMarker>>& markers)
{
    for (auto& marker : markers)
    {
        ClearRangeMarker(marker);
    }
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::MarkersChanged, 0, BufferLocation(m_gapBuffer.size() - 1)));
}

void ZepBuffer::ClearRangeMarkers(uint32_t markerType)
{
    std::set<std::shared_ptr<RangeMarker>> markers;
    ForEachMarker(markerType, SearchDirection::Forward, 0, EndLocation(), [&](const std::shared_ptr<RangeMarker>& pMarker) {
        markers.insert(pMarker);
        return true;
    });

    for (auto& victim : markers)
    {
        ClearRangeMarker(victim);
    }

    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::MarkersChanged, 0, BufferLocation(m_gapBuffer.size() - 1)));
}

void ZepBuffer::ForEachMarker(uint32_t markerType, SearchDirection dir, BufferLocation begin, BufferLocation end, const std::function<bool(const std::shared_ptr<RangeMarker>&)>& fnCB) const
{
    auto itrStart = m_rangeMarkers.lower_bound(begin);
    if (itrStart == m_rangeMarkers.end())
    {
        return;
    }

    auto itrEnd = m_rangeMarkers.upper_bound(end);

    if (dir == SearchDirection::Forward)
    {
        for (auto itr = itrStart; itr != itrEnd; itr++)
        {
            for (auto& markerItem : itr->second)
            {
                if ((markerItem->markerType & markerType) == 0)
                {
                    continue;
                }

                if (!fnCB(markerItem))
                {
                    return;
                }
            }
        }
    }
    else
    {
        auto itrREnd = std::make_reverse_iterator(itrStart);
        auto itrRStart = std::make_reverse_iterator(itrEnd);

        for (auto itr = itrRStart; itr != itrREnd; itr++)
        {
            for (auto& markerItem : itr->second)
            {
                if ((markerItem->markerType & markerType) == 0)
                {
                    continue;
                }
                if (!fnCB(markerItem))
                {
                    return;
                }
            }
        }
    }
}

void ZepBuffer::HideMarkers(uint32_t markerType)
{
    ForEachMarker(markerType, SearchDirection::Forward, 0, EndLocation(), [&](const std::shared_ptr<RangeMarker>& spMarker) {
        if ((spMarker->markerType & markerType) != 0)
        {
            spMarker->displayType = RangeMarkerDisplayType::Hidden;
        }
        return true;
    });
}

void ZepBuffer::ShowMarkers(uint32_t markerType, uint32_t displayType)
{
    ForEachMarker(markerType, SearchDirection::Forward, 0, EndLocation(), [&](const std::shared_ptr<RangeMarker>& spMarker) {
        if ((spMarker->markerType & markerType) != 0)
        {
            spMarker->displayType = displayType;
        }
        return true;
    });
}

auto ZepBuffer::GetRangeMarkers(uint32_t markerType) const -> tRangeMarkers
{
    tRangeMarkers markers;
    ForEachMarker(markerType, SearchDirection::Forward, 0, EndLocation(), [&](const std::shared_ptr<RangeMarker>& spMarker) {
        if ((spMarker->markerType & markerType) != 0)
        {
            markers[spMarker->range.first].insert(spMarker);
        }
        return true;
    });
    return markers;
}

auto ZepBuffer::FindNextMarker(BufferLocation start, SearchDirection dir, uint32_t markerType) -> std::shared_ptr<RangeMarker>
{
    start = std::max(0, start);

    std::shared_ptr<RangeMarker> spFound;
    auto search = [&]() {
        ForEachMarker(markerType, dir, 0, EndLocation(), [&](const std::shared_ptr<RangeMarker>& marker) {
            if (dir == SearchDirection::Forward)
            {
                if (marker->range.first <= start)
                {
                    return true;
                }
            }
            else
            {
                if (marker->range.first >= start)
                {
                    return true;
                }
            }

            spFound = marker;
            return false;
        });
    };

    search();
    if (spFound == nullptr)
    {
        // Wrap
        start = (dir == SearchDirection::Forward ? 0 : EndLocation());
        search();
    }
    return spFound;
}

void ZepBuffer::SetBufferType(BufferType type)
{
    m_bufferType = type;
}

auto ZepBuffer::GetBufferType() const -> BufferType
{
    return m_bufferType;
}

void ZepBuffer::SetLastEditLocation(BufferLocation loc)
{
    m_lastEditLocation = loc;
}

auto ZepBuffer::GetLastEditLocation() const -> BufferLocation
{
    return m_lastEditLocation;
}

auto ZepBuffer::GetMode() const -> ZepMode*
{
    if (m_spMode)
    {
        return m_spMode.get();
    }
    return GetEditor().GetGlobalMode();
}

void ZepBuffer::SetMode(const std::shared_ptr<ZepMode>& spMode)
{
    m_spMode = spMode;
}

void ZepBuffer::AddLineWidget(int32_t line, const std::shared_ptr<ILineWidget>& spWidget)
{
    // TODO(unknown): Add layout changed message
    int32_t start;
    int32_t end;
    GetLineOffsets(line, start, end);

    m_lineWidgets[start].push_back(spWidget);
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::TextChanged, 0, 0));
}

void ZepBuffer::ClearLineWidgets(int32_t line)
{
    if (line != -1)
    {
        int32_t start;
        int32_t end;
        GetLineOffsets(line, start, end);
        m_lineWidgets.erase(start);
    }
    else
    {
        m_lineWidgets.clear();
    }
    GetEditor().Broadcast(std::make_shared<BufferMessage>(this, BufferMessageType::TextChanged, 0, 0));
}

auto ZepBuffer::GetLineWidgets(int32_t line) const -> const ZepBuffer::tLineWidgets*
{
    int32_t start;
    int32_t end;
    GetLineOffsets(line, start, end);

    auto itrFound = m_lineWidgets.find(start);
    if (itrFound != m_lineWidgets.end())
    {
        return &itrFound->second;
    }
    return nullptr;
}

auto ZepBuffer::IsHidden() const -> bool
{
    auto windows = GetEditor().FindBufferWindows(this);
    return windows.empty();
}

} // namespace Zep
