#include "outputmux.h"

void doOutputMultiplexor(stateMachine* sm){

  uint8_t shortcutBuff[16] = {0};

  // Convenience Variables
  const uint16_t  accentsColor  = sm->getDetailsColor();
  const uint16_t  primaryColor  = sm->getPrimaryColor();
  const uint16_t  averageColor  = sm->getAverageColor();
  uint8_t         numOutputs    = sm->getNumOutputs();

  const   bool    screenWasPressed      = sm->screenWasPressed();
  const   bool    screenWasLongPressed  = sm->screenWasLongPressed();
  const   bool    screenWasReleased     = sm->screenWasReleased();

  // Size of UI buttons
  const uint8_t   buttonRadX    = 28;
  const uint8_t   buttonRadY    = 24;

  // Expose Advanced controls
  static bool     advancedMode  = false;

  static uint8_t currentOutput = 1; // UI Index of currently selected output
  static uint8_t outputAddress = 0; // address of currently selected output
  uint16_t tx, ty, tw, th;          // temporary variables for getting text spatial information
  uint16_t posX=14+2*buttonRadX,    // General-use drawing coordinates
           posY=29;//+1*buttonRadY;
  
  char textString[48] = {'\0'};     // General-use string buffer
  char tmcstr[8] = {'\0'};          // General-use string buffer

  // Setup SM
  sm->enableDrawing();
  sm->enableTouchInput();
  //sm->tft->fillScreen(ILI9341_BLACK);
  sm->drawWallpaper();

  sm->tft->setTextColor(accentsColor);
  sm->tft->setTextDatum(TL_DATUM);
  sm->tft->setFont(Arial_12);
  strcpy(textString, "Output: ");
  itoa(currentOutput, tmcstr, 10);  // convert output index from int to string
  textString[ 8] = tmcstr[0];       // assign output index as chars to textstring
  textString[ 9] = tmcstr[1];       // v
  textString[10] = tmcstr[2];       // v
  //textString[8] = (currentOutput+1) + '0';
  sm->tft->drawString(textString, 11, posX, posY);
  sm->tft->getTextBounds((const uint8_t*)textString, 11, int16_t(posX), int16_t(posY), &tx, &ty, &tw, &th);
  strcpy(textString, " / ");
  memset(tmcstr, '\0', 8);
  itoa(numOutputs, tmcstr, 10);     // Convert number of outputs from int to string
  textString[3] = tmcstr[0];        // assign number of outputs as chars to textring
  textString[4] = tmcstr[1];        // v
  textString[5] = tmcstr[2];        // v
  sm->tft->drawString(textString, 6, posX+tw, posY);
  posY += 3*(th>>1);
  strcpy(textString, "Address: ");
  itoa(outputAddress, tmcstr, 10);
  textString[ 9] = tmcstr[0];       // assign output index as chars to textstring
  textString[10] = tmcstr[1];       // v
  textString[11] = tmcstr[2];       // v
  sm->tft->drawString(textString, 12, posX, posY);
  posY += 3*(th>>1);
  strcpy(textString, "Description: ");
  sm->tft->drawString(textString, 13, posX, posY);
  posY += 3*(th>>1);
  memset(tmcstr, '\0', 8);
  // Output is sole system primary
  if (outputAddress == 0){
    strcpy(textString, "Primary Output");
    sm->tft->drawString(textString, 14, posX, posY);
    posY += 3*(th>>1);
    strcpy(textString, "(Lowest Latency)");
    sm->tft->drawString(textString, 16, posX, posY);
    posY += 3*(th>>1);
    sm->tft->setFont(AwesomeF100_12);
    sm->tft->setCursor(posX, posY);
    sm->tft->print((char)8); // Computer Symbol
    posX += 24;
  }
  // Output supports media keys
  if (sm->getOutputCapability(outputAddress) & 32) {
    sm->tft->setFont(AwesomeF000_12);
    sm->tft->setCursor(posX, posY);
    sm->tft->print((char)1);
    posX += 24;
  }

  // Output supports bluetooth
  if (sm->getOutputCapability(outputAddress) & 16) {
    sm->tft->drawBitmap(posX, posY, bt_16, 8, 16, accentsColor);
    posX += 24;
  } else {
    sm->tft->drawBitmap(posX, posY, usb_16, 16, 16, accentsColor);
    posX += 24;
  }

  // Button coordinates
  posX=34;
  posY=44+51*3;

  // 4 buttons vertical: y=44,44+51,44+51*2,44+51*3
  // 3 buttons vertical: y=48,48+72,48+2*72
  char titleBuff[32] = {'\0'};

  const uint8_t scOffset = advancedMode?4:0;
  for (uint8_t i = scOffset; i < 4+scOffset; i++){
    posX+=(2*buttonRadX)+7;
    const bool shortcutSaved = sm->isShortcutSet(i);
    shortcutSaved?memset(titleBuff, '\0', 16):strcpy(titleBuff, "Add\nShortcut");
    if ((shortcutSaved?
        doIconButton(   // Use Icon if shortcut is set
            posX, posY,
            buttonRadX, buttonRadY, 6,
            Michroma_18,
            (char)(i+1+'0'),
            primaryColor,
            accentsColor,
            sm
            ):
        doIconTextButton( // "Add Shortcut"
            posX, posY,
            buttonRadX, buttonRadY,  -5,
            titleBuff,
            12,
            AwesomeF000_18,
            (char)103,
            primaryColor,
            averageColor,
            sm
            ))            &&
        screenWasReleased ){
      // Save current output configuration to shortcut
      if (sm->getTouchPressHeld()){
        if (sm->isShortcutSet(i))
          sm->assignShortcut(i, shortcutBuff);
        else
        {
          sm->getEnabledOutputs(shortcutBuff);
          sm->assignShortcut(i, shortcutBuff);
        }

#ifdef DEBUG
        if (shortcutSaved) Serial.println("Setting Shortcut!");
        else Serial.println ("Unsetting Shortcut");
#endif
      } else {  // Recall configuration from shortcut
#ifdef DEBUG
        Serial.println("Applying Shortcut!");
#endif
        if (shortcutSaved) sm->applyShortcut(i);
      }
    }
  }

  posX = 34;
  posY = 64;
  if (doIconButton(
           posX, posY,
           buttonRadX, 3*(buttonRadY>>1), 5,
           AwesomeF000_18,
           (char)119, // Up arrow
           primaryColor,
           accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    currentOutput = rollover(currentOutput+1, 1, numOutputs); // Update index
    if (!advancedMode){
      if (sm->getNumActOutputs() > 0) sm->disableAllOutputs();// Disable all outputs if more than one enabled
      else sm->disableOutput(outputAddress);                  // Disable current output if using basic controls
      outputAddress = sm->nextOutputAddress(outputAddress);   // Update output address
      sm->enableOutput(outputAddress);                        // Enable output on new address
    } else {
      outputAddress = sm->nextOutputAddress(outputAddress);   // Update output address
    }
  }
  posY+=(6*(buttonRadY>>1))+7;
  if (doIconButton(
           posX, posY,
           buttonRadX, 3*(buttonRadY>>1), 10,
           AwesomeF000_18,
           (char)120, // Down Arrow
           primaryColor,
           accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    currentOutput = rollover(currentOutput-1, 1, numOutputs); // Update Index
    if (!advancedMode){
      if (sm->getNumActOutputs() > 0) sm->disableAllOutputs();// Disable all outputs if more than one enabled
      else sm->disableOutput(outputAddress);                  // Disable current output if using basic controls
      outputAddress = sm->prevOutputAddress(outputAddress);   // Update output address
      sm->enableOutput(outputAddress);                        // Enable output on new address
    } else {
      outputAddress = sm->prevOutputAddress(outputAddress);   // Update output address
    }
  }

  posX=DISP_WIDTH-34;
  posY=44;
  strcpy(titleBuff, "Advanced\nSettings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           17,
           AwesomeF000_18,
           (char)41,  // Four boxes idk
           primaryColor,
           advancedMode?sm->getPrimaryColorInverted():accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    advancedMode = !advancedMode;
  }

  posX-=(2*buttonRadX)+7;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Refresh\nDevices");
  if (advancedMode &&
      doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           15,
           AwesomeF000_18,
           (char)30,  // Refresh symbol
           primaryColor,
           accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    currentOutput = 1;
    outputAddress = 0;
    sm->scanForOutputs();
  }

  //posX-=(2*buttonRadX)+7;
  posY = 44;
  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Enable\nOutput");
  if (advancedMode &&
      doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           13,
           AwesomeF000_18,
           (char)88,  // Check mark
           primaryColor,
           sm->isOutputEnabled(outputAddress)?sm->getPrimaryColorInverted():accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    sm->enableOutput(outputAddress);
  }
  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Disable\nOutput");
  if (advancedMode &&
      doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           14,
           AwesomeF000_18,
           (char)87,  // X mark
           primaryColor,
           !sm->isOutputEnabled(outputAddress)?sm->getPrimaryColorInverted():accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    sm->disableOutput(outputAddress);
  }

  posX+=(2*buttonRadX)+7;
  posY = 44;
  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Enable\nAll");
  if (advancedMode &&
      doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           10,
           AwesomeF000_18,
           (char)93,  // Circled Check mark
           primaryColor,
           accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    sm->enableAllOutputs();
  }
  posY+=(2*buttonRadY)+3;
  memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Disable\nAll");
  if (advancedMode &&
      doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           11,
           AwesomeF000_18,
           (char)92,  // Circled X mark
           primaryColor,
           accentsColor,
           sm
           )            &&
      screenWasPressed  ){
    sm->disableAllOutputs();
  }

  return;
}

const short rollover(const short input, const short lowerLimit, const short upperLimit){
  unsigned short output = input;
  const unsigned short diff = upperLimit - lowerLimit + 1;
  while(output > upperLimit) output -= diff;
  while(output < lowerLimit) output += diff;
  return output;
}
