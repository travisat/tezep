#pragma once
#include <string>

#include "zep/imgui/display_imgui.hpp"
#include "zep/imgui/usb_hid_keys.hpp"

#include "zep/editor.hpp"
#include "zep/mode_standard.hpp"
#include "zep/mode_vim.hpp"
#include "zep/syntax.hpp"
#include "zep/tab_window.hpp"
#include "zep/window.hpp"

namespace Zep
{

class ZepDisplay_ImGui;
class ZepTabWindow;
class ZepEditor_ImGui : public ZepEditor
{
public:
    explicit ZepEditor_ImGui(const ZepPath& root, uint32_t flags = 0, IZepFileSystem* pFileSystem = nullptr)
        : ZepEditor(new ZepDisplay_ImGui(), root, flags, pFileSystem)
    {
    }

    void HandleInput()
    {
        auto& io = ImGui::GetIO();

        bool handled = false;

        uint32_t mod = 0;

        static std::map<int, int> MapUSBKeys = {
            { ZKEY_F1, ExtKeys::F1 },
            { ZKEY_F2, ExtKeys::F2 },
            { ZKEY_F3, ExtKeys::F3 },
            { ZKEY_F4, ExtKeys::F4 },
            { ZKEY_F5, ExtKeys::F5 },
            { ZKEY_F6, ExtKeys::F6 },
            { ZKEY_F7, ExtKeys::F7 },
            { ZKEY_F8, ExtKeys::F8 },
            { ZKEY_F9, ExtKeys::F9 },
            { ZKEY_F10, ExtKeys::F10 },
            { ZKEY_F11, ExtKeys::F11 },
            { ZKEY_F12, ExtKeys::F12 }
        };
        if (io.MouseDelta.x != 0 || io.MouseDelta.y != 0)
        {
            OnMouseMove(toNVec2f(io.MousePos));
        }

        if (io.MouseClicked[0])
        {
            if (OnMouseDown(toNVec2f(io.MousePos), ZepMouseButton::Left))
            {
                // Hide the mouse click from imgui if we handled it
                io.MouseClicked[0] = false;
            }
        }

        if (io.MouseClicked[1])
        {
            if (OnMouseDown(toNVec2f(io.MousePos), ZepMouseButton::Right))
            {
                // Hide the mouse click from imgui if we handled it
                io.MouseClicked[0] = false;
            }
        }

        if (io.MouseReleased[0])
        {
            if (OnMouseUp(toNVec2f(io.MousePos), ZepMouseButton::Left))
            {
                // Hide the mouse click from imgui if we handled it
                io.MouseClicked[0] = false;
            }
        }

        if (io.MouseReleased[1])
        {
            if (OnMouseUp(toNVec2f(io.MousePos), ZepMouseButton::Right))
            {
                // Hide the mouse click from imgui if we handled it
                io.MouseClicked[0] = false;
            }
        }

        if (io.KeyCtrl)
        {
            mod |= ModifierKey::Ctrl;
        }
        if (io.KeyShift)
        {
            mod |= ModifierKey::Shift;
        }

        auto pWindow = GetActiveTabWindow()->GetActiveWindow();
        const auto& buffer = pWindow->GetBuffer();

        // Check USB Keys
        for (auto& usbKey : MapUSBKeys)
        {
            if (ImGui::IsKeyPressed(usbKey.first))
            {
                buffer.GetMode()->AddKeyPress(usbKey.second, mod);
                return;
            }
        }

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::TAB, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::ESCAPE, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::RETURN, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::DEL, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::HOME, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::END, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::BACKSPACE, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::RIGHT, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::LEFT, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::UP, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::DOWN, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_PageDown)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::PAGEDOWN, mod);
            return;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_PageUp)))
        {
            buffer.GetMode()->AddKeyPress(ExtKeys::PAGEUP, mod);
            return;
        }
        if (io.KeyCtrl)
        {
            // SDL Remaps to its own scancodes; and since we can't look them up in the standard IMGui list
            // without modifying the ImGui base code, we have special handling here for CTRL.
            // For the Win32 case, we use VK_A (ASCII) is handled below
#if defined(_SDL_H) || defined(ZEP_USE_SDL)
            if (ImGui::IsKeyPressed(ZKEY_1))
            {
                SetGlobalMode(ZepMode_Standard::StaticName());
                handled = true;
            }
            else if (ImGui::IsKeyPressed(ZKEY_2))
            {
                SetGlobalMode(ZepMode_Vim::StaticName());
                handled = true;
            }
            else
            {
                for (int ch = ZKEY_A; ch <= ZKEY_Z; ch++)
                {
                    if (ImGui::IsKeyPressed(ch))
                    {
                        buffer.GetMode()->AddKeyPress((ch - ZKEY_A) + 'a', mod);
                        handled = true;
                    }
                }

                if (ImGui::IsKeyPressed(ZKEY_SPACE))
                {
                    buffer.GetMode()->AddKeyPress(' ', mod);
                    handled = true;
                }
            }
#else
            if (ImGui::IsKeyPressed('1'))
            {
                SetGlobalMode(ZepMode_Standard::StaticName());
                handled = true;
            }
            else if (ImGui::IsKeyPressed('2'))
            {
                SetGlobalMode(ZepMode_Vim::StaticName());
                handled = true;
            }
            else
            {
                for (int ch = 'A'; ch <= 'Z'; ch++)
                {
                    if (ImGui::IsKeyPressed(ch))
                    {
                        buffer.GetMode()->AddKeyPress(ch - 'A' + 'a', mod);
                        handled = true;
                    }
                }

                if (ImGui::IsKeyPressed(ZKEY_SPACE))
                {
                    buffer.GetMode()->AddKeyPress(' ', mod);
                    handled = true;
                }
            }
#endif
        }

        if (!handled)
        {
            for (int n = 0; n < io.InputQueueCharacters.Size && (io.InputQueueCharacters[n] != 0); n++)
            {
                // Ignore '\r' - sometimes ImGui generates it!
                if (io.InputQueueCharacters[n] == '\r')
                {
                    continue;
                }

                buffer.GetMode()->AddKeyPress(io.InputQueueCharacters[n], mod);
            }
        }
    }

private:
};

} // namespace Zep
