#include "zep/syntax_rainbow_brackets.hpp"
#include "zep/theme.hpp"

#include "zep/mcommon/logger.hpp"
#include "zep/mcommon/string/stringutils.hpp"

// A Simple adornment to add rainbow brackets to the syntax
namespace Zep
{

ZepSyntaxAdorn_RainbowBrackets::ZepSyntaxAdorn_RainbowBrackets(ZepSyntax& syntax, ZepBuffer& buffer)
    : ZepSyntaxAdorn(syntax, buffer)
{
    syntax.GetEditor().RegisterCallback(this);

    Update(0, buffer.EndLocation());
}

ZepSyntaxAdorn_RainbowBrackets::~ZepSyntaxAdorn_RainbowBrackets() = default;

void ZepSyntaxAdorn_RainbowBrackets::Notify(std::shared_ptr<ZepMessage> message) 
{
    // Handle any interesting buffer messages
    if (message->messageId == Msg::Buffer)
    {
        auto spBufferMsg = std::static_pointer_cast<BufferMessage>(message);
        if (spBufferMsg->pBuffer != &m_buffer)
        {
            return;
        }
        if (spBufferMsg->type == BufferMessageType::TextDeleted)
        {
            Clear(spBufferMsg->startLocation, spBufferMsg->endLocation);
        }
        else if (spBufferMsg->type == BufferMessageType::TextAdded || spBufferMsg->type == BufferMessageType::Loaded)
        {
            Insert(spBufferMsg->startLocation, spBufferMsg->endLocation);
            Update(spBufferMsg->startLocation, spBufferMsg->endLocation);
        }
        else if (spBufferMsg->type == BufferMessageType::TextChanged)
        {
            Update(spBufferMsg->startLocation, spBufferMsg->endLocation);
        }
    }
}

auto ZepSyntaxAdorn_RainbowBrackets::GetSyntaxAt(int32_t offset, bool& found) const -> SyntaxData
{
    SyntaxData data;
    auto itr = m_brackets.find(offset);
    if (itr == m_brackets.end())
    {
        found = false;
        return data;
    }

    found = true;
    if (!itr->second.valid)
    {
        data.foreground = ThemeColor::Text;
        data.background = ThemeColor::Error;
    }
    else
    {
        data.foreground = (ThemeColor)(((int32_t)ThemeColor::UniqueColor0 + itr->second.indent) % (int32_t)ThemeColor::UniqueColorLast);
        data.background = ThemeColor::None;
    }

    return data;
}

void ZepSyntaxAdorn_RainbowBrackets::Insert(int32_t start, int32_t end)
{
    // Adjust all the brackets after us by the same distance
    auto diff = end - start;
    std::map<BufferLocation, Bracket> replace;
    for (auto& b : m_brackets)
    {
        if (b.first < start)
        {
            replace[b.first] = b.second;
        }
        else
        {
            replace[b.first + diff] = b.second;
        }
    }
    std::swap(m_brackets, replace);

    RefreshBrackets();
}

void ZepSyntaxAdorn_RainbowBrackets::Clear(int32_t start, int32_t end)
{
    // Remove brackets in the erased section
    for (auto current = start; current < end; current++)
    {
        m_brackets.erase(BufferLocation(current));
    }

    // Adjust remaining brackets by the difference
    auto diff = end - start;
    std::map<BufferLocation, Bracket> replace;
    for (auto& b : m_brackets)
    {
        if (b.first < start)
        {
            replace[b.first] = b.second;
        }
        else
        {
            replace[b.first - diff] = b.second;
        }
    }
    std::swap(m_brackets, replace);

    RefreshBrackets();
}

void ZepSyntaxAdorn_RainbowBrackets::Update(int32_t start, int32_t end)
{
    auto& buffer = m_buffer.GetText();
    auto itrStart = buffer.begin() + start;
    auto itrEnd = buffer.begin() + end;

    for (auto itrBracket = itrStart; itrBracket != itrEnd; itrBracket++)
    {
        auto offset = itrBracket - buffer.begin();
        if (*itrBracket == '(')
        {
            m_brackets[BufferLocation(offset)] = Bracket{ 0, BracketType::Bracket, true };
        }
        else if (*itrBracket == ')')
        {
            m_brackets[BufferLocation(offset)] = Bracket{ 0, BracketType::Bracket, false };
        }
        else if (*itrBracket == '[')
        {
            m_brackets[BufferLocation(offset)] = Bracket{ 0, BracketType::Group, true };
        }
        else if (*itrBracket == ']')
        {
            m_brackets[BufferLocation(offset)] = Bracket{ 0, BracketType::Group, false };
        }
        else if (*itrBracket == '{')
        {
            m_brackets[BufferLocation(offset)] = Bracket{ 0, BracketType::Brace, true };
        }
        else if (*itrBracket == '}')
        {
            m_brackets[BufferLocation(offset)] = Bracket{ 0, BracketType::Brace, false };
        }
        else
        {
            auto itr = m_brackets.find(BufferLocation(offset));
            if (itr != std::end(m_brackets))
            {
                m_brackets.erase(itr);
            }
        }
    }
    RefreshBrackets();
}

void ZepSyntaxAdorn_RainbowBrackets::RefreshBrackets()
{
    std::vector<int32_t> indents((int)BracketType::Max, 0);
    for (auto& b : m_brackets)
    {
        auto& bracket = b.second;
        if (!bracket.is_open)
        {
            indents[int(bracket.type)]--;
        }
        bracket.indent = indents[int(bracket.type)];
        // Allow one bracket error, before going back to normal
        bracket.valid = indents[int(bracket.type)] >= 0;
        if (!bracket.valid)
        {
            indents[int(bracket.type)] = 0;
        }
        if (bracket.is_open)
        {
            indents[int(bracket.type)]++;
        }
    }

    auto MarkTails = [&](auto type) {
        if (indents[int(type)] > 0)
        {
            for (auto& b : m_brackets)
            {
                if (b.second.type == type)
                {
                    b.second.valid = false;
                    return;
                }
            }
        }
    };
    MarkTails(BracketType::Brace);
    MarkTails(BracketType::Bracket);
    MarkTails(BracketType::Group);
}

} // namespace Zep
