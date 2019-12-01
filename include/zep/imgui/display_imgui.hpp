#pragma once
#include <string>

#include "zep/display.hpp"
#include "zep/syntax.hpp"

#include <imgui.h>

// Can't include this publicly
//#include "zep/mcommon/logger.h"

namespace Zep
{

inline auto toNVec2f(const ImVec2& im) -> NVec2f
{
    return NVec2f(im.x, im.y);
}
inline auto toImVec2(const NVec2f& im) -> ImVec2
{
    return ImVec2(im.x, im.y);
}

class ZepDisplay_ImGui : public ZepDisplay
{
public:
    // ImGui specific display methods
    [[nodiscard]] auto GetFontPointSize() const -> float override
    {
        return ImGui::GetFontSize();
    }

    void SetFontPointSize(float size) override
    {
        // TODO(unknown): Allow change of font size in ImGui
        // See the Qt demo for functional font size changing with CTRL+/-
        (void)size;
    }

    [[nodiscard]] auto GetFontHeightPixels() const -> float override
    {
        // TODO(unknown): Check low DPI monitor for correct behavior
        return ImGui::GetFontSize();
    }

    auto GetTextSize(const utf8* pBegin, const utf8* pEnd) const -> NVec2f override
    {
        // This is the code from ImGui internals; we can't call GetTextSize, because it doesn't return the correct 'advance' formula, which we
        // need as we draw one character at a time...
        ImFont* font = ImGui::GetFont();
        const float font_size = ImGui::GetFontSize();
        ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, FLT_MAX, reinterpret_cast<const char*>(pBegin), reinterpret_cast<const char*>(pEnd), nullptr);
        if (text_size.x == 0.0)
        {
            // Make invalid characters a default fixed_size
            return GetTextSize(reinterpret_cast<const utf8*>("A"), nullptr);
        }

        return toNVec2f(text_size);
    }

    void DrawChars(const NVec2f& pos, const NVec4f& col, const utf8* text_begin, const utf8* text_end) const override
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (text_end == nullptr)
        {
            text_end = text_begin + strlen(reinterpret_cast<const char*>(text_begin));
        }
        if (m_clipRect.Width() == 0)
        {
            drawList->AddText(toImVec2(pos), ToPackedABGR(col), reinterpret_cast<const char*>(text_begin), reinterpret_cast<const char*>(text_end));
        }
        else
        {
            drawList->PushClipRect(toImVec2(m_clipRect.topLeftPx), toImVec2(m_clipRect.bottomRightPx));
            drawList->AddText(toImVec2(pos), ToPackedABGR(col), reinterpret_cast<const char*>(text_begin), reinterpret_cast<const char*>(text_end));
            drawList->PopClipRect();
        }
    }

    void DrawLine(const NVec2f& start, const NVec2f& end, const NVec4f& color, float width) const override
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        // Background rect for numbers
        if (m_clipRect.Width() == 0)
        {
            drawList->AddLine(toImVec2(start), toImVec2(end), ToPackedABGR(color), width);
        }
        else
        {
            drawList->PushClipRect(toImVec2(m_clipRect.topLeftPx), toImVec2(m_clipRect.bottomRightPx));
            drawList->AddLine(toImVec2(start), toImVec2(end), ToPackedABGR(color), width);
            drawList->PopClipRect();
        }
    }

    void DrawRectFilled(const NRectf& rc, const NVec4f& color) const override
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        // Background rect for numbers
        if (m_clipRect.Width() == 0)
        {
            drawList->AddRectFilled(toImVec2(rc.topLeftPx), toImVec2(rc.bottomRightPx), ToPackedABGR(color));
        }
        else
        {
            drawList->PushClipRect(toImVec2(m_clipRect.topLeftPx), toImVec2(m_clipRect.bottomRightPx));
            drawList->AddRectFilled(toImVec2(rc.topLeftPx), toImVec2(rc.bottomRightPx), ToPackedABGR(color));
            drawList->PopClipRect();
        }
        //LOG(INFO) << "Draw: RC: " << rc << ", Color:" << color;
    }

    void SetClipRect(const NRectf& rc) override
    {
        m_clipRect = rc;
    }

private:
    NRectf m_clipRect;
};

} // namespace Zep
