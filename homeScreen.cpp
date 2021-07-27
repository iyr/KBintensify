#include "homeScreen.h"
#include <t3nfonts.h>

void drawStatusBars(stateMachine* sm){
  bool        refresh   = true;
  static uint8_t prevHour=0, 
                 prevMinute=0, 
                 prevSecond=0, 
                 prevDay=0, 
                 prevMonth=0, 
                 prevWeekDay=0;

  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();
  const   uint16_t  averageColor  = sm->getAverageColor();
  const   uint16_t  accentsColorI = sm->getDetailsColorInverted();
  const   uint16_t  primaryColorI = sm->getPrimaryColorInverted();
  const   bool      passThruTru   = sm->getKeyStrokePassthrough();
  const   bool      touchEnabled  = sm->touchEnabled();

  // Size of UI buttons
  const   uint8_t   buttonRadX    = 28;
  const   uint8_t   buttonRadY    = 24;

  if (sm->getScreen() == 0) return;

  sm->enableTouchInput();

  if (sm->getCurrTouch() != sm->getPrevTouch()) {
    //if (!currTouch)
      //sm->tft->fillScreen(ILI9341_BLACK);
      //sm->tft->fillScreen(primaryColor);
    refresh = true;
  }

  if (  prevMinute  !=  minute()  ||
        //prevSecond  !=  second()  ||
        prevHour    !=  hour()    ||
        prevDay     !=  day()     ||
        prevWeekDay !=  weekday() ||
        prevMonth   !=  month()   ){
    refresh = true;
  }

  if (refresh){
    sm->tft->fillRect(0, 0, 320, 16, primaryColor);
    sm->tft->fillRect(0, 240-16, 320, 16, primaryColor);
    sm->tft->setFont(Arial_12);

    // Placeholder buffer
    char    timeStr[16] = {'\0'};
    uint8_t timeLen = 0;
    //memset(timeStr, '\0', 16);

    // Update Clock
    sm->tft->setTextColor(accentsColor);
    sm->tft->setTextDatum(TR_DATUM);
    timeLen = buildTimeStr(timeStr, true, hour(), minute());//, second());
    sm->tft->drawString(timeStr, timeLen+2, 318, 2);

    // Update Date
    memset(timeStr, '\0', 16);
    sm->tft->setTextDatum(TC_DATUM);
    timeLen = buildDateStr(timeStr, weekday(), month(), day());
    sm->tft->drawString(timeStr, timeLen+2, 160, 2);

    // Draw+watch return home button
    char titleBuff[32] = {'\0'};
    strcpy(titleBuff, " Home");
    if (sm->getScreen() != 1 && sm->getScreen() != 0)
      if (doIconTextButton(
            buttonRadX, DISP_HEIGHT-buttonRadY, // Lower-left corner
            buttonRadX, buttonRadY,  -10,
            titleBuff,
            AwesomeF000_32,
            (char)21,
            primaryColor,
            accentsColor,
            sm
            )){
        sm->setScreen(1);
        sm->enableKeyStrokePassthrough();
				sm->resetTouch();
      }

    uint16_t bx=8,
             by=0;

    /*
     *  Display Keyboard, System connection status
     */

    // Use color to indicate Keyboard input status
    sm->tft->setTextColor(sm->getNumKeysPressed()==0?accentsColor:primaryColorI);
    bx += 200; by += 223;
    sm->tft->setCursor(bx, by);
    sm->tft->setFont(AwesomeF100_12);
    sm->tft->print((char)28); // Keyboard Symbol

    // Use color to indicate key-held status
    sm->tft->setTextColor(sm->getKeyPressHeld()?primaryColorI:accentsColor);
    bx += 22; by += 3;
    sm->tft->setCursor(bx, by);
    sm->tft->setFont(AwesomeF000_10);
    sm->tft->print((char)84); // Right Arrow

    // Use color to indicate touch sensor input status
    //sm->tft->setTextColor(!sm->getCurrTouch()?accentsColor:primaryColorI);
		if (sm->getCurrTouch()) 
    	sm->tft->setTextColor(sm->getTouchPressHeld()?accentsColorI:primaryColorI);
		else
			sm->tft->setTextColor(accentsColor);
    bx += 12; by -= 0;
    sm->tft->setCursor(bx, by);
    sm->tft->setFont(Michroma_10);
    sm->tft->print("KB");
    sm->tft->setFont(Logisoso_12);
    sm->tft->print('i');
    
    // Use Color+Symbol to indicate keystroke passthrough status
    if (passThruTru) {
      bx += 36; by -= 0;
			sm->tft->setTextColor(accentsColor);
      sm->tft->setCursor(bx, by);
      sm->tft->setFont(AwesomeF000_10);
      sm->tft->print((char)84); // Right Arrow
    } else {
      bx += 34; by -= 1;
      static bool flipCol = false;
      sm->tft->setTextColor(flipCol?accentsColor:primaryColorI);
      flipCol = !flipCol;
      sm->tft->setCursor(bx, by);
      sm->tft->setFont(AwesomeF000_10);
      sm->tft->print((char)94); // Block symbol
      bx += 2; by += 1;
    }
    bx += 14; by -= 1;
    sm->tft->setTextColor(passThruTru?accentsColor:averageColor);
    sm->tft->setCursor(bx, by);
    sm->tft->setFont(AwesomeF100_12);
    sm->tft->print((char)8); // Computer Symbol
  }
  
  prevMonth   = month();
  prevWeekDay = weekday();
  prevDay     = day();
  prevHour    = hour();
  prevMinute  = minute();
  //prevSecond  = second();
  
  if (!touchEnabled) sm->disableTouchInput();
  return;
}

/*
 * Builds a neatly formatted date c-string
 * Returns length of formatted string
 */
uint8_t buildDateStr(
      char* output,
      int   weekday,
      int   month,
      int   day
      ){

  char* tmpBuff;
  tmpBuff = dayShortStr(weekday);
  output[0] = tmpBuff[0];
  output[1] = tmpBuff[1];
  output[2] = tmpBuff[2];
  output[3] = ',';
  output[4] = ' ';
  tmpBuff = monthShortStr(month);
  output[5] = tmpBuff[0];
  output[6] = tmpBuff[1];
  output[7] = tmpBuff[2];
  output[8] = '.';
  output[9] = ' ';
  if (day < 10) {
    output[10] = ' ';
    output[11] = day + '0';
  } else {
    itoa(day, tmpBuff, 10);
    output[10] = tmpBuff[0];
    output[11] = tmpBuff[1];
  }

  return strlen(output);
}

/*
 * Builds a neatly formatted time c-string, including seconds
 */
uint8_t buildTimeStr(
     char* output, 
     const bool useMeridiem, 
     uint8_t t_hour,
     const uint8_t t_minute,
     const uint8_t t_second
     ){

  // safety check
  if (t_hour > 23) t_hour = 0;

  if (useMeridiem) {

     // Placeholder
     char tmcstr[2];

     // Delineators
     output[2] = ':';
     output[5] = ':';
     output[8] = ' ';

     //Determine Meridiem, write to output
     output[10] = 'M';
     if (t_hour > 12) {
        output[9] = 'P';
        t_hour -= 12;
     } else {
        output[9] = 'A';
     }

     // Pad hour string if not two digits
     if (t_hour < 10) {
        output[0] = ' ';
        output[1] = t_hour + '0';
     } else {
        itoa(t_hour, tmcstr, 10);
        output[0] = tmcstr[0];
        output[1] = tmcstr[1];
     }

     // Pad minute string if not two digits
     if (t_minute < 10) {
        output[3] = '0';
        output[4] = t_minute + '0';
     } else {
        itoa(t_minute, tmcstr, 10);
        output[3] = tmcstr[0];
        output[4] = tmcstr[1];
     }

     // Pad second string if not two digits
     if (t_second < 10) {
        output[6] = '0';
        output[7] = t_second + '0';
     } else {
        itoa(t_second, tmcstr, 10);
        output[6] = tmcstr[0];
        output[7] = tmcstr[1];
     }
  }
  // Set string for 24-hour clock
  else {
     // Placeholder
     char tmcstr[2];
     // Delineators
     output[2] = ':';
     output[5] = ':';

     // Pad hour string if not two digits
     if (t_hour < 10) {
        output[0] = ' ';
        output[1] = t_hour + '0';
     } else {
        itoa(t_hour, tmcstr, 10);
        output[0] = tmcstr[0];
        output[1] = tmcstr[1];
     }

     // Pad minute string if not two digits
     if (t_minute < 10) {
        output[3] = '0';
        output[4] = t_minute + '0';
     } else {
        itoa(t_minute, tmcstr, 10);
        output[3] = tmcstr[0];
        output[4] = tmcstr[1];
     }
     // Pad second string if not two digits
     if (t_second < 10) {
        output[6] = '0';
        output[7] = t_second + '0';
     } else {
        itoa(t_second, tmcstr, 10);
        output[6] = tmcstr[0];
        output[7] = tmcstr[1];
     }
  }
   
  return strlen(output);
}

uint8_t buildTimeStr(
     char* output, 
     const bool useMeridiem, 
     uint8_t t_hour,
     const uint8_t t_minute
     ){

  // safety check
  if (t_hour > 23) t_hour = 0;

  if (useMeridiem) {

     // Placeholder
     char tmcstr[2];

     // Delineators
     output[2] = ':';
     output[5] = ' ';

     //Determine Meridiem, write to output
     output[7] = 'M';
     if (t_hour > 12) {
        output[6] = 'P';
        t_hour -= 12;
     } else {
        output[6] = 'A';
     }

     // Pad hour string if not two digits
     if (t_hour < 10) {
        output[0] = ' ';
        output[1] = t_hour + '0';
     } else {
        itoa(t_hour, tmcstr, 10);
        output[0] = tmcstr[0];
        output[1] = tmcstr[1];
     }

     // Pad minute string if not two digits
     if (t_minute < 10) {
        output[3] = '0';
        output[4] = t_minute + '0';
     } else {
        itoa(t_minute, tmcstr, 10);
        output[3] = tmcstr[0];
        output[4] = tmcstr[1];
     }
  }
  // Set string for 24-hour clock
  else {
     // Placeholder
     char tmcstr[2];

     // Delineators
     output[2] = ':';

     // Pad hour string if not two digits
     if (t_hour < 10) {
        output[0] = ' ';
        output[1] = t_hour + '0';
     } else {
        itoa(t_hour, tmcstr, 10);
        output[0] = tmcstr[0];
        output[1] = tmcstr[1];
     }

     // Pad minute string if not two digits
     if (t_minute < 10) {
        output[3] = '0';
        output[4] = t_minute + '0';
     } else {
        itoa(t_minute, tmcstr, 10);
        output[3] = tmcstr[0];
        output[4] = tmcstr[1];
     }
  }
   
  return strlen(output);
}

time_t getTeensy3Time(void){
  return Teensy3Clock.get();
}


void doHomeScreen(stateMachine* sm){
  if (sm->getCurrTouch() != sm->getPrevTouch()) {
    // toggle passthrough enable
    //if (sm->getCurrTouch()) sm->keyPassthrough = !(sm->keyPassthrough);
  } 
  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();

  // Size of UI buttons
  const   uint8_t   buttonRadX    = 28;
  const   uint8_t   buttonRadY    = 24;

  sm->enableDrawing();
  sm->enableTouchInput();
  sm->tft->fillScreen(ILI9341_BLACK);

  uint16_t posX=32,
           posY=44;

  // 4 buttons vertical: y=44,44+51,44+51*2,44+51*3
  // 3 buttons vertical: y=48,48+72,48+2*72
  char titleBuff[32] = {'\0'};
  strcpy(titleBuff, "Image Viewer");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)62,
           primaryColor,
           accentsColor,
           sm
           )){
    sm->setScreen(2);
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Macro- nomicon");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)45,
           primaryColor,
           accentsColor,
           sm
           )){
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Password Manager");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF200_18,
           (char)62,
           primaryColor,
           accentsColor,
           sm
           )){
    sm->setScreen(3);
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Output Multiplexer");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF080_18,
           (char)104,
           primaryColor,
           accentsColor,
           sm
           )){
		sm->setScreen(4);
		//sm->scanForOutputs();
  }

  posX+=(2*buttonRadX)+4;
  posY=44;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "System Settings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF080_18,
           (char)5,
           primaryColor,
           accentsColor,
           sm
           )){
    Serial.println("moo");
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Keyboard Settings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF080_18,
           (char)45,
           primaryColor,
           accentsColor,
           sm
           )){
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Media Controller");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF100_18,
           (char)68,
           primaryColor,
           accentsColor,
           sm
           )){
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Save Settings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF100_18,
           (char)68,
           primaryColor,
           accentsColor,
           sm
           )){
    sm->saveUserSettings();
  }
  return;
}
