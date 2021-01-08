#include "smStruct.h"

stateMachine::stateMachine(void){
  return;
};
stateMachine::~stateMachine(void){
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
  TS_Point p = this->ts->getPoint();
  return p.x;
};
uint16_t stateMachine::getRawTouchY(void){
  TS_Point p = this->ts->getPoint();
  return p.y;
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

