#pragma once

#include <cmath>
#include <ostream>


// 2D and 4D vectors, used for area and color calculations.
// Some helpers for color conversion too.
// This just saves using a library like glm (my personal preference)
// - and it keeps the dependencies of Zep to just the source folder contents

namespace Zep
{

template <class T>
struct NVec2
{
    NVec2(T xVal, T yVal)
        : x(xVal)
        , y(yVal)
    {
    }

    explicit NVec2(T v)
        : x(v)
        , y(v)
    {
    }

    NVec2()
        : x(0)
        , y(0)
    {
    }

    T x;
    T y;

    auto operator==(const NVec2<T>& rhs) const -> bool
    {
        if (x == rhs.x && y == rhs.y)
            return true;
        return false;
    }

    auto operator!=(const NVec2<T>& rhs) const -> bool
    {
        return !(*this == rhs);
    }
};
template <class T>
inline auto operator+(const NVec2<T>& lhs, const NVec2<T>& rhs) -> NVec2<T>
{
    return NVec2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}
template <class T>
inline auto operator-(const NVec2<T>& lhs, const NVec2<T>& rhs) -> NVec2<T>
{
    return NVec2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}
template <class T>
inline auto operator/(const NVec2<T>& lhs, const NVec2<T>& rhs) -> NVec2<T>
{
    return NVec2<T>(lhs.x / rhs.x, lhs.y / rhs.y);
}
template <class T>
inline auto operator+=(NVec2<T>& lhs, const NVec2<T>& rhs) -> NVec2<T>&
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}
template <class T>
inline auto operator-=(NVec2<T>& lhs, const NVec2<T>& rhs) -> NVec2<T>&
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}
template <class T>
inline auto operator*(const NVec2<T>& lhs, float val) -> NVec2<T>
{
    return NVec2<T>(lhs.x * val, lhs.y * val);
}
template <class T>
inline auto operator/(const NVec2<T>& lhs, float val) -> NVec2<T>
{
    return NVec2<T>(lhs.x / val, lhs.y / val);
}
template <class T>
inline auto operator*=(NVec2<T>& lhs, float val) -> NVec2<T>&
{
    lhs.x *= val;
    lhs.y *= val;
    return lhs;
}
template <class T>
inline auto operator<(const NVec2<T>& lhs, const NVec2<T>& rhs) -> bool
{
    if (lhs.x != rhs.x)
    {
        return lhs.x < rhs.x;
    }
    return lhs.y < rhs.y;
}
template <class T>
inline auto Clamp(const NVec2<T>& val, const NVec2<T>& min, const NVec2<T>& max) -> NVec2<T>
{
    return NVec2<T>(std::min(max.x, std::max(min.x, val.x)), std::min(max.y, std::max(min.y, val.y)));
}
template <class T>
inline auto ManhattanDistance(const NVec2<T>& l, const NVec2<T>& r) -> T
{
    return std::abs(l.x - r.x) + std::abs(r.y - l.y);
}

template <class T>
auto operator<<(std::ostream& str, const NVec2<T>& v) -> std::ostream&
{
    str << "(" << v.x << ", " << v.y << ")";
    return str;
}

using NVec2f = NVec2<float>;
using NVec2i = NVec2<int32_t>;

template <class T>
struct NVec4
{
    NVec4(T xVal, T yVal, T zVal, T wVal)
        : x(xVal)
        , y(yVal)
        , z(zVal)
        , w(wVal)
    {
    }

    explicit NVec4(T val)
        : NVec4(val, val, val, val)
    {
    }

    NVec4()
        : x(0)
        , y(0)
        , z(0)
        , w(1)
    {
    }

    T x;
    T y;
    T z;
    T w;

    auto operator==(const NVec4<T>& rhs) const -> bool
    {
        if (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w)
            return true;
        return false;
    }

    auto operator!=(const NVec4<T>& rhs) const -> bool
    {
        return !(*this = rhs);
    }
};
template <class T>
inline auto operator+(const NVec4<T>& lhs, const NVec4<T>& rhs) -> NVec4<T>
{
    return NVec4<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}
template <class T>
inline auto operator-(const NVec4<T>& lhs, const NVec4<T>& rhs) -> NVec4<T>
{
    return NVec4<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}
template <class T>
inline auto operator+=(NVec4<T>& lhs, const NVec4<T>& rhs) -> NVec4<T>&
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    lhs.w += rhs.w;
    return lhs;
}
template <class T>
inline auto operator-=(NVec4<T>& lhs, const NVec4<T>& rhs) -> NVec4<T>&
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    lhs.w -= rhs.w;
    return lhs;
}
template <class T>
inline auto operator*(const NVec4<T>& lhs, float val) -> NVec4<T>
{
    return NVec4<T>(lhs.x * val, lhs.y * val, lhs.z * val, lhs.w * val);
}
template <class T>
inline auto operator*=(NVec4<T>& lhs, float val) -> NVec4<T>&
{
    lhs.x *= val;
    lhs.y *= val;
    lhs.z *= val;
    lhs.w *= val;
    return lhs;
}
template <class T>
inline auto Clamp(const NVec4<T>& val, const NVec4<T>& min, const NVec4<T>& max) -> NVec4<T>
{
    return NVec4<T>(std::min(max.x, std::max(min.x, val.x)),
        std::min(max.y, std::max(min.y, val.y)),
        std::min(max.z, std::max(min.z, val.z)),
        std::min(max.w, std::max(min.w, val.w)));
}

inline auto ToPacked(const NVec4<float>& val) -> uint32_t
{
    uint32_t col = 0;
    col |= uint32_t(val.x * 255.0F) << 24;
    col |= uint32_t(val.y * 255.0F) << 16;
    col |= uint32_t(val.z * 255.0F) << 8;
    col |= uint32_t(val.w * 255.0F);
    return col;
}

inline auto ToPackedARGB(const NVec4<float>& val) -> uint32_t
{
    uint32_t col = 0;
    col |= uint32_t(val.w * 255.0F) << 24;
    col |= uint32_t(val.x * 255.0F) << 16;
    col |= uint32_t(val.y * 255.0F) << 8;
    col |= uint32_t(val.z * 255.0F);
    return col;
}

inline auto ToPackedABGR(const NVec4<float>& val) -> uint32_t
{
    uint32_t col = 0;
    col |= uint32_t(val.w * 255.0F) << 24;
    col |= uint32_t(val.x * 255.0F);
    col |= uint32_t(val.y * 255.0F) << 8;
    col |= uint32_t(val.z * 255.0F) << 16;
    return col;
}

inline auto ToPackedBGRA(const NVec4<float>& val) -> uint32_t
{
    uint32_t col = 0;
    col |= uint32_t(val.w * 255.0F) << 8;
    col |= uint32_t(val.x * 255.0F) << 16;
    col |= uint32_t(val.y * 255.0F) << 24;
    col |= uint32_t(val.z * 255.0F);
    return col;
}

inline auto Luminosity(const NVec4<float>& intensity) -> float
{
    return (0.2126F * intensity.x + 0.7152F * intensity.y + 0.0722F * intensity.z);
}

inline auto HSVToRGB(float h, float s, float v) -> NVec4<float>
{
    auto r = 0.0F;
    auto g = 0.0F;
    auto b = 0.0F;

    if (s == 0)
    {
        r = v;
        g = v;
        b = v;
    }
    else
    {
        int i;
        float f;
        float p;
        float q;
        float t;

        if (h == 360)
        {
            h = 0;
        }
        else
        {
            h = h / 60.0F;
        }

        i = (int)trunc(h);
        f = h - static_cast<float>(i);

        p = v * (1.0F - s);
        q = v * (1.0F - (s * f));
        t = v * (1.0F - (s * (1.0F - f)));

        switch (i)
        {
        case 0:
            r = v;
            g = t;
            b = p;
            break;

        case 1:
            r = q;
            g = v;
            b = p;
            break;

        case 2:
            r = p;
            g = v;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = v;
            break;

        case 4:
            r = t;
            g = p;
            b = v;
            break;

        default:
            r = v;
            g = p;
            b = q;
            break;
        }
    }

    return NVec4<float>(r / 255.0F, g / 255.0F, b / 255.0F, 1.0F);
}

template <class T>
inline auto operator<<(std::ostream& str, const NVec4<T>& region) -> std::ostream&
{
    str << "(" << region.x << ", " << region.y << ", " << region.z << ", " << region.w << ")";
    return str;
}

using NVec4f = NVec4<float>;
using NVec4i = NVec4<int32_t>;

template <class T>
struct NRect
{
    NRect(const NVec2<T>& topLeft, const NVec2<T>& bottomRight)
        : topLeftPx(topLeft)
        , bottomRightPx(bottomRight)
    {
    }

    NRect(T left, T top, T width, T height)
        : topLeftPx(NVec2<T>(left, top))
        , bottomRightPx(NVec2<T>(left, top) + NVec2<T>(width, height))
    {
    }

    NRect()
    {
    }

    NVec2f topLeftPx;
    NVec2f bottomRightPx;

    auto Contains(const NVec2<T>& pt) const -> bool
    {
        return topLeftPx.x <= pt.x && topLeftPx.y <= pt.y && bottomRightPx.x > pt.x && bottomRightPx.y > pt.y;
    }

    [[nodiscard]] auto BottomLeft() const -> NVec2f
    {
        return NVec2f(topLeftPx.x, bottomRightPx.y);
    }
    [[nodiscard]] auto TopRight() const -> NVec2f
    {
        return NVec2f(bottomRightPx.x, topLeftPx.y);
    }

    [[nodiscard]] auto Left() const -> float
    {
        return topLeftPx.x;
    }
    [[nodiscard]] auto Right() const -> float
    {
        return TopRight().x;
    }
    [[nodiscard]] auto Top() const -> float
    {
        return TopRight().y;
    }
    [[nodiscard]] auto Bottom() const -> float
    {
        return bottomRightPx.y;
    }
    [[nodiscard]] auto Height() const -> float
    {
        return bottomRightPx.y - topLeftPx.y;
    }
    [[nodiscard]] auto Width() const -> float
    {
        return bottomRightPx.x - topLeftPx.x;
    }
    [[nodiscard]] auto Center() const -> NVec2f
    {
        return (bottomRightPx + topLeftPx) * .5f;
    }
    [[nodiscard]] auto Size() const -> NVec2f
    {
        return bottomRightPx - topLeftPx;
    }
    [[nodiscard]] auto Empty() const -> bool
    {
        return (Height() == 0.0f || Width() == 0.0f) ? true : false;
    }
    void Clear()
    {
        topLeftPx = NRect<T>();
        bottomRightPx = NRect<T>();
    }

    void Adjust(float x, float y, float z, float w)
    {
        topLeftPx.x += x;
        topLeftPx.y += y;
        bottomRightPx.x += z;
        bottomRightPx.y += w;
    }

    void Adjust(float x, float y)
    {
        topLeftPx.x += x;
        topLeftPx.y += y;
        bottomRightPx.x += x;
        bottomRightPx.y += y;
    }

    void Move(float x, float y)
    {
        auto width = Width();
        auto height = Height();
        topLeftPx.x = x;
        topLeftPx.y = y;
        bottomRightPx.x = x + width;
        bottomRightPx.y = y + height;
    }

    auto operator==(const NRect<T>& region) const -> bool
    {
        return (topLeftPx == region.topLeftPx) && (bottomRightPx == region.bottomRightPx);
    }
    auto operator!=(const NRect<T>& region) const -> bool
    {
        return !(*this == region);
    }
};

template <class T>
inline auto operator*(const NRect<T>& lhs, float val) -> NRect<T>
{
    return NRect<T>(lhs.topLeftPx * val, lhs.bottomRightPx * val);
}
template <class T>
inline auto operator<<(std::ostream& str, const NRect<T>& region) -> std::ostream&
{
    str << region.topLeftPx << ", " << region.bottomRightPx << ", size: " << region.Width() << ", " << region.Height();
    return str;
}

using NRectf = NRect<float>;

} // namespace Zep
