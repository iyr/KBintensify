#define NUM_HID_ENTRIES 111
// Key values set based on US keyboard layout
// Likely needs tweaks depending on layout

// Used to determine which look-up table to use
bool SHIFTED = false;

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

// Use lookup tables to convert USB HID key codes
// to what Arduino/Teensyduino uses.
uint16_t HID2ArduKEY(const int hidkey){

  // Return the input key if outside
  // look-up table length
  if (hidkey >= NUM_HID_ENTRIES) return hidkey;

  // Determine which look-up table to use, return
  if (SHIFTED) {
    return hidascii_uc[hidkey];
  } else {
    return hidascii_lc[hidkey];
  }
}

// Returns the bitmask integer of which modifiers
// are active based on whether their keys were pressed
int setMods(const int key, const bool press) {

  // Used to track whether a modifier bitmask
  // should be set
  static bool LCTRL  = false;
  static bool LSHIFT = false;
  static bool LALT   = false;
  static bool LGUI   = false;
  static bool RCTRL  = false;
  static bool RSHIFT = false;
  static bool RALT   = false;
  static bool RGUI   = false;

  // Set global modifier bools
  const int ArduKey = HID2ArduKEY(key);
  switch (ArduKey) {
    case KEY_LEFT_CTRL:   LCTRL   = press; break;
    case KEY_LEFT_SHIFT:  LSHIFT  = press; break;
    case KEY_LEFT_ALT:    LALT    = press; break;
    case KEY_LEFT_GUI:    LGUI    = press; break;
    case KEY_RIGHT_CTRL:  RCTRL   = press; break;
    case KEY_RIGHT_SHIFT: RSHIFT  = press; break;
    case KEY_RIGHT_ALT:   RALT    = press; break;
    case KEY_RIGHT_GUI:   RGUI    = press; break;
  }

  // Set shifted bool
  SHIFTED = (LSHIFT || RSHIFT);

  // Build modifier int from bitmasks
  int mods = 0;
  if (LCTRL)  mods = (mods | MODIFIERKEY_CTRL       );
  if (LSHIFT) mods = (mods | MODIFIERKEY_SHIFT      );
  if (LALT)   mods = (mods | MODIFIERKEY_ALT        );
  if (LGUI)   mods = (mods | MODIFIERKEY_GUI        );
  if (RCTRL)  mods = (mods | MODIFIERKEY_RIGHT_CTRL );
  if (RSHIFT) mods = (mods | MODIFIERKEY_RIGHT_SHIFT);
  if (RALT)   mods = (mods | MODIFIERKEY_RIGHT_ALT  );
  if (RGUI)   mods = (mods | MODIFIERKEY_RIGHT_GUI  );

  return mods;
}

