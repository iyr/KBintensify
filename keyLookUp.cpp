#include "keyLookUp.h"

// Used to determine which look-up table to use
static bool SHIFTED = false;

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
uint16_t Tnsy2ArduKEY(const int tnsyKey){

  // Return the input key if outside
  // look-up table length
  //if (tnsyKey >= NUM_HID_ENTRIES) return tnsyKey;

  // Determine which look-up table to use, return
  if (SHIFTED) {
    return ardascii_uc[tnsyKey];
  } else {
    return ardascii_lc[tnsyKey];
  }
}
// Use lookup tables to convert USB HID key codes
// to what Arduino/Teensyduino uses.
uint16_t HID2ArduKEY(const int hidkey, const bool shifted){

  // Return the input key if outside
  // look-up table length
  if (hidkey >= NUM_HID_ENTRIES) return hidkey;

  // Determine which look-up table to use, return
  if (shifted) {
    return hidascii_uc[hidkey];
  } else {
    return hidascii_lc[hidkey];
  }
}

void insChar(char *inputString, uint16_t buffSize, uint16_t index, const char Character){
  // Avoid buffer overrun
  if (buffSize-1 == index ||
      buffSize-1 == strlen(inputString)) return;
  memmove(inputString+1+index, inputString+index, strlen(inputString)-index);
  inputString[index] = Character;
  return;
}

void delChar(char *inputString, uint16_t index, uint16_t numChars){
  if ((numChars+index-1) <= strlen(inputString))
    strcpy(&inputString[index-1],&inputString[numChars+index-1]);
  return;
}

void delShort(short *inputArr, uint16_t buffSize, uint16_t index){
  //for (uint16_t i = index; i < buffSize - 1; i++) inputArr[i] = inputArr[i+1];
  for (uint16_t i = index; i < buffSize-1; i++) inputArr[i] = inputArr[i+1];
  return;
}


// Returns the bitmask integer of which modifiers
// are active based on whether their keys were pressed
int getModMask(const int key, const bool press) {

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
