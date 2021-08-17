#ifndef _KEYLOOKUP_
#define _KEYLOOKUP_

#include "Arduino.h"
#define NUM_HID_ENTRIES 111
// Use lookup tables to convert USB HID key codes
// to what Arduino/Teensyduino uses.
uint16_t HID2ArduKEY(const int hidkey);
uint16_t HID2ArduKEY(const int hidkey, const bool shifted);
uint16_t Tnsy2ArduKEY(const int tnsyKey);

void delShort(short *inputArr, uint16_t buffSize, uint16_t index);
void delChar(char *inputString, uint16_t index, uint16_t numChars);
void insChar(char *inputString, uint16_t buffSize, uint16_t index, const char Character);

// Returns the bitmask integer of which modifiers
// are active based on whether their keys were pressed
int getModMask(const int key, const bool press);

// Key values set based on US keyboard layout
// Likely needs tweaks depending on layout

// HID ASCII-ish set for lower-case entries
const uint16_t hidascii_lc[NUM_HID_ENTRIES] = {
    0,
    0,
    0,
    0,
    97,     // a
    98,     // b
    99,     // c
    100,    // d
    101,    // e
    102,    // f
    103,    // g
    104,    // h
    105,    // i
    106,    // j
    107,    // k
    108,    // l
    109,    // m
    110,    // n
    111,    // o
    112,    // p
    113,    // q
    114,    // r
    115,    // s
    116,    // t
    117,    // u
    118,    // v
    119,    // w
    120,    // x
    121,    // y
    122,    // z
    49,     // 1
    50,     // 2
    51,     // 3
    52,     // 4
    53,     // 5
    54,     // 6
    55,     // 7
    56,     // 8
    57,     // 9
    48,     // 0
    KEY_ENTER,
    KEY_ESC,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,
    45,     // -
    61,     // =
    91,     // [
    93,     // ]
    92,     // 
    0,      // "europe"
    59,     // ;
    39,     // '
    96,     // `
    44,     // ,
    46,     // .
    47,     // /
    KEY_CAPS_LOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_PRINTSCREEN,
    KEY_SCROLL_LOCK,
    KEY_PAUSE, // "break"
    KEY_INSERT,
    KEY_HOME,
    KEY_PAGE_UP,
    KEY_DELETE,
    KEY_END,
    KEY_PAGE_DOWN,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_NUM_LOCK,
    KEYPAD_SLASH,
    KEYPAD_ASTERIX,
    KEYPAD_MINUS,
    KEYPAD_PLUS,
    KEYPAD_ENTER,
    KEYPAD_1,
    KEYPAD_2,
    KEYPAD_3,
    KEYPAD_4,
    KEYPAD_5,
    KEYPAD_6,
    KEYPAD_7,
    KEYPAD_8,
    KEYPAD_9,
    KEYPAD_0,
    KEYPAD_PERIOD,
    0,
    0,
    0,
    KEY_LEFT_CTRL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_ALT,
    KEY_LEFT_GUI,
    KEY_RIGHT_CTRL,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_ALT,
    KEY_RIGHT_GUI
};

// HID ASCII-ish set for upper-case entries
const uint16_t hidascii_uc[NUM_HID_ENTRIES] = {
    0,
    0,
    0,
    0,
    65,     // A
    66,     // B
    67,     // C
    68,     // D
    69,     // E
    70,     // F
    71,     // G
    72,     // H
    73,     // I
    74,     // J
    75,     // K
    76,     // L
    77,     // M
    78,     // N
    79,     // O
    80,     // P
    81,     // Q
    82,     // R
    83,     // S
    84,     // T
    85,     // U
    86,     // V
    87,     // W
    88,     // X
    89,     // Y
    90,     // Z
    33,     // !
    64,     // @
    35,     // #
    36,     // $
    37,     // %
    94,     // ^
    38,     // &
    42,     // *
    40,     // (
    41,     // )
    KEY_ENTER,
    KEY_ESC,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,
    95,     // _
    43,     // +
    123,    // {
    125,    // }
    124,    // |
    0,      // "europe"
    58,     // :
    34,     // "
    126,    // ~
    60,     // <
    62,     // >
    63,     // ?
    KEY_CAPS_LOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_PRINTSCREEN,
    KEY_SCROLL_LOCK,
    KEY_PAUSE, // "break"
    KEY_INSERT,
    KEY_HOME,
    KEY_PAGE_UP,
    KEY_DELETE,
    KEY_END,
    KEY_PAGE_DOWN,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_NUM_LOCK,
    KEYPAD_SLASH,
    KEYPAD_ASTERIX,
    KEYPAD_MINUS,
    KEYPAD_PLUS,
    KEYPAD_ENTER,
    KEYPAD_1,
    KEYPAD_2,
    KEYPAD_3,
    KEYPAD_4,
    KEYPAD_5,
    KEYPAD_6,
    KEYPAD_7,
    KEYPAD_8,
    KEYPAD_9,
    KEYPAD_0,
    KEYPAD_PERIOD,
    0,
    0,
    0,
    KEY_LEFT_CTRL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_ALT,
    KEY_LEFT_GUI,
    KEY_RIGHT_CTRL,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_ALT,
    KEY_RIGHT_GUI
};

// Arduino ASCII-ish set for lower-case entries
const uint16_t ardascii_lc[NUM_HID_ENTRIES] = {
    0,
    0,
    0,
    0,
    97,     // a
    98,     // b
    99,     // c
    100,    // d
    101,    // e
    102,    // f
    103,    // g
    104,    // h
    105,    // i
    106,    // j
    107,    // k
    108,    // l
    109,    // m
    110,    // n
    111,    // o
    112,    // p
    113,    // q
    114,    // r
    115,    // s
    116,    // t
    117,    // u
    118,    // v
    119,    // w
    120,    // x
    121,    // y
    122,    // z
    49,     // 1
    50,     // 2
    51,     // 3
    52,     // 4
    53,     // 5
    54,     // 6
    55,     // 7
    56,     // 8
    57,     // 9
    48,     // 0
    176,		// KEY_ENTER,
    177,		// KEY_ESC,
    178,		// KEY_BACKSPACE,
    179,		// KEY_TAB,
    32,			// KEY_SPACE,
    45,     // -	~~ INDEX MATCHES KEYCODE ~~
    61,     // =
    91,     // [
    93,     // ]
    92,     // 
    0,      // "europe"
    59,     // ;
    39,     // '
    96,     // `
    44,     // ,
    46,     // .
    47,     // /
    193,		// KEY_CAPS_LOCK,
    194,		// KEY_F1,
    195,		// KEY_F2,
    196,		// KEY_F3,
    197,		// KEY_F4,
    198,		// KEY_F5,
    199,		// KEY_F6,
    200,		// KEY_F7,
    201,		// KEY_F8,
    202,		// KEY_F9,
    203,		// KEY_F10,
    204,		// KEY_F11,
    205,		// KEY_F12,
    206, //0,	// KEY_PRINTSCREEN,
    0,			// KEY_SCROLL_LOCK,
    0,			// KEY_PAUSE, // "break"
    209,		// KEY_INSERT,
    210,		// KEY_HOME,
    211,		// KEY_PAGE_UP,
    212,		// KEY_DELETE,
    213,		// KEY_END,
    214,		// KEY_PAGE_DOWN,
    215,		// KEY_RIGHT,
    216,		// KEY_LEFT,
    217,		// KEY_DOWN,
    218,		// KEY_UP,
    0,			// KEY_NUM_LOCK,
    47,			// KEYPAD_SLASH,
    42,			// KEYPAD_ASTERIX,
    45,			// KEYPAD_MINUS,
    43,			// KEYPAD_PLUS,
    176,		// KEYPAD_ENTER,
    49,			// KEYPAD_1,
    50,			// KEYPAD_2,
    51,			// KEYPAD_3,
    52,			// KEYPAD_4,
    53,			// KEYPAD_5,
    54,			// KEYPAD_6,
    55,			// KEYPAD_7,
    56,			// KEYPAD_8,
    57,			// KEYPAD_9,
    48,			// KEYPAD_0,
    46,			// KEYPAD_PERIOD,
    0,
    0,
    0,
    128,	// KEY_LEFT_CTRL,
    129,	// KEY_LEFT_SHIFT,
    130,	// KEY_LEFT_ALT,
    131,	// KEY_LEFT_GUI,
    132,	// KEY_RIGHT_CTRL,
    133,	// KEY_RIGHT_SHIFT,
    134,	// KEY_RIGHT_ALT,
    135		// KEY_RIGHT_GUI
};

// HID ASCII-ish set for upper-case entries
const uint16_t ardascii_uc[NUM_HID_ENTRIES] = {
    0,
    0,
    0,
    0,
    65,     // A
    66,     // B
    67,     // C
    68,     // D
    69,     // E
    70,     // F
    71,     // G
    72,     // H
    73,     // I
    74,     // J
    75,     // K
    76,     // L
    77,     // M
    78,     // N
    79,     // O
    80,     // P
    81,     // Q
    82,     // R
    83,     // S
    84,     // T
    85,     // U
    86,     // V
    87,     // W
    88,     // X
    89,     // Y
    90,     // Z
    33,     // !
    64,     // @
    35,     // #
    36,     // $
    37,     // %
    94,     // ^
    38,     // &
    42,     // *
    40,     // (
    41,     // )
    176,		// KEY_ENTER,
    177,		// KEY_ESC,
    178,		// KEY_BACKSPACE,
    179,		// KEY_TAB,
    32,			// KEY_SPACE,
    95,     // _
    43,     // +
    123,    // {
    125,    // }
    124,    // |
    0,      // "europe"
    58,     // :
    34,     // "
    126,    // ~
    60,     // <
    62,     // >
    63,     // ?
    193,		// KEY_CAPS_LOCK,
    194,		// KEY_F1,
    195,		// KEY_F2,
    196,		// KEY_F3,
    197,		// KEY_F4,
    198,		// KEY_F5,
    199,		// KEY_F6,
    200,		// KEY_F7,
    201,		// KEY_F8,
    202,		// KEY_F9,
    203,		// KEY_F10,
    204,		// KEY_F11,
    205,		// KEY_F12,
    206, //0,			// KEY_PRINTSCREEN,
    0,			// KEY_SCROLL_LOCK,
    0,			// KEY_PAUSE, // "break"
    209,		// KEY_INSERT,
    210,		// KEY_HOME,
    211,		// KEY_PAGE_UP,
    212,		// KEY_DELETE,
    213,		// KEY_END,
    214,		// KEY_PAGE_DOWN,
    215,		// KEY_RIGHT,
    216,		// KEY_LEFT,
    217,		// KEY_DOWN,
    218,		// KEY_UP,
    0,			// KEY_NUM_LOCK,
    47,			// KEYPAD_SLASH,
    42,			// KEYPAD_ASTERIX,
    45,			// KEYPAD_MINUS,
    43,			// KEYPAD_PLUS,
    176,		// KEYPAD_ENTER,
    49,			// KEYPAD_1,
    50,			// KEYPAD_2,
    51,			// KEYPAD_3,
    52,			// KEYPAD_4,
    53,			// KEYPAD_5,
    54,			// KEYPAD_6,
    55,			// KEYPAD_7,
    56,			// KEYPAD_8,
    57,			// KEYPAD_9,
    48,			// KEYPAD_0,
    46,			// KEYPAD_PERIOD,
    0,
    0,
    0,
    128,	// KEY_LEFT_CTRL,
    129,	// KEY_LEFT_SHIFT,
    130,	// KEY_LEFT_ALT,
    131,	// KEY_LEFT_GUI,
    132,	// KEY_RIGHT_CTRL,
    133,	// KEY_RIGHT_SHIFT,
    134,	// KEY_RIGHT_ALT,
    135		// KEY_RIGHT_GUI
};

#endif
