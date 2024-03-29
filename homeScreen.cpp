#include "homeScreen.h"

#define degToRad(angleindegrees) ((angleindegrees) * 3.1415926535 / 180.0)

void drawStatusBars(stateMachine* sm){

  static  uint32_t  frameTimer    = 0;
  static  uint16_t  frameCount    = 0;
  static  uint16_t  fps           = 0;

  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();
  const   uint16_t  averageColor  = sm->getAverageColor();
  const   uint16_t  accentsColorI = sm->getDetailsColorInverted();
  const   uint16_t  primaryColorI = sm->getPrimaryColorInverted();
  const   uint8_t   numActOutputs = sm->getNumActOutputs();
  const   uint8_t   activeOutput  = sm->getActiveOutput();
  const   uint8_t   currentScreen = sm->getScreen();
  const   bool      passThruTru   = sm->getKeyStrokePassthrough();
  const   bool      touchEnabled  = sm->touchEnabled();
  const   bool      currTouch     = sm->getCurrTouch();

  const   bool      screenWasPressed      = sm->screenWasPressed();
  const   bool      screenWasLongPressed  = sm->screenWasLongPressed();
  const   bool      screenWasReleased     = sm->screenWasReleased();

  // Size of UI buttons
  const   uint8_t   buttonRadX    = 28;
  const   uint8_t   buttonRadY    = 24;

  static  bool      homeDebounce  = false;
  static  uint8_t   prevHour      = 0, 
                    prevMinute    = 0, 
                    prevSecond    = 0, 
                    prevDay       = 0, 
                    prevMonth     = 0, 
                    prevWeekDay   = 0;

  bool        refresh   = true;

  if (currentScreen == 0) return;

  sm->enableTouchInput();

  // Calculate system FPS
  if (  millis()-frameTimer > 1000  ||
        millis() < frameTimer       ){
    fps = frameCount;
    frameCount = 0;
    frameTimer = millis();
  } else {
    frameCount++;
  }

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

    // Draw FPS
    memset(timeStr, '\0', 16);
    itoa(fps, timeStr, 10);
    sm->tft->setTextDatum(TL_DATUM);
    sm->tft->setTextColor(accentsColor);
    sm->tft->drawString(timeStr, 4, 2, 2);

    // Draw+watch return home button
    char titleBuff[32] = {'\0'};
    //strcpy(titleBuff, "Home");
    strcpy(titleBuff, " ");

    if (currentScreen != 1  &&
        currentScreen != 0  &&
        doIconTextButton(
            buttonRadX, DISP_HEIGHT-buttonRadY, // Lower-left corner
            buttonRadX, buttonRadY,  -5,
            titleBuff,
						1,
            AwesomeF000_32,
            (char)21,
            primaryColor,
            accentsColor,
            sm
            )         &&
        !homeDebounce ){
      if (screenWasLongPressed){
        sm->setScreen(-1);  // Revert to last app screen if long pressed
        homeDebounce = true;
      } 
      if (screenWasReleased){                 // Go to home screen
        sm->setScreen(1);
        //sm->resetTouch();
      }
      sm->enableKeyStrokePassthrough();
    }
    if (!currTouch) homeDebounce = false;

  /*
   *  Draw Speedometer on homescreen
   */
    if (currentScreen == 1) {

      static int16_t someNum = 0;
      //someNum++;
      uint16_t  posX     = DISP_WIDTH  -(DISP_WIDTH>>5),
                posY     = DISP_HEIGHT -(DISP_HEIGHT/5),
                rDial    = min(DISP_WIDTH, DISP_HEIGHT)/3,
                rNeedle  = min(DISP_WIDTH, DISP_HEIGHT)>>6,
                needLen  = min(DISP_WIDTH, DISP_HEIGHT)/3;
      float     ang0     = degToRad(someNum),
                ang1     = degToRad(someNum-90);

      // Draw Dial Body
      sm->tft->fillCircle(
          posX, posY,
          rDial,
          primaryColor);
      sm->tft->drawCircle(
          posX, posY,
          rDial,
          accentsColor);

      //char tmc[8] = {'k', 's', '/', 'm', ' ', 'x', '1', '0'};
      char tmc[8] = {'\0'};
      sm->tft->setFont(Michroma_10);
      sm->tft->setTextDatum(CR_DATUM);
      strcpy(tmc, "ks/m");
      sm->tft->drawString(tmc, 8,
        posX+8,
        posY-38
        );
      strcpy(tmc, "x10");
      sm->tft->drawString(tmc, 8,
        posX+8,
        posY-24
        );
      memset(tmc, '\0', 8);

      // Draw Tick Marks
      sm->tft->setFont(Michroma_10);
      sm->tft->setTextColor(accentsColor);
      sm->tft->setTextDatum(CC_DATUM);
      int16_t rf0, rf1;
      float tcos, tsin;
      for (uint8_t i = 0; i < 12; i++) {
        ang0 = degToRad(171+i*9);
        tcos = cosf(ang0);
        tsin = sinf(ang0);
        rf0 = rDial-(3<<(!(i&1)));
        rf1 = rDial-(7<<(!(i&1)));
        tmc[0] = (i>>1)+'0';
        if (!(i&1)) sm->tft->drawString(tmc, 1, 
          posX + int16_t((float)(rf1)*tcos),
          posY + int16_t((float)(rf1)*tsin) - 2
          );
        sm->tft->drawLine(
          posX + int16_t((float)rDial*tcos), 
          posY + int16_t((float)rDial*tsin),
          posX + int16_t((float)(rf0)*tcos),
          posY + int16_t((float)(rf0)*tsin),
          accentsColor
          );
      }

      // Draw Needle
      uint16_t      kpm       = map(sm->getKPM(), 0, 600, 171, 279);
      float         currAng   = (float)kpm;
      static float  prevAng   = 0.0f;
                
      if (abs(currAng - prevAng) > 0.01f) {
        if (currAng > prevAng)
          prevAng += (currAng - prevAng) / 4.0f;
        if (currAng < prevAng)
          prevAng -= (prevAng - currAng) / 8.0f;
      }

#ifdef DEBUG
      Serial.print("currAng: ");
      Serial.print(currAng);
      Serial.print(", prevAng: ");
      Serial.print(prevAng);
      Serial.print(", kpm: ");
      Serial.print(kpm);
      Serial.println();
#endif

      int16_t intAng = constrain((int16_t)prevAng, 171, 270);
      ang0 = degToRad(intAng),
      ang1 = degToRad(intAng-90);
      sm->tft->fillCircle(
          posX, posY,
          rNeedle,
          primaryColorI);
      sm->tft->fillTriangle(
          posX + int16_t((float)needLen*cosf(ang0)), posY + int16_t((float)needLen*sinf(ang0)),
          posX + int16_t((float)rNeedle*cosf(ang1)), posY + int16_t((float)rNeedle*sinf(ang1)),
          posX + int16_t((float)rNeedle*cosf(ang1+3.14159)), posY + int16_t((float)rNeedle*sinf(ang1+3.14159)),
          primaryColorI);

      // Draw Speed as 3-Digit 7-Segment display
      memset(tmc, '\0', 8);
      someNum++;
      itoa(min(sm->getKPM(), 999), tmc, 10);
      sm->tft->setFont(Crystal_18_Italic);
      sm->tft->setTextDatum(CR_DATUM);
      sm->tft->drawString(tmc, 3,
        posX,
        posY+24
        );

    }
    /*
    if (currentScreen != 1  &&
        currentScreen != 0  &&
        doIconTextButton(
            buttonRadX, DISP_HEIGHT-buttonRadY, // Lower-left corner
            buttonRadX, buttonRadY,  -10,
            titleBuff,
            AwesomeF000_32,
            (char)21,
            primaryColor,
            accentsColor,
            sm
            )               &&
      screenWasReleased     ){
    if (sm->getTouchPressHeld()) sm->setScreen(-1); // Revert to last app screen if long pressed
    else {                  // Go to home screen
      sm->setScreen(1);
      sm->resetTouch();
    }
      sm->enableKeyStrokePassthrough();
    }
    */

    uint16_t bx=8,
             by=0;

    /*
     * BOTTOM STATUS BAR
     *  Display Keyboard, System connection status
     */

    // Use color to indicate touch sensor input status
    //sm->tft->setTextColor(!sm->getCurrTouch()?accentsColor:primaryColorI);
    if (sm->getCurrTouch()) 
      sm->tft->setTextColor(sm->getTouchPressHeld()?accentsColorI:primaryColorI);
    else
      sm->tft->setTextColor(accentsColor);
    bx = 150; by = 226;
    sm->tft->setCursor(bx, by);
    sm->tft->setFont(Michroma_10);
    sm->tft->print("KB");
    sm->tft->setFont(Logisoso_12);
    sm->tft->print('i');

    // Use color to indicate Keyboard input status
    sm->tft->setTextColor(sm->getNumKeysPressed()==0?accentsColor:primaryColorI);
    bx -= 36; by -= 3;
    sm->tft->setCursor(bx, by);
    sm->tft->setFont(AwesomeF100_12);
    if (sm->keyboardConnected()) sm->tft->print((char)28); // Keyboard Symbol

    // Use color to indicate key-held status
    sm->tft->setTextColor(sm->getKeyPressHeld()?primaryColorI:accentsColor);
    bx += 22; by += 3;
    sm->tft->setCursor(bx, by);
    sm->tft->setFont(AwesomeF000_10);
    if (sm->keyboardConnected()) sm->tft->print((char)84); // Right Arrow
    
    bx += 12;   
    // Use Color+Symbol to indicate keystroke passthrough status
    if (passThruTru) {
      if (numActOutputs != 0) {
        bx += 36; by -= 0;
        sm->tft->setTextColor(accentsColor);
        sm->tft->setCursor(bx, by);
        sm->tft->setFont(AwesomeF000_10);
        sm->tft->print((char)84); // Right Arrow
        if (numActOutputs > 1) {  // Use multiple arrows for 2+ enabled outputs
          bx += 8;
          sm->tft->setCursor(bx, by);
          sm->tft->print((char)84); // Another Right Arrow
          bx += 24;
        }
      }
    } else {
      bx += 36; by -= 1;
      static bool flipCol = false;
      sm->tft->setTextColor(flipCol?accentsColor:primaryColorI);
      flipCol = !flipCol;
      sm->tft->setCursor(bx, by);
      sm->tft->setFont(AwesomeF000_10);
      sm->tft->print((char)94); // Block symbol
      bx += 2; by += 1;
    }

    sm->tft->setTextColor(passThruTru?accentsColor:averageColor);
    if (numActOutputs == 1) {
      bx += 14; by -= 1;
      if (activeOutput == 0) {    // Current Output is system primary
        sm->tft->setCursor(bx, by);
        sm->tft->setFont(AwesomeF100_12);
        sm->tft->print((char)8); // Computer Symbol
      } else if (sm->getOutputCapability(activeOutput) & 16) {  // Current Output is bluetooth
        if (sm->getNumKeysPressed() == 0)   // Bluetooth is NOT transmitting
          sm->tft->drawBitmap(bx+4, by-1, bt_16, 8, 16, passThruTru?accentsColor:averageColor);
        else                                // Bluetooth is transmitting
          sm->tft->drawBitmap(bx+4, by-1, btxmit_16, 16, 16, passThruTru?accentsColor:averageColor);
      } else {  // Current Output is presumed to be generic USB
        sm->tft->drawBitmap(bx, by, usb_16, 16, 16, passThruTru?accentsColor:averageColor);
      }
    } else if (numActOutputs > 1) { // Display number of active outputs if 2+ outputs are enabled
      sm->tft->setTextDatum(TC_DATUM);
      memset(timeStr, '\0', 16);
      itoa(numActOutputs, timeStr, 10);
      sm->tft->setFont(Michroma_10);
      sm->tft->drawString(timeStr, numActOutputs>9?(numActOutputs>99?3:2):1, bx, by);
    }
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

  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  averageColor  = sm->getAverageColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();
  const   uint16_t  accentsColorI = sm->getDetailsColorInverted();
  const   uint16_t  primaryColorI = sm->getPrimaryColorInverted();
  const   bool      currTouch     = sm->getCurrTouch();

  const   bool      screenWasPressed      = sm->screenWasPressed();
  const   bool      screenWasLongPressed  = sm->screenWasLongPressed();
  const   bool      screenWasReleased     = sm->screenWasReleased();

  // Size of UI buttons
  const   uint8_t   buttonRadX    = 28;
  const   uint8_t   buttonRadY    = 24;

  static  bool      homeDebounce  = false;

  sm->enableDrawing();
  sm->enableTouchInput();
  //sm->tft->fillScreen(ILI9341_BLACK);
  //sm->tft->writeRect(0, 0, DISP_WIDTH, DISP_HEIGHT, sm->userBackground);
  sm->drawWallpaper();

  uint16_t posX=32;
  uint16_t posY=44;

  if (screenWasPressed){ 
    homeDebounce = false;
  }

  // 4 buttons vertical: y=44,44+51,44+51*2,44+51*3
  // 3 buttons vertical: y=48,48+72,48+2*72
  char titleBuff[32] = {'\0'};
  strcpy(titleBuff, "Image\nViewer");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 13,
           AwesomeF000_18,
           (char)62,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
    sm->setScreen(2);
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Macro-\nnomicon");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 15,
           AwesomeF000_18,
           (char)45,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Password\nManager");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 16,
           AwesomeF200_18,
           (char)62,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
    sm->setScreen(3);
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Output\nMultiplexer");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 18,
           AwesomeF080_18,
           (char)104,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
    sm->setScreen(4);
  }

  posX+=(2*buttonRadX)+4;
  posY=44;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "System\nSettings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 16,
           AwesomeF080_18,
           (char)5,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
    Serial.println("moo");
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Keyboard\nSettings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 17,
           AwesomeF080_18,
           (char)45,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Media\nController");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 17,
           AwesomeF100_18,
           (char)68,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
    sm->setScreen(5);
  }

  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Save\nSettings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
					 13,
           AwesomeF100_18,
           (char)68,
           primaryColor,
           accentsColor,
           sm
           )            &&
      !homeDebounce     &&
      screenWasReleased ){
    homeDebounce = true;
    sm->saveUserSettings();
  }
  
  return;
}
