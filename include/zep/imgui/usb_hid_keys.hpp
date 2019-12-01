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

#ifndef USB_HID_KEYS
#define USB_HID_KEYS

/**
 * Modifier masks - used for the first byte in the HID report.
 * NOTE: The second byte in the report is reserved, 0x00
 */
#define ZKEY_MOD_LCTRL 0x01
#define ZKEY_MOD_LSHIFT 0x02
#define ZKEY_MOD_LALT 0x04
#define ZKEY_MOD_LMETA 0x08
#define ZKEY_MOD_RCTRL 0x10
#define ZKEY_MOD_RSHIFT 0x20
#define ZKEY_MOD_RALT 0x40
#define ZKEY_MOD_RMETA 0x80

/**
 * Scan codes - last N slots in the HID report (usually 6).
 * 0x00 if no key pressed.
 * 
 * If more than N keys are pressed, the HID reports 
 * KEY_ERR_OVF in all slots to indicate this condition.
 */

#define ZKEY_NONE 0x00 // No key pressed
#define ZKEY_ERR_OVF 0x01 //  Keyboard Error Roll Over - used for all slots if too many keys are pressed ("Phantom key")
// 0x02 //  Keyboard POST Fail
// 0x03 //  Keyboard Error Undefined
#define ZKEY_A 0x04 // Keyboard a and A
#define ZKEY_B 0x05 // Keyboard b and B
#define ZKEY_C 0x06 // Keyboard c and C
#define ZKEY_D 0x07 // Keyboard d and D
#define ZKEY_E 0x08 // Keyboard e and E
#define ZKEY_F 0x09 // Keyboard f and F
#define ZKEY_G 0x0a // Keyboard g and G
#define ZKEY_H 0x0b // Keyboard h and H
#define ZKEY_I 0x0c // Keyboard i and I
#define ZKEY_J 0x0d // Keyboard j and J
#define ZKEY_K 0x0e // Keyboard k and K
#define ZKEY_L 0x0f // Keyboard l and L
#define ZKEY_M 0x10 // Keyboard m and M
#define ZKEY_N 0x11 // Keyboard n and N
#define ZKEY_O 0x12 // Keyboard o and O
#define ZKEY_P 0x13 // Keyboard p and P
#define ZKEY_Q 0x14 // Keyboard q and Q
#define ZKEY_R 0x15 // Keyboard r and R
#define ZKEY_S 0x16 // Keyboard s and S
#define ZKEY_T 0x17 // Keyboard t and T
#define ZKEY_U 0x18 // Keyboard u and U
#define ZKEY_V 0x19 // Keyboard v and V
#define ZKEY_W 0x1a // Keyboard w and W
#define ZKEY_X 0x1b // Keyboard x and X
#define ZKEY_Y 0x1c // Keyboard y and Y
#define ZKEY_Z 0x1d // Keyboard z and Z

#define ZKEY_1 0x1e // Keyboard 1 and !
#define ZKEY_2 0x1f // Keyboard 2 and @
#define ZKEY_3 0x20 // Keyboard 3 and #
#define ZKEY_4 0x21 // Keyboard 4 and $
#define ZKEY_5 0x22 // Keyboard 5 and %
#define ZKEY_6 0x23 // Keyboard 6 and ^
#define ZKEY_7 0x24 // Keyboard 7 and &
#define ZKEY_8 0x25 // Keyboard 8 and *
#define ZKEY_9 0x26 // Keyboard 9 and (
#define ZKEY_0 0x27 // Keyboard 0 and )

#define ZKEY_ENTER 0x28 // Keyboard Return (ENTER)
#define ZKEY_ESC 0x29 // Keyboard ESCAPE
#define ZKEY_BACKSPACE 0x2a // Keyboard DELETE (Backspace)
#define ZKEY_TAB 0x2b // Keyboard Tab
#define ZKEY_SPACE 0x2c // Keyboard Spacebar
#define ZKEY_MINUS 0x2d // Keyboard - and _
#define ZKEY_EQUAL 0x2e // Keyboard = and +
#define ZKEY_LEFTBRACE 0x2f // Keyboard [ and {
#define ZKEY_RIGHTBRACE 0x30 // Keyboard ] and }
#define ZKEY_BACKSLASH 0x31 // Keyboard \ and |
#define ZKEY_HASHTILDE 0x32 // Keyboard Non-US # and ~
#define ZKEY_SEMICOLON 0x33 // Keyboard ; and :
#define ZKEY_APOSTROPHE 0x34 // Keyboard ' and "
#define ZKEY_GRAVE 0x35 // Keyboard ` and ~
#define ZKEY_COMMA 0x36 // Keyboard , and <
#define ZKEY_DOT 0x37 // Keyboard . and >
#define ZKEY_SLASH 0x38 // Keyboard / and ?
#define ZKEY_CAPSLOCK 0x39 // Keyboard Caps Lock

#define ZKEY_F1 0x3a // Keyboard F1
#define ZKEY_F2 0x3b // Keyboard F2
#define ZKEY_F3 0x3c // Keyboard F3
#define ZKEY_F4 0x3d // Keyboard F4
#define ZKEY_F5 0x3e // Keyboard F5
#define ZKEY_F6 0x3f // Keyboard F6
#define ZKEY_F7 0x40 // Keyboard F7
#define ZKEY_F8 0x41 // Keyboard F8
#define ZKEY_F9 0x42 // Keyboard F9
#define ZKEY_F10 0x43 // Keyboard F10
#define ZKEY_F11 0x44 // Keyboard F11
#define ZKEY_F12 0x45 // Keyboard F12

#define ZKEY_SYSRQ 0x46 // Keyboard Print Screen
#define ZKEY_SCROLLLOCK 0x47 // Keyboard Scroll Lock
#define ZKEY_PAUSE 0x48 // Keyboard Pause
#define ZKEY_INSERT 0x49 // Keyboard Insert
#define ZKEY_HOME 0x4a // Keyboard Home
#define ZKEY_PAGEUP 0x4b // Keyboard Page Up
#define ZKEY_DELETE 0x4c // Keyboard Delete Forward
#define ZKEY_END 0x4d // Keyboard End
#define ZKEY_PAGEDOWN 0x4e // Keyboard Page Down
#define ZKEY_RIGHT 0x4f // Keyboard Right Arrow
#define ZKEY_LEFT 0x50 // Keyboard Left Arrow
#define ZKEY_DOWN 0x51 // Keyboard Down Arrow
#define ZKEY_UP 0x52 // Keyboard Up Arrow

#define ZKEY_NUMLOCK 0x53 // Keyboard Num Lock and Clear
#define ZKEY_KPSLASH 0x54 // Keypad /
#define ZKEY_KPASTERISK 0x55 // Keypad *
#define ZKEY_KPMINUS 0x56 // Keypad -
#define ZKEY_KPPLUS 0x57 // Keypad +
#define ZKEY_KPENTER 0x58 // Keypad ENTER
#define ZKEY_KP1 0x59 // Keypad 1 and End
#define ZKEY_KP2 0x5a // Keypad 2 and Down Arrow
#define ZKEY_KP3 0x5b // Keypad 3 and PageDn
#define ZKEY_KP4 0x5c // Keypad 4 and Left Arrow
#define ZKEY_KP5 0x5d // Keypad 5
#define ZKEY_KP6 0x5e // Keypad 6 and Right Arrow
#define ZKEY_KP7 0x5f // Keypad 7 and Home
#define ZKEY_KP8 0x60 // Keypad 8 and Up Arrow
#define ZKEY_KP9 0x61 // Keypad 9 and Page Up
#define ZKEY_KP0 0x62 // Keypad 0 and Insert
#define ZKEY_KPDOT 0x63 // Keypad . and Delete

#define ZKEY_102ND 0x64 // Keyboard Non-US \ and |
#define ZKEY_COMPOSE 0x65 // Keyboard Application
#define ZKEY_POWER 0x66 // Keyboard Power
#define ZKEY_KPEQUAL 0x67 // Keypad =

#define ZKEY_F13 0x68 // Keyboard F13
#define ZKEY_F14 0x69 // Keyboard F14
#define ZKEY_F15 0x6a // Keyboard F15
#define ZKEY_F16 0x6b // Keyboard F16
#define ZKEY_F17 0x6c // Keyboard F17
#define ZKEY_F18 0x6d // Keyboard F18
#define ZKEY_F19 0x6e // Keyboard F19
#define ZKEY_F20 0x6f // Keyboard F20
#define ZKEY_F21 0x70 // Keyboard F21
#define ZKEY_F22 0x71 // Keyboard F22
#define ZKEY_F23 0x72 // Keyboard F23
#define ZKEY_F24 0x73 // Keyboard F24

#define ZKEY_OPEN 0x74 // Keyboard Execute
#define ZKEY_HELP 0x75 // Keyboard Help
#define ZKEY_PROPS 0x76 // Keyboard Menu
#define ZKEY_FRONT 0x77 // Keyboard Select
#define ZKEY_STOP 0x78 // Keyboard Stop
#define ZKEY_AGAIN 0x79 // Keyboard Again
#define ZKEY_UNDO 0x7a // Keyboard Undo
#define ZKEY_CUT 0x7b // Keyboard Cut
#define ZKEY_COPY 0x7c // Keyboard Copy
#define ZKEY_PASTE 0x7d // Keyboard Paste
#define ZKEY_FIND 0x7e // Keyboard Find
#define ZKEY_MUTE 0x7f // Keyboard Mute
#define ZKEY_VOLUMEUP 0x80 // Keyboard Volume Up
#define ZKEY_VOLUMEDOWN 0x81 // Keyboard Volume Down
// 0x82  Keyboard Locking Caps Lock
// 0x83  Keyboard Locking Num Lock
// 0x84  Keyboard Locking Scroll Lock
#define ZKEY_KPCOMMA 0x85 // Keypad Comma
// 0x86  Keypad Equal Sign
#define ZKEY_RO 0x87 // Keyboard International1
#define ZKEY_KATAKANAHIRAGANA 0x88 // Keyboard International2
#define ZKEY_YEN 0x89 // Keyboard International3
#define ZKEY_HENKAN 0x8a // Keyboard International4
#define ZKEY_MUHENKAN 0x8b // Keyboard International5
#define ZKEY_KPJPCOMMA 0x8c // Keyboard International6
// 0x8d  Keyboard International7
// 0x8e  Keyboard International8
// 0x8f  Keyboard International9
#define ZKEY_HANGEUL 0x90 // Keyboard LANG1
#define ZKEY_HANJA 0x91 // Keyboard LANG2
#define ZKEY_KATAKANA 0x92 // Keyboard LANG3
#define ZKEY_HIRAGANA 0x93 // Keyboard LANG4
#define ZKEY_ZENKAKUHANKAKU 0x94 // Keyboard LANG5
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
#define ZKEY_KPLEFTPAREN 0xb6 // Keypad (
#define ZKEY_KPRIGHTPAREN 0xb7 // Keypad )
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

#define ZKEY_LEFTCTRL 0xe0 // Keyboard Left Control
#define ZKEY_LEFTSHIFT 0xe1 // Keyboard Left Shift
#define ZKEY_LEFTALT 0xe2 // Keyboard Left Alt
#define ZKEY_LEFTMETA 0xe3 // Keyboard Left GUI
#define ZKEY_RIGHTCTRL 0xe4 // Keyboard Right Control
#define ZKEY_RIGHTSHIFT 0xe5 // Keyboard Right Shift
#define ZKEY_RIGHTALT 0xe6 // Keyboard Right Alt
#define ZKEY_RIGHTMETA 0xe7 // Keyboard Right GUI

#define ZKEY_MEDIA_PLAYPAUSE 0xe8
#define ZKEY_MEDIA_STOPCD 0xe9
#define ZKEY_MEDIA_PREVIOUSSONG 0xea
#define ZKEY_MEDIA_NEXTSONG 0xeb
#define ZKEY_MEDIA_EJECTCD 0xec
#define ZKEY_MEDIA_VOLUMEUP 0xed
#define ZKEY_MEDIA_VOLUMEDOWN 0xee
#define ZKEY_MEDIA_MUTE 0xef
#define ZKEY_MEDIA_WWW 0xf0
#define ZKEY_MEDIA_BACK 0xf1
#define ZKEY_MEDIA_FORWARD 0xf2
#define ZKEY_MEDIA_STOP 0xf3
#define ZKEY_MEDIA_FIND 0xf4
#define ZKEY_MEDIA_SCROLLUP 0xf5
#define ZKEY_MEDIA_SCROLLDOWN 0xf6
#define ZKEY_MEDIA_EDIT 0xf7
#define ZKEY_MEDIA_SLEEP 0xf8
#define ZKEY_MEDIA_COFFEE 0xf9
#define ZKEY_MEDIA_REFRESH 0xfa
#define ZKEY_MEDIA_CALC 0xfb

#endif // USB_HID_KEYS
