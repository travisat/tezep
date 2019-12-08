#pragma once

/**
 * USB HID Keyboard scan codes as per USB spec 1.11
 * plus some additional codes
 * 
 * Created by MightyPork, 2016
 * Public domain
 * 
 * Adapted from:
 * https://source.android.com/devices/input/keyboard-devices.html
 */

namespace Keys
{

/**
 * Modifier masks - used for the first byte in the HID report.
 * NOTE: The second byte in the report is reserved, 0x00
 */
constexpr auto Mod_LCtrl = 0x01;
constexpr auto Mod_LShift = 0x02;
constexpr auto Mod_LAlt = 0x04;
constexpr auto Mod_LMeta = 0x08;
constexpr auto Mod_RCtrl = 0x10;
constexpr auto Mod_RShift = 0x20;
constexpr auto Mod_RAlt = 0x40;
constexpr auto Mod_RMeta = 0x80;

/**
 * Scan codes - last N slots in the HID report (usually 6).
 * 0x00 if no key pressed.
 * 
 * If more than N keys are pressed, the HID reports 
 * KEY_ERR_OVF in all slots to indicate this condition.
 */

constexpr auto None = 0x00; // No key pressed
constexpr auto ErrorRollOver = 0x01; //  Keyboard Error Roll Over - used for all slots if too many keys are pressed ("Phantom key")
constexpr auto POSTFail = 0x02; //  Keyboard POST Fail
constexpr auto ErrorUndefined = 0x03; //  Keyboard Error Undefined
constexpr auto Key_A = 0x04; // Keyboard a and A
constexpr auto Key_B = 0x05; // Keyboard b and B
constexpr auto Key_C = 0x06; // Keyboard c and C
constexpr auto Key_D = 0x07; // Keyboard d and D
constexpr auto Key_E = 0x08; // Keyboard e and E
constexpr auto Key_F = 0x09; // Keyboard f and F
constexpr auto Key_G = 0x0a; // Keyboard g and G
constexpr auto Key_H = 0x0b; // Keyboard h and H
constexpr auto Key_I = 0x0c; // Keyboard i and I
constexpr auto Key_J = 0x0d; // Keyboard j and J
constexpr auto Key_K = 0x0e; // Keyboard k and K
constexpr auto Key_L = 0x0f; // Keyboard l and L
constexpr auto Key_M = 0x10; // Keyboard m and M
constexpr auto Key_N = 0x11; // Keyboard n and N
constexpr auto Key_O = 0x12; // Keyboard o and O
constexpr auto Key_P = 0x13; // Keyboard p and P
constexpr auto Key_Q = 0x14; // Keyboard q and Q
constexpr auto Key_R = 0x15; // Keyboard r and R
constexpr auto Key_S = 0x16; // Keyboard s and S
constexpr auto Key_T = 0x17; // Keyboard t and T
constexpr auto Key_U = 0x18; // Keyboard u and U
constexpr auto Key_V = 0x19; // Keyboard v and V
constexpr auto Key_W = 0x1a; // Keyboard w and W
constexpr auto Key_X = 0x1b; // Keyboard x and X
constexpr auto Key_Y = 0x1c; // Keyboard y and Y
constexpr auto Key_Z = 0x1d; // Keyboard z and Z

constexpr auto Key_1 = 0x1e; // Keyboard 1 and !
constexpr auto Key_2 = 0x1f; // Keyboard 2 and @
constexpr auto Key_3 = 0x20; // Keyboard 3 and #
constexpr auto Key_4 = 0x21; // Keyboard 4 and $
constexpr auto Key_5 = 0x22; // Keyboard 5 and %
constexpr auto Key_6 = 0x23; // Keyboard 6 and ^
constexpr auto Key_7 = 0x24; // Keyboard 7 and &
constexpr auto Key_8 = 0x25; // Keyboard 8 and *
constexpr auto Key_9 = 0x26; // Keyboard 9 and (
constexpr auto Key_0 = 0x27; // Keyboard 0 and )

constexpr auto Key_Enter = 0x28; // Keyboard Return (ENTER)
constexpr auto Key_Esc = 0x29; // Keyboard ESCAPE
constexpr auto Key_Backspace = 0x2a; // Keyboard DELETE (Backspace)
constexpr auto Key_Tab = 0x2b; // Keyboard Tab
constexpr auto Key_Space = 0x2c; // Keyboard Spacebar
constexpr auto Key_Minus = 0x2d; // Keyboard - and _
constexpr auto Key_Equal = 0x2e; // Keyboard = and +
constexpr auto Key_LeftBrace = 0x2f; // Keyboard [ and {
constexpr auto Key_RightBrace = 0x30; // Keyboard ] and }
constexpr auto Key_Backslash = 0x31; // Keyboard \ and |
constexpr auto Key_HashTilde = 0x32; // Keyboard Non-US # and ~
constexpr auto Key_Semicolon = 0x33; // Keyboard ; and :
constexpr auto Key_Apostraphe = 0x34; // Keyboard ' and "
constexpr auto Key_Grave = 0x35; // Keyboard ` and ~
constexpr auto Key_Comma = 0x36; // Keyboard , and <
constexpr auto Key_Dot = 0x37; // Keyboard . and >
constexpr auto Key_Slash = 0x38; // Keyboard / and ?
constexpr auto Key_Capslock = 0x39; // Keyboard Caps Lock

constexpr auto Key_F1 = 0x3a; // Keyboard F1
constexpr auto Key_F2 = 0x3b; // Keyboard F2
constexpr auto Key_F3 = 0x3c; // Keyboard F3
constexpr auto Key_F4 = 0x3d; // Keyboard F4
constexpr auto Key_F5 = 0x3e; // Keyboard F5
constexpr auto Key_F6 = 0x3f; // Keyboard F6
constexpr auto Key_F7 = 0x40; // Keyboard F7
constexpr auto Key_F8 = 0x41; // Keyboard F8
constexpr auto Key_F9 = 0x42; // Keyboard F9
constexpr auto Key_F10 = 0x43; // Keyboard F10
constexpr auto Key_F11 = 0x44; // Keyboard F11
constexpr auto Key_F12 = 0x45; // Keyboard F12

constexpr auto Key_SysRQ = 0x46; // Keyboard Print Screen
constexpr auto Key_ScrollLock = 0x47; // Keyboard Scroll Lock
constexpr auto Key_Pause = 0x48; // Keyboard Pause
constexpr auto Key_Insert = 0x49; // Keyboard Insert
constexpr auto Key_Home = 0x4a; // Keyboard Home
constexpr auto Key_PageUp = 0x4b; // Keyboard Page Up
constexpr auto Key_Delete = 0x4c; // Keyboard Delete Forward
constexpr auto Key_End = 0x4d; // Keyboard End
constexpr auto Key_PageDown = 0x4e; // Keyboard Page Down
constexpr auto Key_Right = 0x4f; // Keyboard Right Arrow
constexpr auto Key_Left = 0x50; // Keyboard Left Arrow
constexpr auto Key_Down = 0x51; // Keyboard Down Arrow
constexpr auto Key_Up = 0x52; // Keyboard Up Arrow

constexpr auto Key_NumLock = 0x53; // Keyboard Num Lock and Clear
constexpr auto Keypad_Slash = 0x54; // Keypad /
constexpr auto Keypad_Asterisk = 0x55; // Keypad *
constexpr auto Keypad_Minus = 0x56; // Keypad -
constexpr auto Keypad_Plus = 0x57; // Keypad +
constexpr auto Keypad_Enter = 0x58; // Keypad ENTER
constexpr auto Keypad_1 = 0x59; // Keypad 1 and End
constexpr auto Keypad_2 = 0x5a; // Keypad 2 and Down Arrow
constexpr auto Keypad_3 = 0x5b; // Keypad 3 and PageDn
constexpr auto Keypad_4 = 0x5c; // Keypad 4 and Left Arrow
constexpr auto Keypad_5 = 0x5d; // Keypad 5
constexpr auto Keypad_6 = 0x5e; // Keypad 6 and Right Arrow
constexpr auto Keypad_7 = 0x5f; // Keypad 7 and Home
constexpr auto Keypad_8 = 0x60; // Keypad 8 and Up Arrow
constexpr auto Keypad_9 = 0x61; // Keypad 9 and Page Up
constexpr auto Keypad_0 = 0x62; // Keypad 0 and Insert
constexpr auto Keypad_Dot = 0x63; // Keypad . and Delete
constexpr auto Key_102nd = 0x64; // Keyboard Non-US \ and |
constexpr auto Key_Compose = 0x65; // Keyboard Application
constexpr auto Key_Power = 0x66; // Keyboard Power
constexpr auto Keypad_Equal = 0x67; // Keypad =

constexpr auto Key_F13 = 0x68; // Keyboard F13
constexpr auto Key_F14 = 0x69; // Keyboard F14
constexpr auto Key_F15 = 0x6a; // Keyboard F15
constexpr auto Key_F16 = 0x6b; // Keyboard F16
constexpr auto Key_F17 = 0x6c; // Keyboard F17
constexpr auto Key_F18 = 0x6d; // Keyboard F18
constexpr auto Key_F19 = 0x6e; // Keyboard F19
constexpr auto Key_F20 = 0x6f; // Keyboard F20
constexpr auto Key_F21 = 0x70; // Keyboard F21
constexpr auto Key_F22 = 0x71; // Keyboard F22
constexpr auto Key_F23 = 0x72; // Keyboard F23
constexpr auto Key_F24 = 0x73; // Keyboard F24

constexpr auto Key_Open = 0x74; // Keyboard Execute
constexpr auto Key_Help = 0x75; // Keyboard Help
constexpr auto Key_Props = 0x76; // Keyboard Menu
constexpr auto Key_Front = 0x77; // Keyboard Select
constexpr auto Key_Stop = 0x78; // Keyboard Stop
constexpr auto Key_Again = 0x79; // Keyboard Again
constexpr auto Key_Undo = 0x7a; // Keyboard Undo
constexpr auto Key_Cut = 0x7b; // Keyboard Cut
constexpr auto Key_Copy = 0x7c; // Keyboard Copy
constexpr auto Key_Paste = 0x7d; // Keyboard Paste
constexpr auto Key_Find = 0x7e; // Keyboard Find
constexpr auto Key_Mute = 0x7f; // Keyboard Mute
constexpr auto Key_VolumeUp = 0x80; // Keyboard Volume Up
constexpr auto Key_VolumeDown = 0x81; // Keyboard Volume Down
// 0x82  Keyboard Locking Caps Lock
// 0x83  Keyboard Locking Num Lock
// 0x84  Keyboard Locking Scroll Lock
constexpr auto Keypad_Comma = 0x85; // Keypad Comma
// 0x86  Keypad Equal Sign
constexpr auto Key_Ro = 0x87; // Keyboard International1
constexpr auto Key_KatakanaHiragana = 0x88; // Keyboard International2
constexpr auto Key_Yen = 0x89; // Keyboard International3
constexpr auto Key_Henkan = 0x8a; // Keyboard International4
constexpr auto Key_Muhenkan = 0x8b; // Keyboard International5
constexpr auto Keypad_JPComma = 0x8c; // Keyboard International6
// 0x8d  Keyboard International7
// 0x8e  Keyboard International8
// 0x8f  Keyboard International9
constexpr auto Key_Hangul = 0x90; // Keyboard LANG1
constexpr auto Key_Hanja = 0x91; // Keyboard LANG2
constexpr auto Key_Katakana = 0x92; // Keyboard LANG3
constexpr auto Key_Hiragana = 0x93; // Keyboard LANG4
constexpr auto Key_ZenkakuHankaku = 0x94; // Keyboard LANG5
// 0x95  Keyboard LANG6
// 0x96  Keyboard LANG7
// 0x97  Keyboard LANG8
// 0x98  Keyboard LANG9
// 0x99  Keyboard Alternate Erase
// 0x9a  Keyboard SysReq/Attention
// 0x9b  Keyboard Cancel
// 0x9c  Keyboard Clear
// 0x9d  Keyboard Prior
// 0x9e  Keyboard Return
// 0x9f  Keyboard Separator
// 0xa0  Keyboard Out
// 0xa1  Keyboard Oper
// 0xa2  Keyboard Clear/Again
// 0xa3  Keyboard CrSel/Props
// 0xa4  Keyboard ExSel

// 0xb0  Keypad 00
// 0xb1  Keypad 000
// 0xb2  Thousands Separator
// 0xb3  Decimal Separator
// 0xb4  Currency Unit
// 0xb5  Currency Sub-unit
constexpr auto Keypad_LeftParen = 0xb6; // Keypad (
constexpr auto Keypad_RightParen = 0xb7; // Keypad )
// 0xb8  Keypad {
// 0xb9  Keypad }
// 0xba  Keypad Tab
// 0xbb  Keypad Backspace
// 0xbc  Keypad A
// 0xbd  Keypad B
// 0xbe  Keypad C
// 0xbf  Keypad D
// 0xc0  Keypad E
// 0xc1  Keypad F
// 0xc2  Keypad XOR
// 0xc3  Keypad ^
// 0xc4  Keypad %
// 0xc5  Keypad <
// 0xc6  Keypad >
// 0xc7  Keypad &
// 0xc8  Keypad &&
// 0xc9  Keypad |
// 0xca  Keypad ||
// 0xcb  Keypad :
// 0xcc  Keypad #
// 0xcd  Keypad Space
// 0xce  Keypad @
// 0xcf  Keypad !
// 0xd0  Keypad Memory Store
// 0xd1  Keypad Memory Recall
// 0xd2  Keypad Memory Clear
// 0xd3  Keypad Memory Add
// 0xd4  Keypad Memory Subtract
// 0xd5  Keypad Memory Multiply
// 0xd6  Keypad Memory Divide
// 0xd7  Keypad +/-
// 0xd8  Keypad Clear
// 0xd9  Keypad Clear Entry
// 0xda  Keypad Binary
// 0xdb  Keypad Octal
// 0xdc  Keypad Decimal
// 0xdd  Keypad Hexadecimal

constexpr auto Key_LeftCtrl = 0xe0; // Keyboard Left Control
constexpr auto Key_LeftShift = 0xe1; // Keyboard Left Shift
constexpr auto Key_LeftAlt = 0xe2; // Keyboard Left Alt
constexpr auto Key_LeftMeta = 0xe3; // Keyboard Left GUI
constexpr auto Key_RightCtrl = 0xe4; // Keyboard Right Control
constexpr auto Key_RightShift = 0xe5; // Keyboard Right Shift
constexpr auto Key_RightAlt = 0xe6; // Keyboard Right Alt
constexpr auto Key_RightMeta = 0xe7; // Keyboard Right GUI

constexpr auto Media_PlayPause = 0xe8;
constexpr auto Media_StopCD = 0xe9;
constexpr auto Media_PreviousSong = 0xea;
constexpr auto Media_NextSong = 0xeb;
constexpr auto Media_EjectCD = 0xec;
constexpr auto Media_VolumeUp = 0xed;
constexpr auto Media_VolumeDown = 0xee;
constexpr auto Media_Mute = 0xef;
constexpr auto Media_WWW = 0xf0;
constexpr auto Media_Back = 0xf1;
constexpr auto Media_Forward = 0xf2;
constexpr auto Media_Stop = 0xf3;
constexpr auto Media_Find = 0xf4;
constexpr auto Media_ScrollUp = 0xf5;
constexpr auto Media_ScrollDown = 0xf6;
constexpr auto Media_Edit = 0xf7;
constexpr auto Media_Sleep = 0xf8;
constexpr auto Media_Coffee = 0xf9;
constexpr auto Media_Refresh = 0xfa;
constexpr auto Media_Calc = 0xfb;

} // namespace Keys