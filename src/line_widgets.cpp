#include "zep/line_widgets.hpp"
#include "zep/display.hpp"

namespace Zep
{

auto FloatSlider::GetSize() const -> NVec2f
{
    // Make the slider as high as the font, but return non-dpi scale
    return NVec2f((60.0F * m_dimension) + (m_sliderGap * ((float)m_dimension - 1)), m_editor.GetDisplay().GetFontHeightPixels() / m_editor.GetPixelScale());
}

void FloatSlider::MouseDown(const NVec2f& pos, ZepMouseButton button)
{
    (void)&pos, (void)&button;
}

void FloatSlider::MouseUp(const NVec2f& pos, ZepMouseButton button)
{
    (void)&pos, (void)&button;
}

void FloatSlider::MouseMove(const NVec2f& pos)
{
    (void)&pos;
}

void FloatSlider::Draw(const ZepBuffer& buffer, const NVec2f& loc)
{
    auto& display = m_editor.GetDisplay();

    for (uint32_t slider = 0; slider < m_dimension; slider++)
    {
        // Convert to low DPI, then double up on submit
        // We should do it this way more.
        auto location = loc / m_editor.GetPixelScale();
        location = NVec2f(location.x + (slider * (60.0F + m_sliderGap)), location.y);

        NVec2f size = GetSize();
        size.x -= (((float)m_dimension - 1) * m_sliderGap);
        size.x /= m_dimension;
        auto rc = NRectf(
            NVec2f(location.x, location.y),
            NVec2f(location.x + size.x, location.y + size.y));

        NVec2f margin = NVec2f(1, 1);
        NRectf rcInner = rc;
        rcInner.Adjust(margin.x, margin.y, -margin.x, -margin.y);

        display.DrawRectFilled(DPI(rc), buffer.GetTheme().GetColor(ThemeColor::WidgetBorder));
        display.DrawRectFilled(DPI(rcInner), buffer.GetTheme().GetColor(ThemeColor::WidgetBackground));

        NRectf rcThumb = rcInner;
        rcThumb.Adjust(margin.x, margin.y, -margin.x, -margin.y);
        rcThumb = NRectf(rcThumb.Left() + 10.0F, rcThumb.Top(), 10.0F, rcThumb.Size().y);
        display.DrawRectFilled(DPI(rcThumb), buffer.GetTheme().GetColor(ThemeColor::WidgetActive));
    }
}

void FloatSlider::Set(const NVec4f& value)
{
    m_value = value;
    if (m_fnChanged)
    {
        m_fnChanged(this);
    }
}

auto FloatSlider::Get() const -> const NVec4f&
{
    return m_value;
}

} // namespace Zep
