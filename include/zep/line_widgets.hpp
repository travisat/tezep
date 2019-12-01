#pragma once

#include "zep/editor.hpp"

#include <utility>

namespace Zep
{

struct ILineWidget
{
    [[nodiscard]] virtual auto GetSize() const -> NVec2f = 0; // Required size of the widget
    virtual void MouseDown(const NVec2f& pos, ZepMouseButton button) = 0;
    virtual void MouseUp(const NVec2f& pos, ZepMouseButton button) = 0;
    virtual void MouseMove(const NVec2f& pos) = 0;
    virtual void Draw(const ZepBuffer& buffer, const NVec2f& location) = 0;
    virtual void Set(const NVec4f& value) = 0;
    [[nodiscard]] virtual auto Get() const -> const NVec4f& = 0;
};

using fnWidgetValueChanged = std::function<void(ILineWidget* pWidget)>;
class FloatSlider : public ILineWidget
{
public:
    FloatSlider(ZepEditor& editor, uint32_t dimension, fnWidgetValueChanged fnChanged = nullptr)
        : m_editor(editor),
        m_dimension(dimension),
        m_fnChanged(std::move(fnChanged))
    {

    }
    [[nodiscard]]  auto GetSize() const -> NVec2f override;
     void MouseDown(const NVec2f& pos, ZepMouseButton button) override;
     void MouseUp(const NVec2f& pos, ZepMouseButton button) override;
     void MouseMove(const NVec2f& pos) override;
     void Draw(const ZepBuffer& buffer, const NVec2f& location) override;
     void Set(const NVec4f& value) override;
     [[nodiscard]] auto Get() const -> const NVec4f& override;

private:
    [[nodiscard]] virtual auto GetEditor() const -> ZepEditor& {
        return m_editor;
    };

private:
    ZepEditor& m_editor;
    uint32_t m_dimension = 1;
    NVec2f m_range = NVec2f(0.0F, 1.0F);
    NVec4f m_value = NVec4f(0.0F, 0.0F, 0.0F, 0.0F);
    float m_sliderGap = 5.0F;
    fnWidgetValueChanged m_fnChanged = nullptr;
};

}  // namespace Zep
