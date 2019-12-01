#pragma once

#include "zep/editor.hpp"
#include "zep/mcommon/animation/timer.hpp"
#include "zep/splits.hpp"

namespace Zep
{
class ZepTheme;
class ZepEditor;

class Scroller : public ZepComponent
{
public:
    Scroller(ZepEditor& editor, Region& parent);

    virtual void Display(ZepTheme& theme);
    void Notify(std::shared_ptr<ZepMessage> message) override;

    float vScrollVisiblePercent = 1.0F;
    float vScrollPosition = 0.0F;
    float vScrollLinePercent = 0.0F;
    float vScrollPagePercent = 0.0F;
    bool vertical = true;

private:
    void CheckState();
    void ClickUp();
    void ClickDown();
    void PageUp();
    void PageDown();
    void DoMove(NVec2f pos);

    [[nodiscard]] auto ThumbSize() const -> float;
    [[nodiscard]] auto ThumbExtra() const -> float;
    [[nodiscard]] auto ThumbRect() const -> NRectf;

private:
    std::shared_ptr<Region> m_region;
    std::shared_ptr<Region> m_topButtonRegion;
    std::shared_ptr<Region> m_bottomButtonRegion;
    std::shared_ptr<Region> m_mainRegion;
    timer m_start_delay_timer;
    timer m_reclick_timer;
    enum class ScrollState
    {
        None,
        ScrollDown,
        ScrollUp,
        PageUp,
        PageDown,
        Drag
    };
    ScrollState m_scrollState = ScrollState::None;
    NVec2f m_mouseDownPos;
    float m_mouseDownPercent{};
};

}; // namespace Zep
