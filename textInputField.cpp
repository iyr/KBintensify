#include "textInputField.h"

// text field input helper for processing individual keystroke behavior
void parseKey(
    char*             textBuffer, // Pointer to the buffer to operate over
    const uint16_t    buffSize,   // >>Size of buffer<< NOT strlen
    uint16_t*         inputCursor,// text input cursor
    uint16_t          keyStroke,  // hid value of the keystroke to be parsed
    uint8_t           modifiers   // bitmask of enabled modifiers
    ){

  int16_t deltaPos  = 0;      // How much to move cursor
  bool    incCursor = false,  // whether or not to increment cursor
          decCursor = false;  // whether or not to decrement cursor

  // (mostly) contiguous and in-order cases to aid compiler optimization
  switch(HID2ArduKEY(keyStroke)) {
    case 0:
      break;
    case KEY_ENTER:
      break;
    case KEY_ESC:
      break;
    case KEY_BACKSPACE:
      if ((* inputCursor) > 0) delChar(textBuffer, (* inputCursor), 1);
      decCursor = true;
      break;
    case KEY_TAB:
      break;
    case KEY_SPACE:
      insChar(textBuffer, buffSize, (* inputCursor), ' ');
      incCursor = true;
      break;
    case KEY_CAPS_LOCK:
      break;
    case KEY_F1:
      break;
    case KEY_F2:
      break;
    case KEY_F3:
      break;
    case KEY_F4:
      break;
    case KEY_F5:
      break;
    case KEY_F6:
      break;
    case KEY_F7:
      break;
    case KEY_F8:
      break;
    case KEY_F9:
      break;
    case KEY_F10:
      break;
    case KEY_F11:
      break;
    case KEY_F12:
      break;
    case KEY_PRINTSCREEN:
      break;
    case KEY_SCROLL_LOCK:
      break;
    case KEY_PAUSE: // "break:
      break;
    case KEY_INSERT:
      break;
    case KEY_HOME:
      (* inputCursor) = 0;
      break;
    case KEY_PAGE_UP:
      break;
    case KEY_DELETE:
      delChar(textBuffer, (* inputCursor)+1, 1);
      break;
    case KEY_END:
      (* inputCursor) = min(strlen(textBuffer), buffSize);
      break;
    case KEY_PAGE_DOWN:
      break;
    case KEY_RIGHT:
      incCursor = true;
      break;
    case KEY_LEFT:
      decCursor = true;
      break;
    case KEY_DOWN:
      (* inputCursor) = min(strlen(textBuffer), buffSize);
      break;
    case KEY_UP:
      (* inputCursor) = 0;
      break;
    case KEY_NUM_LOCK:
      break;
    case KEYPAD_SLASH:
      insChar(textBuffer, buffSize, (* inputCursor), '/');
      incCursor = true;
      break;
    case KEYPAD_ASTERIX:
      insChar(textBuffer, buffSize, (* inputCursor), '*');
      incCursor = true;
      break;
    case KEYPAD_MINUS:
      insChar(textBuffer, buffSize, (* inputCursor), '-');
      incCursor = true;
      break;
    case KEYPAD_PLUS:
      insChar(textBuffer, buffSize, (* inputCursor), '+');
      incCursor = true;
      break;
    case KEYPAD_ENTER:
      break;
    case KEYPAD_1:
      insChar(textBuffer, buffSize, (* inputCursor), '1');
      incCursor = true;
      break;
    case KEYPAD_2:
      insChar(textBuffer, buffSize, (* inputCursor), '2');
      incCursor = true;
      break;
    case KEYPAD_3:
      insChar(textBuffer, buffSize, (* inputCursor), '3');
      incCursor = true;
      break;
    case KEYPAD_4:
      insChar(textBuffer, buffSize, (* inputCursor), '4');
      incCursor = true;
      break;
    case KEYPAD_5:
      insChar(textBuffer, buffSize, (* inputCursor), '5');
      incCursor = true;
      break;
    case KEYPAD_6:
      insChar(textBuffer, buffSize, (* inputCursor), '6');
      incCursor = true;
      break;
    case KEYPAD_7:
      insChar(textBuffer, buffSize, (* inputCursor), '7');
      incCursor = true;
      break;
    case KEYPAD_8:
      insChar(textBuffer, buffSize, (* inputCursor), '8');
      incCursor = true;
      break;
    case KEYPAD_9:
      insChar(textBuffer, buffSize, (* inputCursor), '9');
      incCursor = true;
      break;
    case KEYPAD_0:
      insChar(textBuffer, buffSize, (* inputCursor), '0');
      incCursor = true;
      break;
    case KEYPAD_PERIOD:
      insChar(textBuffer, buffSize, (* inputCursor), '.');
      incCursor = true;
      break;
    case KEY_LEFT_CTRL:
      break;
    case KEY_LEFT_SHIFT:
      break;
    case KEY_LEFT_ALT:
      break;
    case KEY_LEFT_GUI:
      break;
    case KEY_RIGHT_CTRL:
      break;
    case KEY_RIGHT_SHIFT:
      break;
    case KEY_RIGHT_ALT:
      break;
    case KEY_RIGHT_GUI:
      break;
    
    default:
      const bool  shifted = (modifiers & MODIFIERKEY_SHIFT) ||
                            (modifiers & MODIFIERKEY_RIGHT_SHIFT);
      const char  tmp     = (char)HID2ArduKEY(keyStroke, shifted);
      insChar(textBuffer, buffSize, (* inputCursor), tmp);
      incCursor = true;
      break;
  }

  // Update Cursor position
  if (incCursor) deltaPos = 1;
  else if (decCursor) deltaPos = -1;
  if (deltaPos != 0)
    (* inputCursor) = constrain(
        (* inputCursor)+deltaPos, 0, min(strlen(textBuffer), buffSize)
        );
  return;
}

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
    ){

  // only need to do work if field is active
  if (active){
    // ensure cursor range safety
    (* inputCursor) = constrain((* inputCursor), 0, buffSize-1);

    static uint16_t last  = 0;  // last active key in key-press buffer
    uint16_t        key   = 0;  // convenience variable

    // Parse keys in input key-press buffer
    if (keyPressed  &&
        !repeatLast ){

      // Parse keys in input key-press buffer
      for (uint8_t i = 0; i < PRESSED_KEYS_BUFF_SIZE-1; i++) {
        key = keyStrokes[i];
        parseKey(
            textBuffer, 
            buffSize, 
            inputCursor, 
            key,
            modifiers);

        if (key != 0) last = key; // find most recent valid key press
        keyStrokes[i] = 0;        // prevent repeat keystrokes from buffer
      }
    }

    // Repeat last entered key if it's held
    if (repeatLast && last != 0) {
      parseKey(
          textBuffer, 
          buffSize, 
          inputCursor, 
          last,
          modifiers);
    }

    // Stop repeat if last entered key is released
    if (lastReleased == last) last = 0;
  }

  return;
};

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
    ){

  if (active) sm->disableKeyStrokePassthrough();

  uint16_t          tmpCursor     = constrain((* inputCursor), 0, buffSize-1);
  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();
  const   uint16_t  averageColor  = sm->getAverageColor();
  const   uint16_t  primaryColorI = sm->getPrimaryColorInverted();
  const   uint16_t  TouchX        = sm->getTouchX();
  const   uint16_t  TouchY        = sm->getTouchY();

  static  uint16_t  prevCursor    = 0;  // Used for tracking cursor changes
  static  uint32_t  blinkTimer    = 0;
  static  bool      blinkCursor   = false;

  const   uint32_t  timeDiff      = millis() - blinkTimer;
  if (timeDiff >= 666         ||
      millis()  < blinkTimer  ){  // handle integer overflow
    blinkCursor = !blinkCursor;
    blinkTimer = millis();
  }

  sm->tft->setTextWrap(false);

  sm->tft->setFont(font);
  sm->tft->setTextDatum(textDatum);
  uint16_t bx, by, bw, bh;
  sm->tft->getTextBounds(
      (uint8_t *)textBuffer, 
      strlen(textBuffer)+2,
      posX, posY,
      &bx, &by, &bw, &bh
      );
  sm->tft->fillRect(posX-4, posY-6, boxW+8, boxH+12, primaryColor);
  sm->tft->drawRoundRect(
      max(posX-4, 0),  posY-6, 
      boxW+8,    boxH+12,
      boxH/8,
      active?primaryColorI:accentsColor);

  //sm->tft->setTextColor(active?accentsColor:averageColor);
  //sm->tft->setTextColor(active?primaryColorI:accentsColor);
  sm->tft->setTextColor(accentsColor);
  sm->tft->drawString(textBuffer, min(strlen(textBuffer)+2, buffSize+1), posX, posY);

  if (active){
    static uint16_t cursorHPos = posX;

    //if(tmpCursor != prevCursor ){ // Avoid recalculating every frame
    if(true){ // Avoid recalculating every frame
      cursorHPos = posX;
      const char lastChar = textBuffer[tmpCursor-1];

      // trailing spaces are not added to width, change to 'i' to compensate
      if (lastChar == ' ') textBuffer[tmpCursor-1] = 'i';

      // Calculate cursor position
      sm->tft->getTextBounds(
          (uint8_t *)textBuffer, 
          tmpCursor,
          posX, posY,
          &bx, &by, &bw, &bh
          );
      cursorHPos += bw;
      textBuffer[tmpCursor-1] = lastChar;
      prevCursor = tmpCursor;
    }

    // Draw cursor line
    sm->tft->drawFastVLine(
        cursorHPos, posY-2, 
        bh+4, 
        blinkCursor?accentsColor:averageColor
        );
  }

  // reset vars if user exits/returns to home screen via home button
  if (  sm->touchEnabled()                        &&
        sm->getPrevTouch() != sm->getCurrTouch()  &&
        sm->getCurrTouch() == false               ){
    if (  TouchX <= 2*sm->homeButtonRadX              &&
          TouchX >= 0                                 &&
          TouchY <= DISP_HEIGHT-2*sm->homeButtonRadY  &&
          TouchY >= DISP_HEIGHT                       ){
      (* inputCursor) = 0;
    } 

    if (  TouchX <= posX+boxW &&
          TouchX >= posX      &&
          TouchY <= posY+boxH &&
          TouchY >= posY      ){
      (* inputCursor) = 0;
      return true;
    }
  }

  return false;
};
