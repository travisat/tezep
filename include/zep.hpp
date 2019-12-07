#pragma once

#ifdef ZEP_SINGLE_HEADER_BUILD
#include "../src/buffer.cpp"
#include "../src/commands.cpp"
#include "../src/display.cpp"
#include "../src/editor.cpp"
#include "../src/filesystem.cpp"
#include "../src/line_widgets.cpp"
#include "../src/mcommon/animation/timer.cpp"
#include "../src/mcommon/file/path.cpp"
#include "../src/mcommon/string/stringutils.cpp"
#include "../src/mode.cpp"
#include "../src/mode_repl.cpp"
#include "../src/mode_search.cpp"
#include "../src/mode_standard.cpp"
#include "../src/mode_vim.cpp"
#include "../src/scroller.cpp"
#include "../src/splits.cpp"
#include "../src/syntax.cpp"
#include "../src/syntax_providers.cpp"
#include "../src/syntax_rainbow_brackets.cpp"
#include "../src/tab_window.cpp"
#include "../src/theme.cpp"
#include "../src/window.cpp"
#include "zep/imgui/display_imgui.hpp"
#include "zep/imgui/editor_imgui.hpp"

#else
#include "zep/buffer.hpp"
#include "zep/editor.hpp"
#include "zep/imgui/console_imgui.hpp"
#include "zep/imgui/display_imgui.hpp"
#include "zep/imgui/editor_imgui.hpp"
#include "zep/mode.hpp"
#include "zep/mode_standard.hpp"
#include "zep/mode_vim.hpp"
#include "zep/syntax.hpp"
#include "zep/tab_window.hpp"
#include "zep/window.hpp"

#endif