#include "smStruct.h"

stateMachine::stateMachine(void){
  memset(this->pressedKeys, 0, PRESSED_KEYS_BUFF_SIZE*sizeof(short));
  return;
};
stateMachine::~stateMachine(void){
  return;
};

void stateMachine::updateSM(void){

  // Determine if key has been press long enough to repeat
  if (abs(millis() - this->keyPressTimer) >= this->keyHoldTime) 
    this->keyPressHeld = !(this->keyHoldCanceled);

  if (this->numKeysPressed == 0){ this->keyPressHeld = false; this->keyHoldCanceled = false;}

  // Clear released key
  this->releasedKey = 0;
  return;
};

void stateMachine::setTS(XPT2046_Touchscreen *tsPtr){
  this->ts = tsPtr;
  return;
};

void stateMachine::initDevices(void){
  this->tft->begin();
  this->tft->useFrameBuffer(true);
  this->tft->setRotation(1);
  this->ts->begin();
  this->ts->setRotation(1);
  return;
};

void stateMachine::setCalibVars(
    const uint16_t minXts, 
    const uint16_t minYts, 
    const uint16_t maxXts, 
    const uint16_t maxYts){

  this->minXTS = minXts;
  this->minYTS = minYts;
  this->maxXTS = maxXts;
  this->maxYTS = maxYts;
  return;
};

void stateMachine::setScreen(const uint8_t screen) {
  currentScreen = constrain(screen, 0, 3);
  return;
};
uint8_t stateMachine::getScreen(void){
  return this->currentScreen;
};

void stateMachine::updateInputKeys(void){
  return;
};

void stateMachine::updateTouchStatus(void){
  this->prevTouch = this->currTouch;
  this->currTouch = this->ts->touched();
  TS_Point p      = this->ts->getPoint();
  if (this->currentScreen > 0){
    this->TouchX    = map(p.x, this->minXTS, this->maxXTS, 0, DISP_WIDTH);
    this->TouchY    = map(p.y, this->minYTS, this->maxYTS, DISP_HEIGHT, 0);
  }
  return;
};

bool stateMachine::getCurrTouch(void){
  return this->currTouch;
};

bool stateMachine::getPrevTouch(void){
  return this->prevTouch;
};

uint16_t stateMachine::getRawTouchX(void){
  return this->ts->getPoint().x;
};
uint16_t stateMachine::getRawTouchY(void){
  return this->ts->getPoint().y;
};

uint16_t stateMachine::getTouchX(void){
  return this->TouchX;
}

uint16_t stateMachine::getTouchY(void){
  return this->TouchY;
}

uint16_t stateMachine::getPrimaryColor(void){
  return this->primaryColor;
};

uint16_t stateMachine::getDetailsColor(void){
  return this->detailsColor;
};

uint16_t stateMachine::getAverageColor(void){
  // Averages two 16-bit (5-6-5) RGB colors
  const uint16_t colorA = this->primaryColor;
  const uint16_t colorB = this->detailsColor;
  const uint16_t mr = ((colorA >> 11)         + (colorB >> 11)        ) >> 1;
  const uint16_t mg = (((colorA & 2016) >> 5) + ((colorB & 2016) >> 5)) >> 1;
  const uint16_t mb = ((colorA & 31)          + (colorB & 31)         ) >> 1;
  return ((mr << 11) | (mg << 5) | mb);
};

const uint16_t stateMachine::getPrimaryColorInverted(void){
  return invert565color(this->primaryColor);
};

const uint16_t stateMachine::getDetailsColorInverted(void){
  return invert565color(this->detailsColor);
};

bool stateMachine::getKeyStrokePassthrough(void){
  return this->keyPassthrough;
};
void stateMachine::enableKeyStrokePassthrough(void){
  this->keyPassthrough = true;
  return;
};
void stateMachine::disableKeyStrokePassthrough(void){
  this->keyPassthrough = false;
  return;
};

void stateMachine::incNumKeysPressed(void){
  if (this->numKeysPressed < 255) this->numKeysPressed++;
  return;
};

void stateMachine::decNumKeysPressed(void){
  if (this->numKeysPressed > 0) this->numKeysPressed--;
  return;
};

uint16_t* stateMachine::getPressedKeys(void){
  return (const uint16_t*)&this->pressedKeys;
};

void stateMachine::clearPressedKeys(void){
  memset(this->pressedKeys, 0, PRESSED_KEYS_BUFF_SIZE*sizeof(uint16_t));
  return;
};

void stateMachine::clearReleasedKey(void){
  this->releasedKey = 0;
  return;
}

const uint8_t stateMachine::getNumKeysPressed(void){
  return this->numKeysPressed;
};

void stateMachine::setPressedKey(const uint16_t key){
  for (uint16_t i = 0; i < PRESSED_KEYS_BUFF_SIZE-1; i++)
    if (this->pressedKeys[i] == 0) {this->pressedKeys[i] = key; break;}

  //this->keyWasPressed = true;
  this->keyPressHeld  = false;
  this->keyPressTimer = millis();
  this->pressedKey    = key;
  return;
};

const uint16_t stateMachine::getPressedKey(void){
  const uint16_t tmp = this->pressedKey;
  //this->pressedKey = 0;
  return tmp;
};

void stateMachine::setReleasedKey(const uint16_t key){
  for (uint16_t i = 0; i < PRESSED_KEYS_BUFF_SIZE-1; i++)
    if (this->pressedKeys[i] == key) {delShort((short *)this->pressedKeys, PRESSED_KEYS_BUFF_SIZE, i);}

  this->releasedKey = key;
  return;
};

const uint16_t stateMachine::getReleasedKey(void){
  return this->releasedKey;
}

const bool stateMachine::getKeyPressHeld(void){
  return this->keyPressHeld;
};

void stateMachine::cancelKeyPressHold(void){
  this->keyHoldCanceled = true;
  this->keyPressHeld    = false;
  return;
};

const uint16_t invert565color(const uint16_t color){
  const uint16_t ir = 31 - (color >> 11);
  const uint16_t ig = 63 - ((color & 2016) >> 5);
  const uint16_t ib = 31 - (color & 31);
  return ((ir << 11) | (ig << 5) | ib);
}
