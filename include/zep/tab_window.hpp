#pragma once

#include "zep/buffer.hpp"
#include <deque>

namespace Zep
{

class ZepWindow;
class ZepDisplay;
struct Region;

enum class WindowMotion
{
    Left,
    Right,
    Up,
    Down
};

// Display state for a single pane of text.
// Editor operations such as select and change are local to a displayed pane
class ZepTabWindow : public ZepComponent
{
public:
    explicit ZepTabWindow(ZepEditor& editor);
    ~ZepTabWindow() override;

    void Notify(std::shared_ptr<ZepMessage> message) override;

    auto DoMotion(WindowMotion motion) -> ZepWindow*;
    auto AddWindow(ZepBuffer* pBuffer, ZepWindow* pParent, bool vsplit) -> ZepWindow*;
    void RemoveWindow(ZepWindow* pWindow);
    void SetActiveWindow(ZepWindow* pBuffer);
    [[nodiscard]] auto GetActiveWindow() const -> ZepWindow*
    {
        return m_pActiveWindow;
    }
    void CloseActiveWindow();

    using tWindows = std::vector<ZepWindow*>;
    using tWindowRegions = std::map<ZepWindow*, std::shared_ptr<Region>>;
    [[nodiscard]] auto GetWindows() const -> const tWindows&
    {
        return m_windows;
    }

    void SetDisplayRegion(const NRectf& region, bool force = false);

    void Display();

private:
    NRectf m_lastRegionRect;

    tWindows m_windows;
    tWindowRegions m_windowRegions;
    std::shared_ptr<Region> m_spRootRegion;
    ZepWindow* m_pActiveWindow = nullptr;
};

} // namespace Zep
