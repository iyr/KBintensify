#include "passwordManager.h"
#include "textInputFieldClass.h"
#include <t3nfonts.h>

FLASHMEM void doPassMan(stateMachine* sm){

  textInputField passwordField = textInputField();
  textInputField databaseField = textInputField();
  textInputField key_fileField = textInputField();
  //               lol

  
  sm->tft->fillScreen(ILI9341_BLACK);
  sm->tft->setTextDatum(TC_DATUM);
  //sm->tft->setFont(AwesomeF080_96);
  char tmp[4] = {'\0'};
  tmp[0] = (char)4;
  tmp[1] = (char)28;
  //sm->tft->drawString1(tmp, 4, DISP_WIDTH/2, DISP_HEIGHT/5);
  return;
};
