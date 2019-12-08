#pragma once

#include <map>
#include <vector>

#include "zep/mcommon/math/math.hpp"

namespace Zep
{

enum class ThemeColor
{
    None,
    TabBorder,
    HiddenText,
    Text,
    Background,
    TabInactive,
    TabActive,
    LineNumberBackground,
    LineNumber,
    LineNumberActive,
    CursorNormal,
    CursorInsert,
    Light,
    Dark,
    VisualSelectBackground,
    CursorLineBackground,
    AirlineBackground,
    Mode,
    Normal,
    Keyword,
    Identifier,
    Number,
    String,
    Comment,
    Whitespace,
    HiddenChar,
    Parenthesis,
    Error,
    Warning,
    Info,
    WidgetBorder,
    WidgetBackground,
    WidgetActive,
    WidgetInactive,

    // Last in the list!
    // A set of pregenerated, easy to differentiate unique colors
    UniqueColor0,
    UniqueColor1,
    UniqueColor2,
    UniqueColor3,
    UniqueColor4,
    UniqueColor5,
    UniqueColor6,
    UniqueColor7,
    UniqueColor8,
    UniqueColor9,
    UniqueColor10,
    UniqueColor11,
    UniqueColor12,
    UniqueColor13,
    UniqueColor14,
    UniqueColor15,
    UniqueColorLast,
};

enum class ThemeType
{
    Dark,
    Light
};

class ZepTheme
{
public:
    ZepTheme();
    virtual ~ZepTheme() = default;

    [[nodiscard]] virtual auto GetColor(ThemeColor themeColor) const -> NVec4f;
    [[nodiscard]] virtual auto GetComplement(const NVec4f& col, const NVec4f& adjust) const -> NVec4f;
    [[nodiscard]] virtual auto GetUniqueColor(uint32_t index) const -> ThemeColor;

    void SetThemeType(ThemeType type);
    [[nodiscard]] auto GetThemeType() const -> ThemeType;

private:
    void SetDarkTheme();
    void SetLightTheme();

private:
    std::vector<NVec4f> m_uniqueColors;
    std::map<ThemeColor, NVec4f> m_colors;
    ThemeType m_currentTheme = ThemeType::Dark;
};

} // namespace Zep
