#pragma once

#include <deque>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "zep/zep_config.hpp"

#include "zep/mcommon/animation/timer.hpp"
#include "zep/mcommon/file/cpptoml.hpp"
#include "zep/mcommon/file/path.hpp"
#include "zep/mcommon/math/math.hpp"
#include "zep/mcommon/threadpool.hpp"

#include "splits.hpp"

// Basic Architecture

// Editor
//      Buffers
//      Modes -> (Active BufferRegion)
// Display
//      BufferRegions (->Buffers)
//
// A buffer is just an array of chars in a gap buffer, with simple operations to insert, delete and search
// A display is something that can display a collection of regions and the editor controls in a window
// A buffer region is a single view onto a buffer inside the main display
//
// The editor has a list of ZepBuffers.
// The editor has different editor modes (vim/standard)
// ZepDisplay can render the editor (with imgui or something else)
// The display has multiple BufferRegions, each a window onto a buffer.
// Multiple regions can refer to the same buffer (N Regions : N Buffers)
// The Modes receive key presses and act on a buffer region
namespace Zep
{

class ZepBuffer;
class ZepMode;
class ZepMode_Vim;
class ZepMode_Standard;
class ZepEditor;
class ZepSyntax;
class ZepTabWindow;
class ZepWindow;
class ZepTheme;

class ZepDisplay;
class IZepFileSystem;

struct Region;

using utf8 = uint8_t;

namespace ZepEditorFlags
{
enum
{
    None = (0),
    DisableThreads = (1 << 0),
};
} // namespace ZepEditorFlags

enum class ZepMouseButton
{
    Left,
    Middle,
    Right,
    Unknown
};

enum class Msg
{
    HandleCommand,
    RequestQuit,
    GetClipBoard,
    SetClipBoard,
    MouseMove,
    MouseDown,
    MouseUp,
    Buffer,
    ComponentChanged,
    Tick,
    ConfigChanged,
    ToolTip
};

struct IZepComponent;
class ZepMessage
{
public:
    explicit ZepMessage(Msg id, std::string strIn = std::string())
        : messageId(id)
        , str(std::move(strIn))
    {
    }

    ZepMessage(Msg id, const NVec2f& p, ZepMouseButton b = ZepMouseButton::Unknown)
        : messageId(id)
        , pos(p)
        , button(b)
    {
    }

    ZepMessage(Msg id, IZepComponent* pComp)
        : messageId(id)
        , pComponent(pComp)
    {
    }

    Msg messageId; // Message ID
    std::string str; // Generic string for simple messages
    bool handled = false; // If the message was handled
    NVec2f pos;
    ZepMouseButton button = ZepMouseButton::Unknown;
    IZepComponent* pComponent = nullptr;
};

struct IZepComponent
{
    virtual void Notify(std::shared_ptr<ZepMessage> message) = 0;
    [[nodiscard]] virtual auto GetEditor() const -> ZepEditor& = 0;
};

class ZepComponent : public IZepComponent
{
public:
    explicit ZepComponent(ZepEditor& editor);
    virtual ~ZepComponent();
    [[nodiscard]] auto GetEditor() const -> ZepEditor& override
    {
        return m_editor;
    }

private:
    ZepEditor& m_editor;
};

// Registers are used by the editor to store/retrieve text fragments
struct Register
{
    Register()
        : text("")
    {
    }
    Register(const char* ch, bool lw = false)
        : text(ch)
        , lineWise(lw)
    {
    }
    Register(utf8* ch, bool lw = false)
        : text(reinterpret_cast<const char*>(ch))
        , lineWise(lw)
    {
    }
    Register(std::string str, bool lw = false)
        : text(std::move(str))
        , lineWise(lw)
    {
    }

    std::string text;
    bool lineWise = false;
};

using tRegisters = std::map<std::string, Register>;
using tBuffers = std::deque<std::shared_ptr<ZepBuffer>>;
using tSyntaxFactory = std::function<std::shared_ptr<ZepSyntax>(ZepBuffer*)>;

struct SyntaxProvider
{
    std::string syntaxID;
    tSyntaxFactory factory = nullptr;
};

const float bottomBorder = 4.F;
const float textBorder = 4.F;
const float leftBorderChars = 3;

#define DPI_VEC2(value) ((value)*GetEditor().GetPixelScale())
#define DPI_Y(value) (GetEditor().GetPixelScale() * (value))
#define DPI_X(value) (GetEditor().GetPixelScale() * (value))
#define DPI_RECT(value) ((value)*GetEditor().GetPixelScale())

enum class EditorStyle
{
    Normal = 0,
    Minimal
};

struct EditorConfig
{
    uint32_t showScrollBar = 1;
    EditorStyle style = EditorStyle::Normal;
    NVec2f lineMargins = NVec2f(1.0F);
    NVec2f widgetMargins = NVec2f(1.0F);
    bool showLineNumbers = true;
    bool shortTabNames = true;
    bool showIndicatorRegion = true;
    bool autoHideCommandRegion = true;
    bool cursorLineSolid = false;
    float backgroundFadeTime = 60.0F;
    float backgroundFadeWait = 60.0F;
};

class ZepEditor
{
public:
    // Root path is the path to search for a config file
    ZepEditor(ZepDisplay* pDisplay, const ZepPath& root, uint32_t flags = 0, IZepFileSystem* pFileSystem = nullptr);
    ~ZepEditor();

    void LoadConfig(const ZepPath& config_path);
    void LoadConfig(std::shared_ptr<cpptoml::table> spConfig);
    void SaveConfig(std::shared_ptr<cpptoml::table> spConfig);
    void RequestQuit();

    void Reset();
    auto InitWithFileOrDir(const std::string& str) -> ZepBuffer*;
    auto InitWithText(const std::string& strName, const std::string& strText) -> ZepBuffer*;

    auto GetGlobalMode() -> ZepMode*;
    void RegisterGlobalMode(std::shared_ptr<ZepMode> spMode);
    void SetGlobalMode(const std::string& mode);
    auto GetSecondaryMode() const -> ZepMode*;

    void Display();

    void RegisterSyntaxFactory(const std::vector<std::string>& mappings, const SyntaxProvider& factory);
    auto Broadcast(const std::shared_ptr<ZepMessage>& payload) -> bool;
    void RegisterCallback(IZepComponent* pClient)
    {
        m_notifyClients.insert(pClient);
    }
    void UnRegisterCallback(IZepComponent* pClient)
    {
        m_notifyClients.erase(pClient);
    }

    auto GetBuffers() const -> const tBuffers&;
    auto GetMRUBuffer() const -> ZepBuffer*;
    void SaveBuffer(ZepBuffer& buffer);
    auto GetFileBuffer(const ZepPath& filePath, uint32_t fileFlags = 0, bool create = true) -> ZepBuffer*;
    auto GetEmptyBuffer(const std::string& name, uint32_t fileFlags = 0) -> ZepBuffer*;
    void RemoveBuffer(ZepBuffer* pBuffer);
    auto FindBufferWindows(const ZepBuffer* pBuffer) const -> std::vector<ZepWindow*>;

    void SetRegister(const std::string& reg, const Register& val);
    void SetRegister(char reg, const Register& val);
    void SetRegister(const std::string& reg, const char* pszText);
    void SetRegister(char reg, const char* pszText);
    auto GetRegister(const std::string& reg) -> Register&;
    auto GetRegister(char reg) -> Register&;
    auto GetRegisters() -> const tRegisters&;

    void ReadClipboard();
    void WriteClipboard();

    void Notify(const std::shared_ptr<ZepMessage>& message);
    auto GetFlags() const -> uint32_t
    {
        return m_flags;
    }

    // Tab windows
    using tTabWindows = std::vector<ZepTabWindow*>;
    void NextTabWindow();
    void PreviousTabWindow();
    void SetCurrentTabWindow(ZepTabWindow* pTabWindow);
    auto GetActiveTabWindow() const -> ZepTabWindow*;
    auto AddTabWindow() -> ZepTabWindow*;
    void RemoveTabWindow(ZepTabWindow* pTabWindow);
    auto GetTabWindows() const -> const tTabWindows&;

    auto AddRepl() -> ZepWindow*;
    auto AddSearch() -> ZepWindow*;

    void ResetCursorTimer();
    auto GetCursorBlinkState() const -> bool;

    void ResetLastEditTimer();
    auto GetLastEditElapsedTime() const -> float;

    void RequestRefresh();
    auto RefreshRequired() -> bool;

    void SetCommandText(const std::string& strCommand);

    auto GetCommandText() const -> std::string;
    auto GetCommandLines() -> const std::vector<std::string>&
    {
        return m_commandLines;
    }

    void UpdateWindowState();

    // Setup the display fixed_size for the editor
    void SetDisplayRegion(const NVec2f& topLeft, const NVec2f& bottomRight);
    void UpdateSize();

    auto GetDisplay() const -> ZepDisplay&
    {
        return *m_pDisplay;
    }

    auto GetFileSystem() const -> IZepFileSystem&
    {
        return *m_pFileSystem;
    }

    auto GetTheme() const -> ZepTheme&;

    auto OnMouseMove(const NVec2f& mousePos) -> bool;
    auto OnMouseDown(const NVec2f& mousePos, ZepMouseButton button) -> bool;
    auto OnMouseUp(const NVec2f& mousePos, ZepMouseButton button) -> bool;
    auto GetMousePos() const -> NVec2f;

    void SetPixelScale(float pt);
    auto GetPixelScale() const -> float;

    void SetBufferSyntax(ZepBuffer& buffer) const;

    auto GetConfig() const -> EditorConfig
    {
        return m_config;
    }

    auto GetThreadPool() const -> ThreadPool&;

    // Used to inform when a file changes - called from outside zep by the platform specific code, if possible
    virtual void OnFileChanged(const ZepPath& path);

private:
    // Call GetBuffer publicly, to stop creation of duplicate buffers refering to the same file
    auto CreateNewBuffer(const std::string& bufferName) -> ZepBuffer*;
    auto CreateNewBuffer(const ZepPath& path) -> ZepBuffer*;

    // Ensure there is a valid tab window and return it
    auto EnsureTab() -> ZepTabWindow*;

private:
    ZepDisplay* m_pDisplay;
    IZepFileSystem* m_pFileSystem;

    std::set<IZepComponent*> m_notifyClients;
    mutable tRegisters m_registers;

    std::shared_ptr<ZepTheme> m_spTheme;
    std::shared_ptr<ZepMode_Vim> m_spVimMode;
    std::shared_ptr<ZepMode_Standard> m_spStandardMode;
    std::map<std::string, SyntaxProvider> m_mapSyntax;
    std::map<std::string, std::shared_ptr<ZepMode>> m_mapModes;

    // Blinking cursor
    timer m_cursorTimer;

    // Last edit
    timer m_lastEditTimer;

    // Active mode
    ZepMode* m_pCurrentMode = nullptr;

    // Tab windows
    tTabWindows m_tabWindows;
    ZepTabWindow* m_pActiveTabWindow = nullptr;

    // List of buffers that the editor is managing
    // May or may not be visible
    tBuffers m_buffers;
    uint32_t m_flags = 0;

    mutable bool m_bPendingRefresh = true;
    mutable bool m_lastCursorBlink = false;

    std::vector<std::string> m_commandLines; // Command information, shown under the buffer

    std::shared_ptr<Region> m_editorRegion;
    std::shared_ptr<Region> m_tabContentRegion;
    std::shared_ptr<Region> m_commandRegion;
    std::shared_ptr<Region> m_tabRegion;
    std::map<ZepTabWindow*, NRectf> m_tabRects;
    bool m_bRegionsChanged = false;

    NVec2f m_mousePos;
    float m_pixelScale = 1.0F;
    ZepPath m_rootPath;

    // Config
    EditorConfig m_config;

    std::unique_ptr<ThreadPool> m_threadPool;
};

} // namespace Zep
