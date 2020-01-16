#include "zep/display.hpp"

#include "zep/mcommon/logger.hpp"
#include "zep/mcommon/string/stringutils.hpp"

// A 'window' is like a vim window; i.e. a region inside a tab
namespace Zep
{

void ZepDisplay::InvalidateCharCache()
{
    m_charCacheDirty = true;
}

void ZepDisplay::BuildCharCache()
{
    const char chA = 'A';
    const auto chSize = reinterpret_cast<const utf8*>(&chA);
    m_defaultCharSize = GetTextSize(chSize, chSize + 1);
    for (int i = 0; i < 256; i++)
    {
        utf8 ch = (utf8)i;
        m_charCache[i] = GetTextSize(&ch, &ch + 1);
    }
    m_charCacheDirty = false;
}

auto ZepDisplay::GetDefaultCharSize() -> const NVec2f&
{
    if (m_charCacheDirty)
    {
        BuildCharCache();
    }
    return m_defaultCharSize;
}

auto ZepDisplay::GetCharSize(const utf8* pCh) -> NVec2f
{
    if (m_charCacheDirty)
    {
        BuildCharCache();
    }
    return m_charCache[*pCh];
}

} // namespace Zep
