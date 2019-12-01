#pragma once

#include "zep/mcommon/math/math.hpp"
#include <algorithm>
#include <limits>
#include <memory>
#include <ostream>
#include <vector>


namespace Zep
{

namespace RegionFlags
{
enum Flags
{
    Fixed = (1 << 0),
    Expanding = (1 << 1)
};
} // namespace RegionFlags

struct Region
{
    const char* pszName = nullptr;
    uint32_t flags = RegionFlags::Expanding;
    float ratio = 1.0F;
    NRectf rect;
    NVec2f min_size = NVec2f(0.0F, 0.0F);
    NVec2f fixed_size = NVec2f(0.0F, 0.0F);
    bool vertical = true;
    NVec2f margin = NVec2f(0.0F, 0.0F);

    std::shared_ptr<Region> pParent;
    std::vector<std::shared_ptr<Region>> children;
};

inline auto operator<<(std::ostream& str, const Region& region) -> std::ostream&
{
    static int indent = 0;
    auto do_indent = [&str](int sz) { for (int i = 0; i < sz; i++) { str << " "; 
} };

    do_indent(indent);
    if (region.pszName != nullptr)
    {
        str << region.pszName << " ";
    }
    str << std::hex << &region << " -> ";

    str << "RC: " << region.rect << ", pParent: " << std::hex << region.pParent;
    if (!region.children.empty())
    {
        str << std::endl;
        for (auto& child : region.children)
        {
            indent++;
            str << *child;
            indent--;
        }
    }
    str << std::endl;

    return str;
}

void LayoutRegion(Region& region);

} // namespace Zep
