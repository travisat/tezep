#pragma once

#include <array>

#include "zep/buffer.hpp"

namespace Zep
{

class ZepTabWindow;

// A region inside the text for selections
struct SelectRegion
{
    // For vertical select, we will have a list of spans...
    BufferLocation start = 0;
    BufferLocation end = 0;
    bool visible = true;
    bool vertical = false; // Not yet supported
};

// Display interface
class ZepDisplay
{
public:
    virtual ~ZepDisplay()= default;;

    // Renderer specific overrides
    // Implement these to draw the buffer using whichever system you prefer
    virtual auto GetTextSize(const utf8* pBegin, const utf8* pEnd) const -> NVec2f = 0;
    [[nodiscard]] virtual auto GetFontPointSize() const -> float = 0;
    virtual void SetFontPointSize(float size)
    {
        (void)size;
    };
    [[nodiscard]] virtual auto GetFontHeightPixels() const -> float = 0;
    virtual void DrawLine(const NVec2f& start, const NVec2f& end, const NVec4f& color, float width) const = 0;
    virtual void DrawChars(const NVec2f& pos, const NVec4f& col, const utf8* text_begin, const utf8* text_end) const = 0;
    virtual void DrawRectFilled(const NRectf& rc, const NVec4f& col) const = 0;
    virtual void SetClipRect(const NRectf& rc) = 0;

    virtual auto GetCharSize(const utf8* pChar) -> NVec2f;
    virtual auto GetDefaultCharSize() -> const NVec2f&;
    virtual void InvalidateCharCache();

protected:
    void BuildCharCache();

protected:
    bool m_charCacheDirty = true;
    std::array<NVec2f, 256> m_charCache;
    NVec2f m_defaultCharSize;
};

// A NULL renderer, used for testing
// Discards all drawing, and returns text fixed_size of 1 pixel per char, 10 height!
// This is the only work you need to do to make a new renderer type for the editor
class ZepDisplayNull : public ZepDisplay
{
public:
     auto GetTextSize(const utf8* pBegin, const utf8* pEnd) const -> NVec2f override
    {
        return NVec2f(float(pEnd - pBegin), 10.0F);
    }
    [[nodiscard]]  auto GetFontPointSize() const -> float override
    {
        return 10;
    }
     [[nodiscard]] auto GetFontHeightPixels() const -> float override
    {
        return 10;
    }
     void DrawLine(const NVec2f& start, const NVec2f& end, const NVec4f& color, float width) const override
    {
        (void)start;
        (void)end;
        (void)color;
        (void)width;
    };
     void DrawChars(const NVec2f& pos, const NVec4f& col, const utf8* text_begin, const utf8* text_end) const override
    {
        (void)pos;
        (void)col;
        (void)text_begin;
        (void)text_end;
    }
     void DrawRectFilled(const NRectf& a, const NVec4f& col) const override
    {
        (void)a;
        (void)col;
    };
     void SetClipRect(const NRectf& rc) override
    {
        (void)rc;
    }
};

} // namespace Zep
