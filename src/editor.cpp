#include "zep/editor.hpp"
#include "zep/buffer.hpp"
#include "zep/display.hpp"
#include "zep/filesystem.hpp"
#include "zep/mode_repl.hpp"
#include "zep/mode_search.hpp"
#include "zep/mode_standard.hpp"
#include "zep/mode_vim.hpp"
#include "zep/syntax.hpp"
#include "zep/syntax_providers.hpp"
#include "zep/tab_window.hpp"
#include "zep/theme.hpp"
#include "zep/window.hpp"

#include "zep/mcommon/animation/timer.hpp"
#include "zep/mcommon/file/cpptoml.hpp"
#include "zep/mcommon/file/path.hpp"
#include "zep/mcommon/logger.hpp"
#include "zep/mcommon/string/murmur_hash.hpp"
#include "zep/mcommon/string/stringutils.hpp"

#include <stdexcept>

namespace Zep
{
structlog LOGCFG = { true, DEBUG };
} // namespace Zep

namespace Zep
{

ZepComponent::ZepComponent(ZepEditor& editor)
    : m_editor(editor)
{
    m_editor.RegisterCallback(this);
}

ZepComponent::~ZepComponent()
{
    m_editor.UnRegisterCallback(this);
}

ZepEditor::ZepEditor(ZepDisplay* pDisplay, const ZepPath& root, uint32_t flags, IZepFileSystem* pFileSystem)
    : m_pDisplay(pDisplay)
    , m_pFileSystem(pFileSystem)
    , m_flags(flags)
    , m_rootPath(root)
{

#if defined(ZEP_FEATURE_CPP_FILE_SYSTEM)
    if (m_pFileSystem == nullptr)
    {
        m_pFileSystem = new ZepFileSystemCPP();
    }
#else
    if (m_pFileSystem == nullptr)
    {
        assert(!"Must supply a file system - no default available on this platform!");
        throw std::invalid_argument("pFileSystem");
    }
#endif

    if ((m_flags & ZepEditorFlags::DisableThreads) != 0)
    {
        m_threadPool = std::make_unique<ThreadPool>(1);
    }
    else
    {
        m_threadPool = std::make_unique<ThreadPool>();
    }

    LoadConfig(root / "zep.cfg");

    m_spTheme = std::make_shared<ZepTheme>();

    assert(m_pDisplay != nullptr);
    RegisterGlobalMode(std::make_shared<ZepMode_Vim>(*this));
    RegisterGlobalMode(std::make_shared<ZepMode_Standard>(*this));
    SetGlobalMode(ZepMode_Vim::StaticName());

    timer_restart(m_cursorTimer);
    timer_restart(m_lastEditTimer);
    m_commandLines.emplace_back("");

    RegisterSyntaxProviders(*this);

    m_editorRegion = std::make_shared<Region>();
    m_editorRegion->vertical = false;

    m_tabRegion = std::make_shared<Region>();
    m_tabContentRegion = std::make_shared<Region>();
    m_commandRegion = std::make_shared<Region>();

    m_editorRegion->children.push_back(m_tabRegion);
    m_editorRegion->children.push_back(m_tabContentRegion);
    m_editorRegion->children.push_back(m_commandRegion);

    Reset();
}

ZepEditor::~ZepEditor()
{
    delete m_pDisplay;
    delete m_pFileSystem;
}

auto ZepEditor::GetThreadPool() const -> ThreadPool&
{
    return *m_threadPool;
}

void ZepEditor::OnFileChanged(const ZepPath& path)
{
    if (path.filename() == "zep.cfg")
    {
        LOG(INFO) << "Reloading config";
        LoadConfig(path);
        Broadcast(std::make_shared<ZepMessage>(Msg::ConfigChanged));
    }
}

// If you pass a valid path to a 'zep.cfg' file, then editor settings will serialize from that
// You can even edit it inside zep for immediate changes :)
void ZepEditor::LoadConfig(const ZepPath& config_path)
{
    if (!GetFileSystem().Exists(config_path))
    {
        return;
    }

    try
    {
        std::shared_ptr<cpptoml::table> spConfig;
        spConfig = cpptoml::parse_file(config_path.string());
        if (spConfig == nullptr)
        {
            return;
        }

        LoadConfig(spConfig);
    }
    catch (cpptoml::parse_exception& ex)
    {
        std::ostringstream str;
        str << config_path.filename().string() << " : Failed to parse. " << ex.what();
        SetCommandText(str.str());
    }
    catch (...)
    {
        std::ostringstream str;
        str << config_path.filename().string() << " : Failed to parse. ";
        SetCommandText(str.str());
    }
}

void ZepEditor::LoadConfig(const std::shared_ptr<cpptoml::table>& spConfig)
{
    try
    {
        m_config.showIndicatorRegion = spConfig->get_qualified_as<bool>("editor.show_indicator_region").value_or(true);
        m_config.showLineNumbers = spConfig->get_qualified_as<bool>("editor.show_line_numbers").value_or(true);
        m_config.autoHideCommandRegion = spConfig->get_qualified_as<bool>("editor.autohide_command_region").value_or(false);
        m_config.cursorLineSolid = spConfig->get_qualified_as<bool>("editor.cursor_line_solid").value_or(true);
        m_config.backgroundFadeTime = (float)spConfig->get_qualified_as<double>("editor.background_fade_time").value_or(60.0F);
        m_config.backgroundFadeWait = (float)spConfig->get_qualified_as<double>("editor.background_fade_wait").value_or(60.0F);
        m_config.showScrollBar = spConfig->get_qualified_as<uint32_t>("editor.show_scrollbar").value_or(1);
        m_config.lineMargins.x = (float)spConfig->get_qualified_as<double>("editor.line_margin_top").value_or(1);
        m_config.lineMargins.y = (float)spConfig->get_qualified_as<double>("editor.line_margin_bottom").value_or(1);
        m_config.widgetMargins.x = (float)spConfig->get_qualified_as<double>("editor.widget_margin_top").value_or(1);
        m_config.widgetMargins.y = (float)spConfig->get_qualified_as<double>("editor.widget_margin_bottom").value_or(1);
        m_config.shortTabNames = spConfig->get_qualified_as<bool>("editor.short_tab_names").value_or(false);
        auto styleStr = string_tolower(spConfig->get_qualified_as<std::string>("editor.style").value_or("normal"));
        if (styleStr == "normal")
        {
            m_config.style = EditorStyle::Normal;
        }
        else if (styleStr == "minimal")
        {
            m_config.style = EditorStyle::Minimal;
        }
    }
    catch (...)
    {
    }
}

void ZepEditor::SaveConfig(const std::shared_ptr<cpptoml::table>& spConfig)
{
    auto table = spConfig->get_table("editor");
    if (!table)
    {
        table = cpptoml::make_table();
        spConfig->insert("editor", table);
    }

    table->insert("show_indicator_region", m_config.showIndicatorRegion);
    table->insert("show_line_numbers", m_config.showLineNumbers);
    table->insert("autohide_command_region", m_config.autoHideCommandRegion);
    table->insert("cursor_line_solid", m_config.cursorLineSolid);
    table->insert("short_tab_names", m_config.shortTabNames);
    table->insert("background_fade_time", (double)m_config.backgroundFadeTime);
    table->insert("background_fade_wait", (double)m_config.backgroundFadeWait);
    table->insert("show_scrollbar", m_config.showScrollBar);

    table->insert("line_margin_top", m_config.lineMargins.x);
    table->insert("line_margin_bottom", m_config.lineMargins.y);
    table->insert("widget_margin_top", m_config.widgetMargins.x);
    table->insert("widget_margin_bottom", m_config.widgetMargins.y);

    table->insert("style", m_config.style == EditorStyle::Minimal ? "minimal" : "normal");

    /*
    Example Write:
    std::ofstream stream("d:/dev/out.txt");
    cpptoml::toml_writer writer(stream, "");
    writer.visit(*spConfig);
    */
}

void ZepEditor::SaveBuffer(ZepBuffer& buffer)
{
    // TODO(unknown):
    // - What if the buffer has no associated file?  Prompt for one.
    // - We don't check for outside modification yet either, meaning this could overwrite
    std::ostringstream strText;

    if (buffer.TestFlags(FileFlags::ReadOnly))
    {
        strText << "Failed to save, Read Only: " << buffer.GetDisplayName();
    }
    else if (buffer.TestFlags(FileFlags::Locked))
    {
        strText << "Failed to save, Locked: " << buffer.GetDisplayName();
    }
    else if (buffer.GetFilePath().empty())
    {
        strText << "Error: No file name";
    }
    else
    {
        int64_t size;
        if (!buffer.Save(size))
        {
            strText << "Failed to save: " << buffer.GetDisplayName() << " at: " << buffer.GetFilePath().string();
        }
        else
        {
            strText << "Wrote " << buffer.GetFilePath().string() << ", " << size << " bytes";
        }
    }
    SetCommandText(strText.str());
}

auto ZepEditor::FindBufferWindows(const ZepBuffer* pBuffer) const -> std::vector<ZepWindow*>
{
    std::vector<ZepWindow*> bufferWindows;
    for (auto& tab : m_tabWindows)
    {
        for (auto& win : tab->GetWindows())
        {
            if (&win->GetBuffer() == pBuffer)
            {
                bufferWindows.push_back(win);
            }
        }
    }
    return bufferWindows;
}

void ZepEditor::RemoveBuffer(ZepBuffer* pBuffer)
{
    auto bufferWindows = FindBufferWindows(pBuffer);
    for (auto& window : bufferWindows)
    {
        window->GetTabWindow().RemoveWindow(window);
    }

    // Find the buffer in the list of buffers owned by the editor and remove it
    auto itr = std::find_if(m_buffers.begin(), m_buffers.end(), [pBuffer](const std::shared_ptr<ZepBuffer>& spBuffer) {
        return spBuffer.get() == pBuffer;
    });

    if (itr != m_buffers.end())
    {
        m_buffers.erase(itr);
    }
}

auto ZepEditor::GetEmptyBuffer(const std::string& name, uint32_t fileFlags) -> ZepBuffer*
{
    auto pBuffer = CreateNewBuffer(name);
    pBuffer->SetFlags(fileFlags, true);
    return pBuffer;
}

auto ZepEditor::GetFileBuffer(const ZepPath& filePath, uint32_t fileFlags, bool create) -> ZepBuffer*
{
    auto path = GetFileSystem().Exists(filePath) ? GetFileSystem().Canonical(filePath) : filePath;
    if (!path.empty())
    {
        for (auto& pBuffer : m_buffers)
        {
            if (!pBuffer->GetFilePath().empty())
            {
                if (GetFileSystem().Equivalent(pBuffer->GetFilePath(), path))
                {
                    //LOG(DEBUG) << "Found equivalent buffer for file: " << path.string();
                    return pBuffer.get();
                }
            }
        }
    }

    if (!create)
    {
        return nullptr;
    }

    // Create buffer, try to load even if not present, the buffer represents the save path (it just isn't saved yet)
    auto pBuffer = CreateNewBuffer(filePath);

    pBuffer->SetFlags(fileFlags, true);
    return pBuffer;
}

// TODO(unknown): Cleaner handling of window/mode/modal stuff.
auto ZepEditor::AddRepl() -> ZepWindow*
{
    if (GetActiveTabWindow() == nullptr)
    {
        return nullptr;
    }

    auto pActiveWindow = GetActiveTabWindow()->GetActiveWindow();

    auto pReplBuffer = GetEmptyBuffer("Repl.lisp", FileFlags::Locked);
    pReplBuffer->SetBufferType(BufferType::Repl);

    auto pReplWindow = GetActiveTabWindow()->AddWindow(pReplBuffer, nullptr, false);

    auto pMode = std::make_shared<ZepMode_Repl>(*this, *pActiveWindow, *pReplWindow);
    pReplBuffer->SetMode(pMode);
    pMode->Begin();
    return pReplWindow;
}

auto ZepEditor::AddSearch() -> ZepWindow*
{
    if (GetActiveTabWindow() == nullptr)
    {
        return nullptr;
    }

    auto pSearchBuffer = GetEmptyBuffer("Search", FileFlags::Locked | FileFlags::ReadOnly);
    pSearchBuffer->SetBufferType(BufferType::Search);

    auto pActiveWindow = GetActiveTabWindow()->GetActiveWindow();
    auto searchPath = GetFileSystem().GetSearchRoot(pActiveWindow->GetBuffer().GetFilePath());

    auto pSearchWindow = GetActiveTabWindow()->AddWindow(pSearchBuffer, nullptr, false);
    pSearchWindow->SetWindowFlags(pSearchWindow->GetWindowFlags() & WindowFlags::Modal);
    pSearchWindow->SetCursorType(CursorType::LineMarker);

    auto pMode = std::make_shared<ZepMode_Search>(*this, *pActiveWindow, *pSearchWindow, searchPath);
    pSearchBuffer->SetMode(pMode);
    pMode->Begin();
    return pSearchWindow;
}

auto ZepEditor::EnsureTab() -> ZepTabWindow*
{
    if (m_tabWindows.empty())
    {
        return AddTabWindow();
    }

    if (m_pActiveTabWindow != nullptr)
    {
        return m_pActiveTabWindow;
    }
    return m_tabWindows[0];
}

// Reset editor to start state; with a single tab, a single window and an empty unmodified buffer
void ZepEditor::Reset()
{
    EnsureTab();
}

// TODO(unknown): fix for directory startup; it won't work
auto ZepEditor::InitWithFileOrDir(const std::string& str) -> ZepBuffer*
{
    ZepPath startPath(str);

    if (GetFileSystem().Exists(startPath))
    {
        startPath = GetFileSystem().Canonical(startPath);
    }

    // If a directory, just return the default already created buffer.
    if (GetFileSystem().IsDirectory(startPath))
    {
        GetFileSystem().SetWorkingDirectory(startPath);
        return &GetActiveTabWindow()->GetActiveWindow()->GetBuffer();
    }

    // Get a buffer for the start file
    auto pFileBuffer = GetFileBuffer(startPath);
    auto pTab = EnsureTab();
    pTab->AddWindow(pFileBuffer, nullptr, false);

    return pFileBuffer;
}

auto ZepEditor::InitWithText(const std::string& strName, const std::string& strText) -> ZepBuffer*
{
    auto pTab = EnsureTab();

    auto pBuffer = GetEmptyBuffer(strName);
    pBuffer->SetText(strText);

    pTab->AddWindow(pBuffer, nullptr, false);

    return pBuffer;
}

// Here we ensure that the editor is in a valid state, and cleanup Default buffers
void ZepEditor::UpdateWindowState()
{
    // If there is no active tab window, and we have one, set it.
    if (m_pActiveTabWindow == nullptr)
    {
        if (!m_tabWindows.empty())
        {
            m_pActiveTabWindow = m_tabWindows.back();
        }
    }

    // If the tab window doesn't contain an active window, and there is one, set it
    if ((m_pActiveTabWindow != nullptr) && (m_pActiveTabWindow->GetActiveWindow() == nullptr))
    {
        if (!m_pActiveTabWindow->GetWindows().empty())
        {
            m_pActiveTabWindow->SetActiveWindow(m_pActiveTabWindow->GetWindows().back());
            m_bRegionsChanged = true;
        }
    }

    // Clean up any default buffers
    std::vector<ZepBuffer*> victims;
    for (auto& buffer : m_buffers)
    {
        if (!buffer->TestFlags(FileFlags::DefaultBuffer) || buffer->TestFlags(FileFlags::Dirty))
        {
            continue;
        }

        auto windows = FindBufferWindows(buffer.get());
        if (windows.empty())
        {
            victims.push_back(buffer.get());
        }
    }

    for (auto& victim : victims)
    {
        RemoveBuffer(victim);
    }

    if (m_pCurrentMode != nullptr)
    {
        m_pCurrentMode->PreDisplay();
    }
}

void ZepEditor::ResetCursorTimer()
{
    timer_restart(m_cursorTimer);
}

void ZepEditor::ResetLastEditTimer()
{
    timer_restart(m_lastEditTimer);
}

auto ZepEditor::GetLastEditElapsedTime() const -> float
{
    return (float)timer_get_elapsed_seconds(m_lastEditTimer);
}

void ZepEditor::NextTabWindow()
{
    auto itr = std::find(m_tabWindows.begin(), m_tabWindows.end(), m_pActiveTabWindow);
    if (itr != m_tabWindows.end())
    {
        itr++;
    }

    if (itr == m_tabWindows.end())
    {
        itr = m_tabWindows.end() - 1;
    }
    m_pActiveTabWindow = *itr;
}

void ZepEditor::PreviousTabWindow()
{
    auto itr = std::find(m_tabWindows.begin(), m_tabWindows.end(), m_pActiveTabWindow);
    if (itr == m_tabWindows.end())
    {
        return;
    }

    if (itr != m_tabWindows.begin())
    {
        itr--;
    }

    m_pActiveTabWindow = *itr;
}

void ZepEditor::SetCurrentTabWindow(ZepTabWindow* pTabWindow)
{
    // Sanity
    auto itr = std::find(m_tabWindows.begin(), m_tabWindows.end(), pTabWindow);
    if (itr != m_tabWindows.end())
    {
        m_pActiveTabWindow = pTabWindow;
    }
}

auto ZepEditor::GetActiveTabWindow() const -> ZepTabWindow*
{
    return m_pActiveTabWindow;
}

auto ZepEditor::AddTabWindow() -> ZepTabWindow*
{
    auto pTabWindow = new ZepTabWindow(*this);
    m_tabWindows.push_back(pTabWindow);
    m_pActiveTabWindow = pTabWindow;

    auto pEmpty = GetEmptyBuffer("[No Name]", FileFlags::DefaultBuffer);
    pTabWindow->AddWindow(pEmpty, nullptr, false);

    return pTabWindow;
}

void ZepEditor::RequestQuit()
{
    Broadcast(std::make_shared<ZepMessage>(Msg::RequestQuit, "RequestQuit"));
}

void ZepEditor::RemoveTabWindow(ZepTabWindow* pTabWindow)
{
    assert(pTabWindow);
    if (pTabWindow == nullptr)
    {
        return;
    }

    auto itrFound = std::find(m_tabWindows.begin(), m_tabWindows.end(), pTabWindow);
    if (itrFound == m_tabWindows.end())
    {
        assert(!"Not found?");
        return;
    }

    delete pTabWindow;
    m_tabWindows.erase(itrFound);
    m_tabRects.clear();

    if (m_tabWindows.empty())
    {
        m_pActiveTabWindow = nullptr;

        // Reset the window state, but request a quit
        Reset();
        RequestQuit();
    }
    else
    {
        if (m_pActiveTabWindow == pTabWindow)
        {
            m_pActiveTabWindow = m_tabWindows[m_tabWindows.size() - 1];
        }
    }
}

auto ZepEditor::GetTabWindows() const -> const ZepEditor::tTabWindows&
{
    return m_tabWindows;
}

void ZepEditor::RegisterGlobalMode(std::shared_ptr<ZepMode> spMode)
{
    m_mapModes[spMode->Name()] = spMode;
}

void ZepEditor::SetGlobalMode(const std::string& mode)
{
    auto itrMode = m_mapModes.find(mode);
    if (itrMode != m_mapModes.end())
    {
        m_pCurrentMode = itrMode->second.get();
        m_pCurrentMode->Begin();
    }
}

auto ZepEditor::GetGlobalMode() -> ZepMode*
{
    // The 'Mode' is typically vim or normal and determines how editing is done in a panel
    if ((m_pCurrentMode == nullptr) && !m_mapModes.empty())
    {
        m_pCurrentMode = m_mapModes.begin()->second.get();
    }

    return m_pCurrentMode;
}

void ZepEditor::SetBufferSyntax(ZepBuffer& buffer) const
{
    std::string ext;
    std::string fileName;
    if (buffer.GetFilePath().has_filename() && buffer.GetFilePath().filename().has_extension())
    {
        ext = string_tolower(buffer.GetFilePath().filename().extension().string());
        fileName = string_tolower(buffer.GetFilePath().filename().string());
    }
    else
    {
        const auto& str = buffer.GetName();
        size_t dot_pos = str.find_last_of('.');
        if (dot_pos != std::string::npos)
        {
            ext = string_tolower(str.substr(dot_pos, str.length() - dot_pos));
        }
    }

    // first check file name
    if (!fileName.empty())
    {
        auto itr = m_mapSyntax.find(fileName);
        if (itr != m_mapSyntax.end())
        {
            buffer.SetSyntaxProvider(itr->second);
            return;
        }
    }

    auto itr = m_mapSyntax.find(ext);
    if (itr != m_mapSyntax.end())
    {
        buffer.SetSyntaxProvider(itr->second);
    }
    else
    {
        itr = m_mapSyntax.find(string_tolower(buffer.GetName()));
        if (itr != m_mapSyntax.end())
        {
            buffer.SetSyntaxProvider(itr->second);
        }
        else
        {
            buffer.SetSyntaxProvider(SyntaxProvider{});
        }
    }
}

void ZepEditor::RegisterSyntaxFactory(const std::vector<std::string>& mappings, const SyntaxProvider& provider)
{
    for (auto& m : mappings)
    {
        m_mapSyntax[string_tolower(m)] = provider;
    }
}

// Inform clients of an event in the buffer
auto ZepEditor::Broadcast(const std::shared_ptr<ZepMessage>& message) -> bool
{
    Notify(message);
    if (message->handled)
    {
        return true;
    }

    for (auto& client : m_notifyClients)
    {
        client->Notify(message);
        if (message->handled)
        {
            break;
        }
    }
    return message->handled;
}

auto ZepEditor::GetBuffers() const -> const std::deque<std::shared_ptr<ZepBuffer>>&
{
    return m_buffers;
}

auto ZepEditor::CreateNewBuffer(const std::string& str) -> ZepBuffer*
{
    auto pBuffer = std::make_shared<ZepBuffer>(*this, str);

    // For a new buffer, set the syntax based on the string name
    SetBufferSyntax(*pBuffer);

    m_buffers.push_front(pBuffer);

    return pBuffer.get();
}

auto ZepEditor::CreateNewBuffer(const ZepPath& path) -> ZepBuffer*
{
    auto pBuffer = std::make_shared<ZepBuffer>(*this, path);
    m_buffers.push_front(pBuffer);
    return pBuffer.get();
}

auto ZepEditor::GetMRUBuffer() const -> ZepBuffer*
{
    return m_buffers.front().get();
}

void ZepEditor::ReadClipboard()
{
    auto pMsg = std::make_shared<ZepMessage>(Msg::GetClipBoard);
    Broadcast(pMsg);
    if (pMsg->handled)
    {
        m_registers["+"] = pMsg->str;
        m_registers["*"] = pMsg->str;
    }
}

void ZepEditor::WriteClipboard()
{
    auto pMsg = std::make_shared<ZepMessage>(Msg::SetClipBoard);
    pMsg->str = m_registers["+"].text;
    Broadcast(pMsg);
}

void ZepEditor::SetRegister(const std::string& reg, const Register& val)
{
    m_registers[reg] = val;
    if (reg == "+" || reg == "*")
    {
        WriteClipboard();
    }
}

void ZepEditor::SetRegister(const char reg, const Register& val)
{
    std::string str({ reg });
    m_registers[str] = val;
    if (reg == '+' || reg == '*')
    {
        WriteClipboard();
    }
}

void ZepEditor::SetRegister(const std::string& reg, const char* pszText)
{
    m_registers[reg] = Register(pszText);
    if (reg == "+" || reg == "*")
    {
        WriteClipboard();
    }
}

void ZepEditor::SetRegister(const char reg, const char* pszText)
{
    std::string str({ reg });
    m_registers[str] = Register(pszText);
    if (reg == '+' || reg == '*')
    {
        WriteClipboard();
    }
}

auto ZepEditor::GetRegister(const std::string& reg) -> Register&
{
    if (reg == "+" || reg == "*")
    {
        ReadClipboard();
    }
    return m_registers[reg];
}

auto ZepEditor::GetRegister(const char reg) -> Register&
{
    if (reg == '+' || reg == '*')
    {
        ReadClipboard();
    }
    std::string str({ reg });
    return m_registers[str];
}
auto ZepEditor::GetRegisters() -> const tRegisters&
{
    ReadClipboard();
    return m_registers;
}

void ZepEditor::Notify(const std::shared_ptr<ZepMessage>& message)
{
    if (message->messageId == Msg::MouseDown)
    {
        for (auto& windowRect : m_tabRects)
        {
            if (windowRect.second.Contains(message->pos))
            {
                SetCurrentTabWindow(windowRect.first);
            }
        }
    }
}

auto ZepEditor::GetCommandText() const -> std::string
{
    std::ostringstream str;
    bool start = true;
    for (auto& line : m_commandLines)
    {
        if (!start)
        {
            str << "\n";
        }
        start = false;
        str << line;
    }
    return str.str();
}

void ZepEditor::SetCommandText(const std::string& strCommand)
{
    m_commandLines = string_split(strCommand, "\n\r");
    if (m_commandLines.empty())
    {
        m_commandLines.emplace_back("");
    }
    m_bRegionsChanged = true;
}

void ZepEditor::RequestRefresh()
{
    m_bPendingRefresh = true;
}

auto ZepEditor::RefreshRequired() -> bool
{
    // Allow any components to update themselves
    Broadcast(std::make_shared<ZepMessage>(Msg::Tick));

    if (m_bPendingRefresh || m_lastCursorBlink != GetCursorBlinkState())
    {
        m_bPendingRefresh = false;
        return true;
    }

    return false;
}

auto ZepEditor::GetCursorBlinkState() const -> bool
{
    m_lastCursorBlink = (int(timer_get_elapsed_seconds(m_cursorTimer) * 1.75F) & 1) != 0;
    return m_lastCursorBlink;
}

void ZepEditor::SetDisplayRegion(const NVec2f& topLeft, const NVec2f& bottomRight)
{
    m_editorRegion->rect.topLeftPx = topLeft;
    m_editorRegion->rect.bottomRightPx = bottomRight;
    UpdateSize();
}

void ZepEditor::UpdateSize()
{
    auto commandCount = GetCommandLines().size();
    const float commandSize = m_pDisplay->GetFontHeightPixels() * commandCount + textBorder * 2.0F;

    // Regions
    m_commandRegion->fixed_size = NVec2f(0.0F, commandSize);
    m_commandRegion->flags = RegionFlags::Fixed;

    // Add tabs for extra windows
    if (GetTabWindows().size() > 1)
    {
        m_tabRegion->fixed_size = NVec2f(0.0F, m_pDisplay->GetFontHeightPixels() + textBorder * 2);
        m_tabRegion->flags = RegionFlags::Fixed;
    }
    else
    {
        m_tabRegion->fixed_size = NVec2f(0.0F, 0.0F);
        m_tabRegion->flags = RegionFlags::Fixed;
    }

    m_tabContentRegion->flags = RegionFlags::Expanding;

    LayoutRegion(*m_editorRegion);

    if (GetActiveTabWindow() != nullptr)
    {
        GetActiveTabWindow()->SetDisplayRegion(m_tabContentRegion->rect);
    }
}

void ZepEditor::Display()
{
    UpdateWindowState();

    if (m_bRegionsChanged)
    {
        m_bRegionsChanged = false;
        UpdateSize();
    }

    // Command plus output
    auto& commandLines = GetCommandLines();
    int32_t commandCount = commandLines.size();
    auto commandSpace = commandCount;
    commandSpace = std::max(commandCount, 0);

    // This fill will effectively fill the region around the tabs in Normal mode
    if (GetConfig().style == EditorStyle::Normal)
    {
        GetDisplay().DrawRectFilled(m_editorRegion->rect, GetTheme().GetColor(ThemeColor::Background));
    }

    // Background rect for CommandLine
    if (!GetCommandText().empty() || (GetConfig().autoHideCommandRegion == false))
    {
        m_pDisplay->DrawRectFilled(m_commandRegion->rect, GetTheme().GetColor(ThemeColor::Background));
    }

    // Draw command text
    auto screenPosYPx = m_commandRegion->rect.topLeftPx + NVec2f(0.0F, textBorder);
    for (int i = 0; i < commandSpace; i++)
    {
        if (!commandLines[i].empty())
        {
            m_pDisplay->DrawChars(screenPosYPx, GetTheme().GetColor(ThemeColor::Text), (const utf8*)commandLines[i].c_str(), nullptr);
        }

        screenPosYPx.y += m_pDisplay->GetFontHeightPixels();
        screenPosYPx.x = m_commandRegion->rect.topLeftPx.x;
    }

    if (GetConfig().style == EditorStyle::Normal)
    {
        // A line along the bottom of the tab region
        m_pDisplay->DrawRectFilled(
            NRectf(NVec2f(m_tabRegion->rect.Left(), m_tabRegion->rect.Bottom() - 1), NVec2f(m_tabRegion->rect.Right(), m_tabRegion->rect.Bottom())), GetTheme().GetColor(ThemeColor::TabInactive));
    }

    m_tabRects.clear();
    if (GetTabWindows().size() > 1)
    {
        // Tab region
        // TODO(unknown): Handle it when tabs are bigger than the available width!
        NVec2f currentTab = m_tabRegion->rect.topLeftPx;
        for (auto& window : GetTabWindows())
        {
            // Show active buffer in tab as tab name
            auto& buffer = window->GetActiveWindow()->GetBuffer();
            std::string name = buffer.GetName();
            if (m_config.shortTabNames)
            {
                auto pos = name.find_last_of('.');
                if (pos != std::string::npos)
                {
                    name = name.substr(0, pos);
                }
            }

            auto tabColor = GetTheme().GetColor(ThemeColor::TabActive);
            if (buffer.TestFlags(FileFlags::HasWarnings))
            {
                tabColor = GetTheme().GetColor(ThemeColor::Warning);
            }
            // Errors win for coloring
            if (buffer.TestFlags(FileFlags::HasErrors))
            {
                tabColor = GetTheme().GetColor(ThemeColor::Error);
            }
            if (window != GetActiveTabWindow())
            {
                // Desaturate unselected ones
                tabColor = tabColor * .55F;
                tabColor.w = 1.0F;
            }
            auto tabLength = m_pDisplay->GetTextSize((const utf8*)name.c_str(), nullptr).x + textBorder * 2;

            // Tab background rect
            NRectf tabRect(currentTab, currentTab + NVec2f(tabLength, m_tabRegion->rect.Height()));
            m_pDisplay->DrawRectFilled(tabRect, tabColor);

            // Tab text
            m_pDisplay->DrawChars(currentTab + NVec2f(textBorder, textBorder), NVec4f(1.0F), (const utf8*)name.c_str(), nullptr);

            currentTab.x += tabLength + textBorder;
            m_tabRects[window] = tabRect;
        }
    }

    // Display the tab
    if (GetActiveTabWindow() != nullptr)
    {
        GetActiveTabWindow()->Display();
    }
}

auto ZepEditor::GetTheme() const -> ZepTheme&
{
    return *m_spTheme;
}

auto ZepEditor::OnMouseMove(const NVec2f& mousePos) -> bool
{
    m_mousePos = mousePos;
    bool handled = Broadcast(std::make_shared<ZepMessage>(Msg::MouseMove, mousePos));
    m_bPendingRefresh = true;
    return handled;
}

auto ZepEditor::OnMouseDown(const NVec2f& mousePos, ZepMouseButton button) -> bool
{
    m_mousePos = mousePos;
    bool handled = Broadcast(std::make_shared<ZepMessage>(Msg::MouseDown, mousePos, button));
    m_bPendingRefresh = true;
    return handled;
}

auto ZepEditor::OnMouseUp(const NVec2f& mousePos, ZepMouseButton button) -> bool
{
    m_mousePos = mousePos;
    bool handled = Broadcast(std::make_shared<ZepMessage>(Msg::MouseUp, mousePos, button));
    m_bPendingRefresh = true;
    return handled;
}

auto ZepEditor::GetMousePos() const -> NVec2f
{
    return m_mousePos;
}

void ZepEditor::SetPixelScale(float pt)
{
    m_pixelScale = pt;
}

auto ZepEditor::GetPixelScale() const -> float
{
    return m_pixelScale;
}

} // namespace Zep
