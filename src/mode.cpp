#include "zep/mode.hpp"
#include "zep/buffer.hpp"
#include "zep/commands.hpp"
#include "zep/editor.hpp"
#include "zep/filesystem.hpp"
#include "zep/mode_search.hpp"
#include "zep/tab_window.hpp"
#include "zep/window.hpp"

#include "zep/mcommon/logger.hpp"

namespace Zep
{

ZepMode::ZepMode(ZepEditor& editor)
    : ZepComponent(editor)
{
}

ZepMode::~ZepMode() = default;

auto ZepMode::GetCurrentWindow() const -> ZepWindow*
{
    if (GetEditor().GetActiveTabWindow() != nullptr)
    {
        return GetEditor().GetActiveTabWindow()->GetActiveWindow();
    }
    return nullptr;
}

auto ZepMode::GetEditorMode() const -> EditorMode
{
    return m_currentMode;
}

void ZepMode::SetEditorMode(EditorMode mode)
{
    m_currentMode = mode;
}

void ZepMode::AddCommandText(const std::string& strText)
{
    for (auto& ch : strText)
    {
        AddKeyPress(ch, ModifierKey::None);
    }
}

void ZepMode::AddCommand(const std::shared_ptr<ZepCommand>& spCmd)
{
    if ((GetCurrentWindow() != nullptr) && GetCurrentWindow()->GetBuffer().TestFlags(FileFlags::Locked))
    {
        // Ignore commands on buffers because we are view only,
        // and all commands currently modify the buffer!
        return;
    }

    spCmd->Redo();
    m_undoStack.push(spCmd);

    // Can't redo anything beyond this point
    std::stack<std::shared_ptr<ZepCommand>> empty;
    m_redoStack.swap(empty);

    if (spCmd->GetCursorAfter() != -1)
    {
        GetCurrentWindow()->SetBufferCursor(spCmd->GetCursorAfter());
    }
}

void ZepMode::Redo()
{
    bool inGroup = false;
    do
    {
        if (!m_redoStack.empty())
        {
            auto& spCommand = m_redoStack.top();
            spCommand->Redo();

            if ((spCommand->GetFlags() & CommandFlags::GroupBoundary) != 0)
            {
                inGroup = !inGroup;
            }

            if (spCommand->GetCursorAfter() != -1)
            {
                GetCurrentWindow()->SetBufferCursor(spCommand->GetCursorAfter());
            }

            m_undoStack.push(spCommand);
            m_redoStack.pop();
        }
        else
        {
            break;
        }
    } while (inGroup);
}

void ZepMode::Undo()
{
    bool inGroup = false;
    do
    {
        if (!m_undoStack.empty())
        {
            auto& spCommand = m_undoStack.top();
            spCommand->Undo();

            if ((spCommand->GetFlags() & CommandFlags::GroupBoundary) != 0)
            {
                inGroup = !inGroup;
            }

            if (spCommand->GetCursorBefore() != -1)
            {
                GetCurrentWindow()->SetBufferCursor(spCommand->GetCursorBefore());
            }

            m_redoStack.push(spCommand);
            m_undoStack.pop();
        }
        else
        {
            break;
        }
    } while (inGroup);
}

auto ZepMode::GetVisualRange() const -> NVec2i
{
    return NVec2i(m_visualBegin, m_visualEnd);
}

auto ZepMode::HandleGlobalCommand(const std::string& cmd, uint32_t modifiers, bool& needMoreChars) -> bool
{
    if ((modifiers & ModifierKey::Ctrl) != 0)
    {
        return HandleGlobalCtrlCommand(cmd, modifiers, needMoreChars);
    }

    if (cmd[0] == ExtKeys::F8)
    {
        auto pWindow = GetCurrentWindow();
        auto& buffer = pWindow->GetBuffer();
        auto dir = (modifiers & ModifierKey::Shift) != 0 ? SearchDirection::Backward : SearchDirection::Forward;

        auto pFound = buffer.FindNextMarker(GetCurrentWindow()->GetBufferCursor(), dir, RangeMarkerType::Message);
        if (pFound)
        {
            GetCurrentWindow()->SetBufferCursor(pFound->range.first);
        }
        return true;
    }
    return false;
}

auto ZepMode::HandleGlobalCtrlCommand(const std::string& cmd, uint32_t modifiers, bool& needMoreChars) -> bool
{
    needMoreChars = false;

    // TODO(unknown): I prefer 'ko' but I need to put in a keymapper which can see when the user hasn't pressed a second key in a given time
    // otherwise, this hides 'ctrl+k' for pane navigation!
    if (cmd[0] == 'i')
    {
        if (cmd == "i")
        {
            needMoreChars = true;
        }
        else if (cmd == "io")
        {
            // This is a quick and easy 'alternate file swap'.
            // It searches a preset list of useful folder targets around the current file.
            // A better alternative might be a wildcard list of relations, but this works well enough
            // It also only looks for a file with the same name and different extension!
            // it is good enough for my current needs...
            auto& buffer = GetCurrentWindow()->GetBuffer();
            auto path = buffer.GetFilePath();
            if (!path.empty() && GetEditor().GetFileSystem().Exists(path))
            {
                auto ext = path.extension();
                auto searchPaths = std::vector<ZepPath>{
                    path.parent_path(),
                    path.parent_path().parent_path(),
                    path.parent_path().parent_path().parent_path()
                };

                auto ignoreFolders = std::vector<std::string>{ "build", ".git", "obj", "debug", "release" };

                auto priorityFolders = std::vector<std::string>{ "source", "include", "src", "inc", "lib" };

                // Search the paths, starting near and widening
                for (auto& p : searchPaths)
                {
                    if (p.empty())
                    {
                        continue;
                    }

                    bool found = false;

                    // Look for the priority folder locations
                    std::vector<ZepPath> searchFolders{ path.parent_path() };
                    for (auto& priorityFolder : priorityFolders)
                    {
                        GetEditor().GetFileSystem().ScanDirectory(p, [&](const ZepPath& currentPath, bool& recurse) {
                            recurse = false;
                            if (GetEditor().GetFileSystem().IsDirectory(currentPath))
                            {
                                auto lower = string_tolower(currentPath.filename().string());
                                if (std::find(ignoreFolders.begin(), ignoreFolders.end(), lower) != ignoreFolders.end())
                                {
                                    return true;
                                }

                                if (priorityFolder == lower)
                                {
                                    searchFolders.push_back(currentPath);
                                }
                            }
                            return true;
                        });
                    }

                    for (auto& folder : searchFolders)
                    {
                        LOG(INFO) << "Searching: " << folder.string();

                        GetEditor().GetFileSystem().ScanDirectory(folder, [&](const ZepPath& currentPath, bool& recurse) {
                            recurse = true;
                            if (path.stem() == currentPath.stem() && !(currentPath.extension() == path.extension()))
                            {
                                auto load = GetEditor().GetFileBuffer(currentPath, 0, true);
                                if (load != nullptr)
                                {
                                    GetCurrentWindow()->SetBuffer(load);
                                    found = true;
                                    return false;
                                }
                            }
                            return true;
                        });
                        if (found)
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return true;
    }
    if (cmd == "=" || ((cmd == "+") && ((modifiers & ModifierKey::Shift) != 0)))
    {
        GetEditor().GetDisplay().SetFontPointSize(std::min(GetEditor().GetDisplay().GetFontPointSize() + 1.0F, 20.0F));
        return true;
    }
    if (cmd == "-" || ((cmd == "_") && ((modifiers & ModifierKey::Shift) != 0)))
    {
        GetEditor().GetDisplay().SetFontPointSize(std::max(10.0F, GetEditor().GetDisplay().GetFontPointSize() - 1.0F));
        return true;
    }
    // Moving between splits
    if (cmd == "j")
    {
        GetCurrentWindow()->GetTabWindow().DoMotion(WindowMotion::Down);
        return true;
    }
    if (cmd == "k")
    {
        GetCurrentWindow()->GetTabWindow().DoMotion(WindowMotion::Up);
        return true;
    }
    if (cmd == "h")
    {
        GetCurrentWindow()->GetTabWindow().DoMotion(WindowMotion::Left);
        return true;
    }
    if (cmd == "l")
    {
        GetCurrentWindow()->GetTabWindow().DoMotion(WindowMotion::Right);
        return true;
    }
    // global search
    if (cmd == "p" || cmd == ",")
    {
        GetEditor().AddSearch();
        return true;
    }
    if (cmd == "r")
    {
        Redo();
        return true;
    }
    return false;
}

} // namespace Zep
