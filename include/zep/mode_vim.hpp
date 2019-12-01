#pragma once

#include "zep/mode.hpp"
#include "zep/commands.hpp"

class Timer;

namespace Zep
{

struct SpanInfo;

enum class VimMotion
{
    LineBegin,
    LineEnd,
    NonWhiteSpaceBegin,
    NonWhiteSpaceEnd
};

namespace CommandResultFlags
{
enum
{
    None = 0,
    HandledCount = (1 << 2), // Command implements the count, no need to recall it.
    NeedMoreChars
};
} // namespace CommandResultFlags

struct VimSettings
{
    bool ShowNormalModeKeyStrokes = false;
};

struct CommandResult
{
    uint32_t flags = CommandResultFlags::None;
    EditorMode modeSwitch = EditorMode::None;
    std::shared_ptr<ZepCommand> spCommand;
};

enum class CommandOperation
{
    None,
    Delete,
    DeleteLines,
    Insert,
    Copy,
    CopyLines,
    Replace
};

struct CommandContext
{
    CommandContext(const std::string& commandIn, ZepMode_Vim& md, uint32_t lastK, uint32_t modifierK, EditorMode editorMode);

    // Parse the command into:
    // [count1] opA [count2] opB
    // And generate (count1 * count2), opAopB
    void GetCommandAndCount();
    void GetCommandRegisters();
    void UpdateRegisters();

    ZepMode_Vim& owner;

    std::string commandText;
    std::string commandWithoutCount;
    std::string command;

    const SpanInfo* pLineInfo = nullptr;
    ReplaceRangeMode replaceRangeMode = ReplaceRangeMode::Fill;
    BufferLocation beginRange{ -1 };
    BufferLocation endRange{ -1 };
    ZepBuffer& buffer;

    // Cursor State
    BufferLocation bufferCursor{ -1 };
    BufferLocation cursorAfterOverride{ -1 };

    // Register state
    std::stack<char> registers;
    Register tempReg;
    const Register* pRegister = nullptr;

    // Input State
    uint32_t lastKey = 0;
    uint32_t modifierKeys = 0;
    EditorMode mode = EditorMode::None;
    int count = 1;
    bool foundCount = false;

    // Output result
    CommandResult commandResult;
    CommandOperation op = CommandOperation::None;
};

class ZepMode_Vim : public ZepMode
{
public:
    explicit ZepMode_Vim(ZepEditor& editor);
    ~ZepMode_Vim() override;

    void AddKeyPress(uint32_t key, uint32_t modifiers) override;
    void Begin() override;

    static auto StaticName() -> const char*
    {
        return "Vim";
    }
    [[nodiscard]] auto Name() const -> const char* override
    {
        return StaticName();
    }

    [[nodiscard]] auto GetLastCommand() const -> const std::string&
    {
        return m_lastCommand;
    }
    [[nodiscard]] auto GetLastCount() const -> int
    {
        return m_lastCount;
    }

    void PreDisplay() override;

private:
    void ClampCursorForMode();
    void UpdateVisualSelection();
    void HandleInsert(uint32_t key);
    auto GetOperationRange(const std::string& op, EditorMode mode, BufferLocation& beginRange, BufferLocation& endRange) const -> bool;
    void SwitchMode(EditorMode mode);
    void ResetCommand();
    void Init();
    auto GetCommand(CommandContext& context) -> bool;
    auto HandleExCommand(std::string command, char key) -> bool;

    std::string m_currentCommand;
    std::string m_lastCommand;
    int m_lastCount = 0;
    std::string m_lastInsertString;

    std::string m_lastFind;
    SearchDirection m_lastFindDirection = SearchDirection::Forward;

    bool m_pendingEscape = false;
    timer m_insertEscapeTimer;
    VimSettings m_settings;

    BufferLocation m_exCommandStartLocation = 0;
    SearchDirection m_lastSearchDirection = SearchDirection::Forward;
};

} // namespace Zep
