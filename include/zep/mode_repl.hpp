#pragma once

#include "zep/mode.hpp"

#include <future>
#include <memory>
#include <regex>

namespace Zep
{

struct ZepRepl;

class ZepMode_Repl : public ZepMode
{
public:
    ZepMode_Repl(ZepEditor& editor, ZepWindow& launchWindow, ZepWindow& replWindow);
    ~ZepMode_Repl() override;

    void AddKeyPress(uint32_t key, uint32_t modifiers) override;
    void Begin() override;
    void Notify(std::shared_ptr<ZepMessage> message) override;

    static auto StaticName() -> const char*
    {
        return "REPL";
    }
    [[nodiscard]] auto Name() const -> const char* override
    {
        return StaticName();
    }

private:
    void Close();

private:
    void BeginInput();
    BufferLocation m_startLocation = BufferLocation{ 0 };
    ZepWindow& m_launchWindow;
    ZepWindow& m_replWindow;
    ZepRepl* m_pRepl = nullptr;
};

} // namespace Zep
