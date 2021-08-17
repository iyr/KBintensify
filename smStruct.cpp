#include "smStruct.h"

#define SETTINGS_FILESIZE_MAX 4096
#define DEBUG

stateMachine::stateMachine(void){
  memset(this->pressedKeys, 0, PRESSED_KEYS_BUFF_SIZE*sizeof(short));
  return;
};

stateMachine::~stateMachine(void){
  return;
};

void stateMachine::saveUserSettings(void){
  //if (this->sd->exists("/userSettings.conf"))
  this->sd->remove("/userSettings.cfg");

  File configFile = this->sd->open("/userSettings.cfg", FILE_WRITE);
  if (!configFile) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  StaticJsonDocument<SETTINGS_FILESIZE_MAX> doc;

  // Set touchscreen calibration variables
  doc["minXTS"] = this->minXTS;
  doc["minYTS"] = this->minYTS;
  doc["maxXTS"] = this->maxXTS;
  doc["maxYTS"] = this->maxYTS;

	// Save user shortcuts for sets of enabled outputs
	// Enabled outputs are represented as bitmaps of 16 bytes (for all 128 address outputs)
	for (uint8_t i = 0; i < NUM_OUTPUT_SHORTCUTS; i++)
		for (uint8_t j = 0; j < 16; j++)
			doc["OutputShortcuts"][i][j] = this->enabledOutputSets[i][j];

	// Set selected outputs (could be more memory/space efficient)
	for (uint8_t i = 0; i < MAX_OUTPUTS; i++){
		doc["outputs"][i] = this->outputs[i];
	}

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  configFile.close();
  return;
};

void stateMachine::loadUserSettings(const char* filepath){
  // Open file for reading
  File file = this->sd->open(filepath);

  // Allocate a temporary JsonDocument
  StaticJsonDocument<SETTINGS_FILESIZE_MAX> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  
  // Get calibration variables
  this->minXTS = doc["minXTS"];
  this->minYTS = doc["minYTS"];
  this->maxXTS = doc["maxXTS"];
  this->maxYTS = doc["maxYTS"];

	// Save user shortcuts for sets of enabled outputs
	for (uint8_t i = 0; i < NUM_OUTPUT_SHORTCUTS; i++)
		for (uint8_t j = 0; j < 16; j++)
			this->enabledOutputSets[i][j] = doc["OutputShortcuts"][i][j];

	// Restore selected outputs (could be more memory/space efficient)
	for (uint8_t i = 0; i < MAX_OUTPUTS; i++){
		this->outputs[i] = doc["outputs"][i];
	}

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
  return;
};

void stateMachine::loadUserSettings(void){
  return this->loadUserSettings("/userSettings.cfg");
};

void stateMachine::updateSM(void){

  if (this->keyPassthrough){
		// Determine if key has been pressed long enough to repeat
		if (abs(millis() - this->keyPressTimer) >= this->keyHoldTime) 
			this->keyPressHeld = !(this->keyHoldCanceled);

		if (this->numKeysPressed == 0){ this->keyPressHeld = false; this->keyHoldCanceled = false;}

		// Clear released key
		this->releasedKey = 0;
  }

	// Update backlight level
	if (this->BLrMode & 1) this->BLLevel = 255;
	else {
		uint16_t blt = this->BLLevel;		// convenience placeholder

		// Increase BL if BLrMode is set to respond to touch screen input
		if ((this->BLrMode & 32)	&& this->currTouch == true)		blt = constrain(blt+32, 0, 512);

		// Increase BL if BLrMode is set to respond to keystroke input
		if ((this->BLrMode & 64)	&& this->numKeysPressed > 0)	blt = constrain(blt+32, 0, 512); 

		// Increase BL if BLrMode is set to respond to proximity sensor input
		if ((this->BLrMode & 128)	&& this->proxlux != NULL) {
			// Whether or not the proximity sensor has crossed activation threshold
			static bool proxSense = false;

			// Only read proximity sensor every so often
			if (abs(millis() - this->BLfuncTimer) >= this->BLfuncRate) {
				const uint16_t prox = this->proxlux->getProximity();

				if (prox > this->BLthresh) proxSense = true;
				else proxSense = false;

				this->BLfuncTimer = millis();
			}

			// Increase BL if proximity sensor threshold was exceeded
			if (proxSense) blt = constrain(blt+24, 0, 512);
		}

		// Constantly decay brightness
		blt = constrain(blt-1, 0, 65535);
		this->BLLevel = blt;
	}

	// Write Backlight level (range constrained) to output pin
	analogWrite(this->BLpin, this->getBLlev());
  return;
};

void stateMachine::setKC(KeyboardController *kcPtr){
	this->kc = kcPtr;
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
	
	// output[0] is the default direct output of the Teensy 4.1
	// all other outputs are i2c devices
	this->outputs[0] = 224;
  return;
};


// Scans i2c bus for valid outputs while also getting each output's capabilities
void stateMachine::scanForOutputs(void){
	this->numOutputs = 1;	// There is always at least one output
	byte error;						// Get i2c return code
#ifdef DEBUG
	Serial.println(F("Now Scanning i2c bus for output devices"));
#endif
	this->outputs[0] = 224;
	for (uint8_t address = 1; address < MAX_OUTPUTS; address++){
		this->outputs[address] = 0;
		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		// Device found, check for output capabilities
		if (error == 0){
#ifdef DEBUG
			Serial.print(F("Device found on address: ")); 
			Serial.print(address);
			Serial.println(F(", Identifying..."));
#endif
			uint8_t capBuff[9] = {0};
			Wire.requestFrom(address, 8);
			for (uint8_t i = 0; i < 8; i++){
				capBuff[i] = Wire.read();
#ifdef DEBUG
				if (i < 7) Serial.print((char)capBuff[i]);
				else Serial.print(capBuff[i]);
#endif
				delay(1);
			}
			Serial.println();
			uint8_t tmp = capBuff[7];
			capBuff[7] = 'N';
			capBuff[8] = '\0';
			if (strcmp(capBuff, "Output:N") == 0){
				this->numOutputs++;
				this->outputs[address] = tmp;
#ifdef DEBUG
				Serial.println(F("Output valid!"));
				if (tmp & 16) Serial.println("Output supports Bluetooth");
				if (tmp & 32) Serial.println("Output supports media keys");
#endif
			} else {
#ifdef DEBUG
				for (uint8_t i = 0; i < 8; i++)
					Serial.print((char)capBuff[i]);
				Serial.println(F(" Unknown Device"));
#endif
			}
		}
		delay(1);
	}
	return;
};

// Send keystroke to output device specified at address
void stateMachine::sendKeyStroke(
	const uint8_t		deviceAddress,	// device i2c address to send the keystroke
	const uint8_t		isPressed,			// differentiate press / release
	const uint8_t		modMask,				// modifier keys mask
	const uint16_t	key							// the actual key code (typ 1 byte for ascii, 2 bytes for hid extras)
	) {

	if (deviceAddress > 0) {
		// Create array of bytes to send keystrokes over i2c to auxiliary output devices (eg, usb, bluetooth)
		uint8_t keyStrokeBytes[5];
		keyStrokeBytes[0] = isPressed & 1;						// 1: key press, 0: key release
		keyStrokeBytes[1] = modMask;									// second byte is the modifier mask

		if (isPressed & 128){																// Send raw keycode with no conversion
			keyStrokeBytes[2] = uint8_t((key >> 8) & -1);			// Parse key code into two bytes
			keyStrokeBytes[3] = uint8_t(key & -1);						// Parse key code into two bytes
			keyStrokeBytes[4] = uint8_t(key);
		} else 
		if (this->outputs[deviceAddress] & 32) {						// Send converted key code to another Teensy
			const uint16_t TnsyKey = HID2ArduKEY(key, false);
			keyStrokeBytes[2] = uint8_t((TnsyKey >> 8) & -1);	// Parse key code into two bytes
			keyStrokeBytes[3] = uint8_t(TnsyKey & -1);				// Parse key code into two bytes
			keyStrokeBytes[4] = uint8_t(TnsyKey);
		} else {																						// Send converted to generic Arduino keyboard lib device
			const uint16_t arduKey = Tnsy2ArduKEY(key);
			keyStrokeBytes[2] = uint8_t((key >> 8) & -1);			// Parse key code into two bytes
			keyStrokeBytes[3] = uint8_t(key & -1);						// Parse key code into two bytes
			keyStrokeBytes[4] = uint8_t(arduKey);
		}

		Wire.beginTransmission(deviceAddress);
		Wire.write(keyStrokeBytes, 5);
		Wire.endTransmission();
		return;
	} else {
		if (isPressed & 1)	Keyboard.press(HID2ArduKEY(key, false));
		else								Keyboard.release(HID2ArduKEY(key, false));
	}
}

// Send keystroke to all receiving output devices
void stateMachine::passKeyToOutputs(
	const uint8_t		isPressed,			// differentiate press / release
	const uint8_t		modMask,				// modifier keys mask
	const uint16_t	key							// the actual key code (typ 1 byte for ascii, 2 bytes for hid extras)
	) {
#ifdef DEBUG
	Serial.print(F("HID code (Host, Remap): "));
	Serial.print(key);
	Serial.print(F(", "));
	//if (this->outputs[deviceAddress] & 32)
		Serial.println(HID2ArduKEY(key, false));
	//else
		//Serial.println(Tnsy2ArduKEY(key));
	Serial.println(F("Passing keystroke to outputs..."));
#endif
	for (	uint8_t address = 1; address < MAX_OUTPUTS; address++) {
		if ((this->outputs[address] & 128) && (this->outputs[address] & 64)){
			this->sendKeyStroke(address, isPressed, modMask, key);
#ifdef DEBUG
			if (isPressed){
				Serial.print(F("Sending Key Press to address: " ));
			} else {
				Serial.print(F("Sending Key Release to address: " ));
			}
			Serial.println(address);
#endif
			//delay(1);
		}
	}
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

	// (-1) Revert previous and current application screens
	if (screen == 255) {
		const uint8_t tmp					= this->cycleLastScreen[1];
		this->cycleLastScreen[1]	= this->cycleLastScreen[0];
		this->cycleLastScreen[0]	= tmp;
		this->currentScreen				= constrain(tmp, 1, 5);
		return;

	} else {

		// Only update screen index if not home, calibration, or already in-cycle
		if (screen >  1 && this->cycleLastScreen[1] != screen) this->cycleLastScreen[0] = screen;

		// Swap positions of screen indexes to cycle
		if (screen == 1) {
			const uint8_t tmp					= this->cycleLastScreen[1];
			this->cycleLastScreen[1]	= this->cycleLastScreen[0];
			this->cycleLastScreen[0]	= tmp;
		}

		// Update screen to requested screen
		this->currentScreen = constrain(screen, 0, 5);
		return;
	}
};

const uint8_t stateMachine::getScreen(void){
  return this->currentScreen;
};

void stateMachine::captureFrameBufferToBackBuffer(void){
  //memset(this->backBuffer, -1, DISP_WIDTH*DISP_HEIGHT*sizeof(uint16_t));
  memcpy(
      this->backBuffer,
      this->tft->getFrameBuffer(),
      DISP_WIDTH*DISP_HEIGHT*sizeof(uint16_t)
      );
  //uint16_t* tmp = this->tft->getFrameBuffer();
  //for (uint32_t i = 0; i < DISP_WIDTH*DISP_HEIGHT; i++)
    //this->backBuffer[i] = tmp[i];
  return;
};

uint16_t* stateMachine::getBackBuffer(void){
  return this->backBuffer;
};

const uint16_t stateMachine::getPrimaryColor(void){
  return this->primaryColor;
};

const uint16_t stateMachine::getDetailsColor(void){
  return this->detailsColor;
};

const uint16_t stateMachine::getAverageColor(void){
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

void stateMachine::updateTouchStatus(void){
  this->prevTouch			= this->currTouch;
  this->currTouch			= this->ts->touched();
	this->prevPressHeld = this->touchPressHeld;
  TS_Point p					= this->ts->getPoint();

  // Determine if touch sensor has been pressed long enough for alternate input
  if (this->currTouch && abs(millis() - this->touchPressTimer) >= this->touchHoldTime){
    this->touchPressHeld = !(this->touchHoldCanceled);
	}

	// Record calibrated XY coordinates the first frame the touch screen is pressed
	if (this->currTouch && !this->prevTouch) {	// First frame screen is touched
		this->touchPressHeld		= false;
		this->touchHoldCanceled = false;
		this->touchPressTimer		= millis();
    this->TouchX0 = map(p.x, this->minXTS, this->maxXTS, 0, DISP_WIDTH	);
    this->TouchY0 = map(p.y, this->minYTS, this->maxYTS, DISP_HEIGHT, 0	);
    this->TouchX  = this->TouchX0;
    this->TouchY  = this->TouchY0;
	} 

	// Cancel touch hold if cursor leaves small circle where touch input was first registered
	if (//!this->currTouch || 
			(float)this->touchHoldRadius <= hypot(
				float(this->TouchX0 - this->TouchX), 
				float(this->TouchY0 - this->TouchY)) &&
			!this->touchPressHeld
			){
		this->TouchX0 = -1;
		this->TouchY0 = -1;
		this->touchHoldCanceled = true;
		this->touchPressHeld = false;
	}

	if (this->touchHoldCanceled) this->touchPressTimer = millis();

	if (!this->currTouch && this->prevTouch) {	// Frist frame touch is released from screen
		this->TouchX0 = -1;
		this->TouchY0 = -1;
	}

	//if (!this->currTouch && !this->prevTouch) {

	//}

  if (this->currentScreen > 0){
		if (this->currTouch || this->screenWasReleased()) {
			this->TouchX  = map(p.x, this->minXTS, this->maxXTS, 0, DISP_WIDTH);
			this->TouchY  = map(p.y, this->minYTS, this->maxYTS, DISP_HEIGHT, 0);
		} else {
			this->TouchX	= -1;
			this->TouchY	= -1;
		}
  }
  return;
};

void stateMachine::resetTouch(void){
	this->touchHoldCanceled	= false;
	this->touchPressHeld		= false;
  this->prevTouch					= false;
  this->currTouch					= false;
  this->TouchX						= 0;
  this->TouchY						= 0;
  return;
};

const bool stateMachine::getCurrTouch(void){
  return this->currTouch;
};

const bool stateMachine::getPrevTouch(void){
  return this->prevTouch;
};

const uint16_t stateMachine::getRawTouchX(void){
  return this->ts->getPoint().x;
};
const uint16_t stateMachine::getRawTouchY(void){
  return this->ts->getPoint().y;
};

const uint16_t stateMachine::getTouchX(void){
  return this->TouchX;
};

const uint16_t stateMachine::getTouchY(void){
  return this->TouchY;
};

// The value of touchPressHeld from the previous frame
const bool stateMachine::getPrevLongPress(void){
	return this->prevPressHeld;
};

// Returns true for ONE FRAME when touch input is first registered
const bool stateMachine::screenWasPressed(void){
	if (this->currTouch && !this->prevTouch){
#ifdef DEBUG
		Serial.println("SCREEN WAS PRESSED");
#endif
		return true;
	}
	else return false;
}

// Returns true for ONE FRAME when touch release is registered
const	bool stateMachine::screenWasReleased(void){
	if (!this->currTouch && this->prevTouch){
#ifdef DEBUG
		Serial.println("SCREEN WAS RELEASED");
#endif
		return true;
	}
	else return false;
}

// Returns true for ONE FRAME when a long press is first registered
const bool stateMachine::screenWasLongPressed(void){
	if (this->touchPressHeld && !this->prevPressHeld){
#ifdef DEBUG
		Serial.println("SCREEN WAS LONG PRESSED");
#endif
		return true;
	}
	else return false;
}

const bool stateMachine::getTouchPressHeld(void){
  return this->touchPressHeld;
};

const bool stateMachine::touchEnabled(void){
  return this->touchInputEnabled;
};

void stateMachine::disableTouchInput(void){
  this->touchInputEnabled = false;
  //this->updateTouchStatus();
  return;
};

void stateMachine::enableTouchInput(void){
  this->touchInputEnabled = true;
  //this->updateTouchStatus();
  return;
};

const bool stateMachine::drawingEnabled(void){
  return this->screenDrawingEnabled;
};

void stateMachine::disableDrawing(void){
  this->screenDrawingEnabled = false;
  return;
};

void stateMachine::enableDrawing(void){
  this->screenDrawingEnabled = true;
  return;
};

// Returns number of available outputs
const uint8_t stateMachine::getNumOutputs(void){
	return this->numOutputs;
};

// Returns address of SOLE active output, 255 otherwise
const uint8_t	stateMachine::getActiveOutput(void){
	for (uint8_t i = 0; i < MAX_OUTPUTS-1; i++)
		if (this->outputs[i] & 64) return i;

	return 255;
};

// Returns number of Active outputs
const	uint8_t	stateMachine::getNumActOutputs(void){

	// No outputs are active if passthrough is disabled
	//if (!this->getKeyStrokePassthrough()) return 0;

	uint8_t numActiveOutputs = 0;
	for (uint8_t i = 0; i < MAX_OUTPUTS-1; i++)
		if (this->outputs[i] & 64) numActiveOutputs++;

	return numActiveOutputs;
};

// Enables all available outputs
void stateMachine::enableAllOutputs(void){
	for (uint8_t i = 0; i < MAX_OUTPUTS-1; i++)
		if (this->outputs[i] & 128) this->outputs[i] |= 64;
	return;
};

// Disables all available outputs
void stateMachine::disableAllOutputs(void){
	for (uint8_t i = 0; i < MAX_OUTPUTS-1; i++)
		if (this->outputs[i] & 128) this->outputs[i] &= 191;
	return;
};

const bool stateMachine::getKeyStrokePassthrough(void){
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

// Enables output at address
void stateMachine::enableOutput(const uint8_t address){
	if (address > 127 || !(this->outputs[address] & 128)) {
#ifdef DEBUG
		Serial.print("Failed to enable output, invalid address: ");
		Serial.println(address);
#endif
		return;
	} else {
#ifdef DEBUG
		Serial.print("Enabling key output on address ");
		Serial.println(address);
		Serial.println(this->outputs[address], BIN);
#endif
		this->outputs[address] = this->outputs[address] | 64;
	}
	return;
};

// Disables output at address
void stateMachine::disableOutput(const uint8_t address){
	if (address > 127) return;
	else {
#ifdef DEBUG
		Serial.print("Disabling key output on address ");
		Serial.println(address);
		Serial.println(this->outputs[address], BIN);
#endif
		this->outputs[address] = this->outputs[address] & 191;
	}
	return;
};

// Returns true if output as address is enabled for output
const bool stateMachine::isOutputEnabled(const uint8_t address){
	if (address > 127) return false;
	else {
		if (this->outputs[address] & 64) return true;
		else return false;
	}
};

// Uses 1 bit in each byte to indicate if output at outputAddr is enabled
void stateMachine::getEnabledOutputs(uint8_t* byteAddresses){
	//uint8_t bitOffset		= 0;	// The index of bit 
	uint8_t opstat			= 0;	// Minor convenience variable
	uint8_t bitmap			= 0;	// The byte mask of bits indicating which outputs are enabled

	// Iterate over bytes
	for (uint8_t i = 0; i < 16; i++){
		// Iterate over bits in each byte
		for (uint8_t bitOffset = 0; bitOffset < 8; bitOffset++){
			opstat = this->outputs[i*8+bitOffset];
			// Set bit at bitOffset to true iff output is active and enabled
			if ((opstat & 128) && (opstat & 64)) bitmap |= 1<<bitOffset;
		}
		byteAddresses[i] = bitmap;
		bitmap = 0;
	}
	return;
}

// Uses 1 bit in each byte to indicate if output at outputAddr is enabled
void stateMachine::setEnabledOutputs(uint8_t* byteAddresses){
	//uint8_t bitOffset		= 0;	// The index of bit 
	uint8_t opstat			= 0;	// Minor convenience variable
	uint8_t bitmap			= byteAddresses[0];	// The byte mask of bits indicating which outputs are enabled

	// Iterate over bytes
	for (uint8_t i = 0; i < 16; i++){
		bitmap = byteAddresses[i];
		// Iterate over bits in each byte
		for (uint8_t bitOffset = 0; bitOffset < 8; bitOffset++){
			opstat = this->outputs[i*8+bitOffset];
			// If output is active enable it if bit is true
			if (opstat & 128){
				if ((1<<bitOffset) & bitmap){
					this->enableOutput(i*8 + bitOffset);
				} else {
					this->disableOutput(i*8 + bitOffset);
				}
			}
		}
	}
	return;
}

const uint8_t stateMachine::nextOutputAddress(const uint8_t address){
	return this->nextOutputAddress(address, 128);
	/*
	if (address > 127) return 0;
	uint8_t nextAddress = address + 1;
	while(!((this->outputs[nextAddress] | 127) & 128)){
		nextAddress++;
		if (nextAddress > 127) nextAddress = 0;
	}
	return nextAddress;
	*/
};

const uint8_t stateMachine::nextOutputAddress(const uint8_t address, const uint8_t capMask){
	if (address > 127) return 0;
	uint8_t nextAddress = address + 1;
	while(!((this->outputs[nextAddress] | !capMask) & capMask)){
		nextAddress++;
		if (nextAddress > 127) nextAddress = 0;
	}
	return nextAddress;
};

const uint8_t stateMachine::prevOutputAddress(const uint8_t address){
	return this->prevOutputAddress(address, 128);
	/*
	if (address > 127) return 0;
	uint8_t prevAddress = address - 1;
	while(!((this->outputs[prevAddress] | 127) & 128)){
		prevAddress--;
		if (prevAddress > 127) prevAddress = 127;
	}
	return prevAddress;
	*/
};

const uint8_t stateMachine::prevOutputAddress(const uint8_t address, const uint8_t capMask){
	if (address > 127) return 0;
	uint8_t prevAddress = address - 1;
	while(!((this->outputs[prevAddress] | !capMask) & capMask)){
		prevAddress--;
		if (prevAddress > 127) prevAddress = 127;
	}
	return prevAddress;
};

// Write a set of enabled outputs represented as 16-byte bitmap to shortcut slot
void stateMachine::assignShortcut(const uint8_t slotIndex, uint8_t* byteAddresses){
	for (uint8_t i = 0; i < 16; i++)
		this->enabledOutputSets[constrain(slotIndex, 0, NUM_OUTPUT_SHORTCUTS-1)][i] = byteAddresses[i];
	return;
};

// Recall a set of enabled outputs at slotIndex
void stateMachine::applyShortcut(const uint8_t slotIndex){	// Applies the shortcut at slot index
	if (this->isShortcutSet(slotIndex))
		this->setEnabledOutputs(this->enabledOutputSets[slotIndex]);
	return;
};

// Returns True if a shortcut is set in a given slot (also returns false if slotIndex is invalid)
const bool stateMachine::isShortcutSet(const uint8_t slotIndex){	
	if (slotIndex < 0 || slotIndex >= NUM_OUTPUT_SHORTCUTS) return false;
	else
		for (uint8_t i = 0; i < 16; i++)
			if (this->enabledOutputSets[slotIndex][i]) return true;
	return false;
};

// Returns the bitmask representing the capability of a given output at the specified address
const uint8_t stateMachine::getOutputCapability(const uint8_t address){
	return this->outputs[address];
};

// Returns true if a keyboard is connected (probably needs somework)
const bool stateMachine::keyboardConnected(void){
	return bool(this->kc->idVendor());
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
};

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
};

const bool stateMachine::getKeyPressHeld(void){
  return this->keyPressHeld;
};

void stateMachine::cancelKeyPressHold(void){
  this->keyHoldCanceled = true;
  this->keyPressHeld    = false;
  return;
};

void stateMachine::setModifiers(const uint8_t modifierMask){
  this->modifiers = modifierMask;
  return;
};

const uint8_t stateMachine::getModifiers(void){
  return this->modifiers;
};

// Utilities for controlling display backlight


// Get BL level constrained by min and max
const uint16_t stateMachine::getBLlev(void){
	return constrain(this->BLLevel, this->minBLLevel, this->maxBLLevel);
};

// Set sm backlight level
void  stateMachine::setBLlev(const uint16_t lightLevel){
	this->BLLevel = lightLevel;
	return;
};

void	stateMachine::setBLmin(const uint16_t lightLevel){
	this->minBLLevel = constrain(lightLevel, 0, this->maxBLLevel);
	return;
};

void  stateMachine::setBLmax(const uint16_t lightLevel){
	this->maxBLLevel = constrain(lightLevel, this->minBLLevel, 255);
	return;
};

// Set the physical pin the backlight is attached to
void	stateMachine::setBLpin(const uint8_t pin){
	this->BLpin = pin;
	return;
};

const uint16_t invert565color(const uint16_t color){
  const uint16_t ir = 31 - (color >> 11);
  const uint16_t ig = 63 - ((color & 2016) >> 5);
  const uint16_t ib = 31 - (color & 31);
  return ((ir << 11) | (ig << 5) | ib);
};
