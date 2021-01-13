#include "keyLookUp.h"
#include "smStruct.h"

void doTextInputField(
    char*             textBuffer,   // Pointer to the buffer to operate over
    const uint16_t    buffSize,     // >>Size of buffer<< NOT strlen
    uint16_t*         inputCursor,  // text input cursor
    uint16_t*         keyStrokes,   // hid value of the keystroke to be parsed
    const uint8_t     modifiers,    // bitmask of active modifier keys
    const bool        active,       // whether or not to parse keystrokes
    const bool        repeatLast,   // whether or not to repeat last-entered key
    const bool        keyPressed,   // whether or not a key is currently pressed
    const uint16_t    lastReleased  // the most recently released key
    );

bool drawTextInputField(
    const uint16_t    posX,
    const uint16_t    posY,
    const uint16_t    boxW,
    const uint16_t    boxH,
    ILI9341_t3_font_t font,       // 
    const uint8_t     textDatum,
    char*             textBuffer, // Pointer to the buffer to operate over
    const uint16_t    buffSize,   // >>Size of buffer<< NOT strlen
    uint16_t*         inputCursor,// text input cursor
    const bool        active,     // whether or not to parse keystrokes
    stateMachine*     sm
    );
