#include "zep/theme.hpp"
#include "zep/editor.hpp"
#include "zep/syntax.hpp"

namespace Zep
{

ZepTheme::ZepTheme()
{
    double golden_ratio_conjugate = 0.618033988749895;
    double h = .85F;
    for (int i = 0; i < (int)ThemeColor::UniqueColorLast; i++)
    {
        h += golden_ratio_conjugate;
        h = std::fmod(h, 1.0);
        m_uniqueColors.emplace_back(HSVToRGB(float(h) * 360.0F, 0.6F, 200.0F));
    }
    SetThemeType(ThemeType::Dark);
}

void ZepTheme::SetThemeType(ThemeType type)
{
    m_currentTheme = type;
    switch (type)
    {
    default:
    case ThemeType::Dark:
        SetDarkTheme();
        break;
    case ThemeType::Light:
        SetLightTheme();
        break;
    }
}

auto ZepTheme::GetThemeType() const -> ThemeType
{
    return m_currentTheme;
}

void ZepTheme::SetDarkTheme()
{
    m_colors[ThemeColor::Text] = NVec4f(1.0F);
    m_colors[ThemeColor::Background] = NVec4f(0.13F, 0.13F, 0.13F, 1.0F);
    m_colors[ThemeColor::HiddenText] = NVec4f(.9F, .1F, .1F, 1.0F);
    m_colors[ThemeColor::TabBorder] = NVec4f(.55F, .55F, .55F, 1.0F);
    m_colors[ThemeColor::TabInactive] = NVec4f(.4F, .4F, .4F, .55F);
    m_colors[ThemeColor::TabActive] = NVec4f(.55F, .55F, .55F, 1.0F);
    m_colors[ThemeColor::LineNumberBackground] = m_colors[ThemeColor::Background] + NVec4f(.02F, .02F, .02F, 0.0F);
    m_colors[ThemeColor::LineNumber] = NVec4f(.13F, 1.0F, .13F, 1.0F);
    m_colors[ThemeColor::LineNumberActive] = NVec4f(.13F, 1.0F, .13F, 1.0F);
    m_colors[ThemeColor::CursorNormal] = NVec4f(130.0F / 255.0F, 140.0F / 255.0F, 230.0F / 255.0F, 1.0F);
    m_colors[ThemeColor::CursorInsert] = NVec4f(1.0F, 1.0F, 1.0F, .9F);
    m_colors[ThemeColor::CursorLineBackground] = NVec4f(.25F, .25F, .25F, 1.0F);
    m_colors[ThemeColor::AirlineBackground] = NVec4f(.15F, .15F, .15F, 1.0F);
    m_colors[ThemeColor::Light] = NVec4f(1.0F);
    m_colors[ThemeColor::Dark] = NVec4f(0.0F, 0.0F, 0.0F, 1.0F);
    m_colors[ThemeColor::VisualSelectBackground] = NVec4f(.47F, 0.30F, 0.25F, 1.0F);
    m_colors[ThemeColor::Mode] = NVec4f(.2F, 0.8F, 0.2F, 1.0F);

    m_colors[ThemeColor::Normal] = m_colors[ThemeColor::Text];
    m_colors[ThemeColor::Parenthesis] = m_colors[ThemeColor::Text];
    m_colors[ThemeColor::Comment] = NVec4f(0.0F, 1.0F, .1F, 1.0F);
    m_colors[ThemeColor::Keyword] = NVec4f(0.1F, 1.0F, 1.0F, 1.0F);
    m_colors[ThemeColor::Identifier] = NVec4f(1.0F, .75F, 0.5F, 1.0F);
    m_colors[ThemeColor::Number] = NVec4f(0.1F, 1.0F, 1.0F, 1.0F);
    m_colors[ThemeColor::String] = NVec4f(1.0F, 0.5F, 1.0F, 1.0F);
    m_colors[ThemeColor::Whitespace] = NVec4f(0.2F, .2F, .2F, 1.0F);

    m_colors[ThemeColor::Error] = NVec4f(0.65F, .2F, .15F, 1.0F);
    m_colors[ThemeColor::Warning] = NVec4f(0.15F, .2F, .65F, 1.0F);
    m_colors[ThemeColor::Info] = NVec4f(0.15F, .6F, .15F, 1.0F);

    m_colors[ThemeColor::WidgetBorder] = NVec4f(.5F, .5F, .5F, 1.0F);
    m_colors[ThemeColor::WidgetActive] = m_colors[ThemeColor::TabActive];
    m_colors[ThemeColor::WidgetInactive] = m_colors[ThemeColor::TabInactive];
    m_colors[ThemeColor::WidgetBackground] = NVec4f(.2F, .2F, .2F, 1.0F);
}

void ZepTheme::SetLightTheme()
{
    m_colors[ThemeColor::Text] = NVec4f(0.0F, 0.0F, 0.0F, 1.0F);
    m_colors[ThemeColor::Background] = NVec4f(1.0F, 1.0F, 1.0F, 1.0F);
    m_colors[ThemeColor::HiddenText] = NVec4f(.9F, .1F, .1F, 1.0F);
    m_colors[ThemeColor::TabBorder] = NVec4f(.55F, .55F, .55F, 1.0F);
    m_colors[ThemeColor::TabInactive] = NVec4f(.4F, .4F, .4F, .55F);
    m_colors[ThemeColor::TabActive] = NVec4f(.55F, .55F, .55F, 1.0F);
    m_colors[ThemeColor::LineNumberBackground] = m_colors[ThemeColor::Background] - NVec4f(.02F, .02F, .02F, 0.0F);
    m_colors[ThemeColor::LineNumber] = NVec4f(.13F, .4F, .13F, 1.0F);
    m_colors[ThemeColor::LineNumberActive] = NVec4f(.13F, 0.6F, .13F, 1.0F);
    m_colors[ThemeColor::CursorNormal] = NVec4f(130.0F / 255.0F, 140.0F / 255.0F, 230.0F / 255.0F, 1.0F);
    m_colors[ThemeColor::CursorInsert] = NVec4f(1.0F, 1.0F, 1.0F, .9F);
    m_colors[ThemeColor::CursorLineBackground] = NVec4f(.85F, .85F, .85F, 1.0F);
    m_colors[ThemeColor::AirlineBackground] = NVec4f(.85F, .85F, .85F, 1.0F);
    m_colors[ThemeColor::Light] = NVec4f(1.0F);
    m_colors[ThemeColor::Dark] = NVec4f(0.0F, 0.0F, 0.0F, 1.0F);
    m_colors[ThemeColor::VisualSelectBackground] = NVec4f(.49F, 0.60F, 0.45F, 1.0F);
    m_colors[ThemeColor::Mode] = NVec4f(.2F, 0.8F, 0.2F, 1.0F);

    m_colors[ThemeColor::Normal] = m_colors[ThemeColor::Text];
    m_colors[ThemeColor::Parenthesis] = m_colors[ThemeColor::Text];
    m_colors[ThemeColor::Comment] = NVec4f(0.1F, .4F, .1F, 1.0F);
    m_colors[ThemeColor::Keyword] = NVec4f(0.1F, .2F, .3F, 1.0F);
    m_colors[ThemeColor::Identifier] = NVec4f(0.2F, .2F, .1F, 1.0F);
    m_colors[ThemeColor::Number] = NVec4f(0.1F, .3F, .2F, 1.0F);
    m_colors[ThemeColor::String] = NVec4f(0.1F, .1F, .4F, 1.0F);
    m_colors[ThemeColor::Whitespace] = NVec4f(0.2F, .2F, .2F, 1.0F);

    m_colors[ThemeColor::Error] = NVec4f(0.89F, .2F, .15F, 1.0F);
    m_colors[ThemeColor::Warning] = NVec4f(0.15F, .2F, .89F, 1.0F);
    m_colors[ThemeColor::Info] = NVec4f(0.15F, .85F, .15F, 1.0F);

    m_colors[ThemeColor::WidgetActive] = m_colors[ThemeColor::TabActive];
    m_colors[ThemeColor::WidgetInactive] = m_colors[ThemeColor::TabInactive];

    m_colors[ThemeColor::WidgetBorder] = NVec4f(.5F, .5F, .5F, 1.0F);
    m_colors[ThemeColor::WidgetActive] = m_colors[ThemeColor::TabActive];
    m_colors[ThemeColor::WidgetInactive] = m_colors[ThemeColor::TabInactive];
    m_colors[ThemeColor::WidgetBackground] = NVec4f(.8F, .8F, .8F, 1.0F);
}

auto ZepTheme::GetUniqueColor(uint32_t index) const -> ThemeColor
{
    return ThemeColor((uint32_t)ThemeColor::UniqueColor0 + (uint32_t)(index % (uint32_t)ThemeColor::UniqueColorLast));
}

auto ZepTheme::GetColor(ThemeColor themeColor) const -> NVec4f
{
    if (themeColor >= ThemeColor::UniqueColor0)
    {
        // Return the unique color
        return m_uniqueColors[((uint32_t)themeColor - (uint32_t)ThemeColor::UniqueColor0) % (uint32_t)ThemeColor::UniqueColorLast];
    }

    auto itr = m_colors.find(themeColor);
    if (itr == m_colors.end())
    {
        return NVec4f(1.0F);
    }
    return itr->second;
}

auto ZepTheme::GetComplement(const NVec4f& col, const NVec4f& adjust) const -> NVec4f
{
    auto lum = Luminosity(col);
    if (lum > 0.5F)
    {
        return GetColor(ThemeColor::Dark) + adjust;
    }
    return GetColor(ThemeColor::Light) - adjust;
}

} // namespace Zep
