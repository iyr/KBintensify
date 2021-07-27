#include "outputmux.h"

void doOutputMultiplexor(stateMachine* sm){
	// Expose Advanced controls
	static bool			advancedMode	= false;

	static bool			savedShortcuts[10] = {0};

  // Convenience placeholders
  const	uint16_t  accentsColor  = sm->getDetailsColor();
  const	uint16_t	primaryColor  = sm->getPrimaryColor();
	const uint16_t	averageColor	= sm->getAverageColor();
	uint8_t					numOutputs		= sm->getNumOutputs();

  // Size of UI buttons
  const	uint8_t   buttonRadX    = 28;
  const uint8_t   buttonRadY    = 24;

  sm->enableDrawing();
  sm->enableTouchInput();
  sm->tft->fillScreen(ILI9341_BLACK);

	static uint8_t currentOutput = 1;
	static uint8_t outputAddress = 0;
	uint16_t tx, ty, tw, th;
	
	char textString[16] = {'\0'};
	char tmcstr[8] = {'\0'};
  sm->tft->setTextDatum(TL_DATUM);
  sm->tft->setFont(Arial_12);
	strcpy(textString, "Output: ");
	itoa(currentOutput, tmcstr, 10);	// convert output index from int to string
	textString[ 8] = tmcstr[0];				// assign output index to textstring
	textString[ 9] = tmcstr[1];				// v
	textString[10] = tmcstr[2];				// v
	//textString[8] = (currentOutput+1) + '0';
	sm->tft->drawString(textString, 11, 15+2*buttonRadX, 55+1*buttonRadY);
	sm->tft->getTextBounds((const uint8_t*)textString, (uint16_t)11, int16_t(15+2*buttonRadX), int16_t(44+1*buttonRadY+5), &tx, &ty, &tw, &th);
	strcpy(textString, " / ");
	memset(tmcstr, '\0', 8);
	itoa(numOutputs, tmcstr, 10);
	textString[3] = tmcstr[0];
	textString[4] = tmcstr[1];
	textString[5] = tmcstr[2];
	sm->tft->drawString(textString, 6, 15+2*buttonRadX+tw, 55+1*buttonRadY);

	// Button coordinates
  uint16_t posX=34,
           posY=44;

  // 4 buttons vertical: y=44,44+51,44+51*2,44+51*3
  // 3 buttons vertical: y=48,48+72,48+2*72
  char titleBuff[32] = {'\0'};
  strcpy(titleBuff, "(1)Add Shortcut");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)103,
           savedShortcuts[0]?accentsColor:averageColor,
           primaryColor,
           sm
           )){
		// TODO
  }
  posX+=(2*buttonRadX)+7;
  strcpy(titleBuff, "(2)Add Shortcut");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)103,
           savedShortcuts[1]?accentsColor:averageColor,
           primaryColor,
           sm
           )){
		// TODO
  }
  posX+=(2*buttonRadX)+7;
  strcpy(titleBuff, "(3)Add Shortcut");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)103,
           savedShortcuts[2]?accentsColor:averageColor,
           primaryColor,
           sm
           )){
		// TODO
  }
  posX+=(2*buttonRadX)+7;
  strcpy(titleBuff, "(4)Add Shortcut");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)103,
           savedShortcuts[3]?accentsColor:averageColor,
           primaryColor,
           sm
           )){
		// TODO
  }
  posX+=(2*buttonRadX)+7;
  strcpy(titleBuff, "(5)Add Shortcut");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)103,
           savedShortcuts[4]?accentsColor:averageColor,
           primaryColor,
           sm
           )){
		// TODO
  }

	posX = 34;
	posY = 44;
  //posX+=(2*buttonRadX)+7;
  posY+=(2*buttonRadY)+7;
  if (doIconButton(
           posX, posY,
           buttonRadX, buttonRadY,	0,
           AwesomeF000_18,
           (char)119, // Up arrow
           primaryColor,
           accentsColor,
           sm
           )){
		currentOutput = rollover(currentOutput+1, 1, numOutputs);	// Update index
		if (!advancedMode){
			sm->disableOutput(outputAddress);												// Disable current output if using basic controls
			outputAddress = sm->nextOutputAddress(outputAddress);		// Update output address
			sm->enableOutput(outputAddress);												// Enable output on new address
		} else {
			outputAddress = sm->nextOutputAddress(outputAddress);		// Update output address
		}
  }
  posY+=(2*buttonRadY)+7;
  if (doIconButton(
           posX, posY,
           buttonRadX, buttonRadY,	0,
           AwesomeF000_18,
           (char)120,	// Down Arrow
           primaryColor,
           accentsColor,
           sm
           )){
		currentOutput = rollover(currentOutput-1, 1, numOutputs);	// Update Index
		if (!advancedMode){
			sm->disableOutput(outputAddress);												// Disable current output if using basic controls
			outputAddress = sm->prevOutputAddress(outputAddress);		// Update output address
			sm->enableOutput(outputAddress);												// Enable output on new address
		} else {
			outputAddress = sm->prevOutputAddress(outputAddress);		// Update output address
		}
  }

  posX=DISP_WIDTH-34;
  posY=DISP_HEIGHT-44;
  strcpy(titleBuff, "Advanced Settings");
  if (doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)41,	// Four boxes idk
           primaryColor,
           advancedMode?sm->getPrimaryColorInverted():accentsColor,
           sm
           )){
		advancedMode = !advancedMode;
  }

  posX-=(2*buttonRadX)+7;
	memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Refresh Devices");
  if (advancedMode &&
			doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)30,	// Refresh symbol
           primaryColor,
           accentsColor,
           sm
           )){
		sm->scanForOutputs();
  }

  //posX-=(2*buttonRadX)+7;
	posY = 44;
  posY+=(2*buttonRadY)+3;
	memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Enable Output");
  if (advancedMode &&
			doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)88,	// Check mark
           primaryColor,
           sm->isOutputEnabled(outputAddress)?sm->getPrimaryColorInverted():accentsColor,
           sm
           )){
		sm->enableOutput(outputAddress);
  }
  posY+=(2*buttonRadY)+3;
  strcpy(titleBuff, "Disable Output");
  if (advancedMode &&
			doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)87,	// X mark
           primaryColor,
           !sm->isOutputEnabled(outputAddress)?sm->getPrimaryColorInverted():accentsColor,
           sm
           )){
		sm->disableOutput(outputAddress);
  }

  posX+=(2*buttonRadX)+7;
	posY = 44;
  posY+=(2*buttonRadY)+3;
	memset(titleBuff, '\0', 32);
  strcpy(titleBuff, "Enable All");
  if (advancedMode &&
			doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)93,	// Circled Check mark
           primaryColor,
           accentsColor,
           sm
           )){
		sm->enableAllOutputs();
  }
  posY+=(2*buttonRadY)+3;
  strcpy(titleBuff, "Disable All");
  if (advancedMode &&
			doIconTextButton(
           posX, posY,
           buttonRadX, buttonRadY,  -5,
           titleBuff,
           AwesomeF000_18,
           (char)92,	// Circled X mark
           primaryColor,
           accentsColor,
           sm
           )){
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
