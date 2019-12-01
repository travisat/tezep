#pragma once

#include "zep/mode.hpp"

namespace Zep
{

class ZepMode_Standard : public ZepMode
{
public:
    explicit ZepMode_Standard(ZepEditor& editor);
    ~ZepMode_Standard() override;

    void AddKeyPress(uint32_t key, uint32_t modifiers) override;
    void Begin() override;

    static auto StaticName() -> const char*
    {
        return "Standard";
    }
    [[nodiscard]] auto Name() const -> const char* override
    {
        return StaticName();
    }

private:
    virtual auto SwitchMode(EditorMode mode) -> bool;
    std::string keyCache;
};

} // namespace Zep
