#include "mediaController.h"

void doMediaController(stateMachine* sm){

  // Convenience Variables
  const uint16_t  accentsColor  = sm->getDetailsColor();
  const uint16_t  primaryColor  = sm->getPrimaryColor();
  const uint16_t  averageColor  = sm->getAverageColor();
  const uint16_t  accentsColorI = sm->getDetailsColorInverted();
  const uint16_t  primaryColorI = sm->getPrimaryColorInverted();
  uint8_t         numOutputs    = sm->getNumOutputs();

  const   bool    screenWasPressed      = sm->screenWasPressed();
  const   bool    screenWasLongPressed  = sm->screenWasLongPressed();
  const   bool    screenWasReleased     = sm->screenWasReleased();

  // Size of UI buttons
  const uint8_t   buttonRadX    = 28;
  const uint8_t   buttonRadY    = 24;

  static uint8_t  lastSetOutputs[16]  = {0};
  static uint8_t  passThroughEnabled  = false;  // Whether or not keystrokes are being sent to the media output
  static bool     flipCol             = false;  // Used for strobing color
                  flipCol             = !flipCol;
  static uint8_t  currentOutput       = 1;  // UI Index of currently selected output
  static uint8_t  outputAddress       = 0;  // address of currently selected output
  uint16_t tx, ty, tw, th;          // temporary variables for getting text spatial information
  //uint16_t posX=DISP_WIDTH>>2,      // General-use drawing coordinates
           //posY=DISP_HEIGHT>>2;
  uint16_t posX=160,      // General-use drawing coordinates
           posY=120+buttonRadY+6;
  
  char textString[48] = {'\0'};     // General-use string buffer
  char tmcstr[8] = {'\0'};          // General-use string buffer

  // Setup SM
  sm->enableDrawing();
  sm->enableTouchInput();
  sm->drawWallpaper();
  
  //  Central PLAY / PAUSE button
  if (doDoubleIconButton(
           posX, 
           posY,
           buttonRadX, (3*buttonRadY)/2 + 3,  15,
           AwesomeF000_12,
           (char)75,  // Track Play Symbol
           (char)76,  // Track Pause Symbol
           primaryColor,
           accentsColor,
           true,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_PLAY_PAUSE);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_PLAY_PAUSE);
  }

  // PLAY button
  if (doIconButton(
           posX + buttonRadX*2 + 5, 
           posY - (buttonRadY + 2),
           buttonRadX, buttonRadY,  5,
           AwesomeF000_18,
           (char)75,  // Track Play Symbol
           primaryColor,
           accentsColor,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_PLAY);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_PLAY);
  }

  // PAUSE butotn
  if (doIconButton(
           posX + buttonRadX*2 + 5, 
           posY + (buttonRadY + 2),
           buttonRadX, buttonRadY,  4,
           AwesomeF000_18,
           (char)76,  // Track Pause Symbol
           primaryColor,
           accentsColor,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_PAUSE);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_PAUSE);
  }

  // Next Track button
  if (doIconButton(
           posX + 2*(buttonRadX*2 + 5), 
           posY,
           buttonRadX, buttonRadY,  5,
           AwesomeF000_18,
           (char)81,  // Next Track Symbol
           primaryColor,
           accentsColor,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_NEXT_TRACK);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_NEXT_TRACK);
  }

  // Eject Disk Button
  if (doIconButton(
           posX + 2*(buttonRadX*2 + 5), 
           posY + 1*(buttonRadY*2 + 3),
           buttonRadX, buttonRadY - 7,  -1,
           AwesomeF000_18,
           (char)82,  // Eject Media Symbol
           averageColor,
           primaryColor,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_EJECT);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_EJECT);
  }

  // Track STOP button
  if (doIconButton(
           posX - (buttonRadX*2 + 5), 
           posY,
           buttonRadX, buttonRadY,  4,
           AwesomeF000_18,
           (char)77,  // Track Stop Symbol
           primaryColor,
           accentsColor,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_STOP);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_STOP);
  }

  // Previous Track button
  if (doIconButton(
           posX - 2*(buttonRadX*2 + 5), 
           posY,
           buttonRadX, buttonRadY,  5,
           AwesomeF000_18,
           (char)72,  // Previous Track Symbol
           primaryColor,
           accentsColor,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_PREV_TRACK);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_PREV_TRACK);
  }

  //  Volume Increase Button
  if (doDoubleIconButton(
           posX + 2*(buttonRadX*2 + 5), 
           posY - 2*(buttonRadY*2 + 5),
           buttonRadX, buttonRadY,  5,
           AwesomeF000_18,
           (char)40,  // Volume Up Symbol
           (char)103, // Plus Symbol
           accentsColor,
           primaryColor,
           true,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_VOLUME_INC);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_VOLUME_INC);
  }

  //  Volume Decrease Button
  if (doDoubleIconButton(
           posX + 2*(buttonRadX*2 + 5), 
           posY - (buttonRadY*2 + 5),
           buttonRadX, buttonRadY,  5,
           AwesomeF000_18,
           (char)40,  // Volume Up Symbol
           (char)104, // Minus Symbol
           accentsColor,
           primaryColor,
           true,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_VOLUME_DEC);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_VOLUME_DEC);
  }

  //  Volume Mute Button
  if (doDoubleIconButton(
           posX + (buttonRadX*2 + 5), 
           posY - 3*(buttonRadY + 3),
           buttonRadX, buttonRadY,  5,
           AwesomeF000_18,
           (char)38,  // Volume Mute Symbol
           (char)13,  // Cross Symbol
           accentsColor,
           primaryColor,
           true,
           sm)){
    if (screenWasPressed)   sm->sendKeyStroke(outputAddress, 1|128, 0, KEY_MEDIA_MUTE);
    if (screenWasReleased)  sm->sendKeyStroke(outputAddress, 0|128, 0, KEY_MEDIA_MUTE);
  }
  
  //  Next Selected Output Button
  if (doIconButton(
           posX - 2*(buttonRadX*2 + 5), 
           posY - 2*(buttonRadY*2 + 5),
           buttonRadX, buttonRadY,  0,
           AwesomeF000_18,
           (char)119, //  Up Arrow 
           primaryColor,
           passThroughEnabled?averageColor:primaryColorI,
           sm)){
    if (screenWasPressed & !passThroughEnabled) outputAddress = sm->nextOutputAddress(outputAddress, 32);
  }

  //  Previous Selected Output Button
  if (doIconButton(
           posX - 2*(buttonRadX*2 + 5), 
           posY - 1*(buttonRadY*2 + 5),
           buttonRadX, buttonRadY,  4,
           AwesomeF000_18,
           (char)120, //  Down Arrow
           primaryColor,
           passThroughEnabled?averageColor:primaryColorI,
           sm)){
    if (screenWasPressed & !passThroughEnabled) outputAddress = sm->prevOutputAddress(outputAddress, 32);
  }

  //  Toggle Keystroke passthrough to media output
  if (doDoubleIconButton(
           posX - (buttonRadX*2 + 5), 
           posY - 3*(buttonRadY + 3),
           buttonRadX, buttonRadY,  5,
           AwesomeF100_18,
           (char)28,  // Keyboard Symbol
           (char)76,  // Outgoing Symbol
           primaryColor,
           passThroughEnabled?(flipCol?primaryColorI:accentsColor):primaryColorI,
           passThroughEnabled,
           sm)){
    if (screenWasPressed) {
      passThroughEnabled = !passThroughEnabled;

      if (passThroughEnabled) {
        sm->getEnabledOutputs(lastSetOutputs);
        sm->disableAllOutputs();
        sm->enableOutput(outputAddress);
      } else {
        sm->disableAllOutputs();
        sm->setEnabledOutputs(lastSetOutputs);
        memset(lastSetOutputs, 0, 16);
      }
    }
  }

  sm->tft->setFont(Michroma_10);
  sm->tft->setTextColor(primaryColorI);
  sm->tft->setTextDatum(BL_DATUM);
  strcpy(textString, "KB-Intensify            ");
  sm->tft->drawString(textString, 24,
           5, 
           DISP_HEIGHT - (buttonRadY + 32)
      );
  strcpy(textString, "Multi-Media             ");
  sm->tft->drawString(textString, 24,
           (buttonRadX*2 + 5), 
           DISP_HEIGHT - (buttonRadY + 17)
      );
  strcpy(textString, "            Control Deck");
  sm->tft->drawString(textString, 24,
           (buttonRadX*2 + 30), 
           DISP_HEIGHT - (buttonRadY + 1)
      );

  itoa(outputAddress, tmcstr, 10);
  sm->tft->setTextColor(accentsColor);
  sm->tft->setTextDatum(TL_DATUM);
  if (passThroughEnabled) {
    sm->tft->setFont(Arial_8);
    strcpy(textString, "M.Output: ___ (Bypassing Keystrokes)");
  } else {
    sm->tft->setFont(Arial_12);
    strcpy(textString, "M.Output: ___                       ");
  }
  textString[10] = tmcstr[0];
  textString[11] = tmcstr[1];
  textString[12] = tmcstr[2];
  sm->tft->drawString(textString, passThroughEnabled?36:13,
           posX - 1*(buttonRadX*3 + 5), 
           posY - 1*(buttonRadY*5 + 5)
      );

  return;
}
