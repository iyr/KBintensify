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

  // Determine if touch sensor has been pressed long enough for alternate input
  if (this->currTouch && abs(millis() - this->touchPressTimer) >= this->touchHoldTime){
    this->touchPressHeld = !(this->touchHoldCanceled);
	}
  if (this->keyPassthrough){
  	// Determine if key has been pressed long enough to repeat
  	if (abs(millis() - this->keyPressTimer) >= this->keyHoldTime) 
    	this->keyPressHeld = !(this->keyHoldCanceled);

  	if (this->numKeysPressed == 0){ this->keyPressHeld = false; this->keyHoldCanceled = false;}

  	// Clear released key
  	this->releasedKey = 0;
  }
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
}

// Send keystroke to output device specified at address
void stateMachine::sendKeyStroke(
	const uint8_t		deviceAddress,	// device i2c address to send the keystroke
	const bool 			isPressed, 			// differentiate press / release
	const uint8_t 	modMask, 				// modifier keys mask
	const uint16_t	key							// the actual key code (typ 1 byte for ascii, 2 bytes for hid extras)
	) {
	const uint16_t arduKey = Tnsy2ArduKEY(key);
	// Create array of bytes to send keystrokes over i2c to auxiliary output devices (eg, usb, bluetooth)
	uint8_t keyStrokeBytes[5];
	keyStrokeBytes[0] = isPressed;								// 1: key press, 0: key release
	keyStrokeBytes[1] = modMask;									// second byte is the modifier mask
	keyStrokeBytes[2] = uint8_t((key >> 8) & -1);	// Parse key code into two bytes
	keyStrokeBytes[3] = uint8_t(key & -1);				// Parse key code into two bytes
	keyStrokeBytes[4] = uint8_t(arduKey);

	Wire.beginTransmission(deviceAddress);
	Wire.write(keyStrokeBytes, 5);
	Wire.endTransmission();
	return;
}

// Send keystroke to all receiving output devices
void stateMachine::passKeyToOutputs(
	const bool 			isPressed, 			// differentiate press / release
	const uint8_t 	modMask, 				// modifier keys mask
	const uint16_t	key							// the actual key code (typ 1 byte for ascii, 2 bytes for hid extras)
	) {
#ifdef DEBUG
	Serial.print(F("HID code (Host, Remap): "));
	Serial.print(key);
	Serial.print(F(", "));
	Serial.println(Tnsy2ArduKEY(key));
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
}

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
  currentScreen = constrain(screen, 0, 4);
  return;
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
  this->prevTouch = this->currTouch;
  this->currTouch = this->ts->touched();
  TS_Point p      = this->ts->getPoint();

	// Record calibrated XY coordinates the first frame the touch screen is pressed
	if (this->currTouch && !this->prevTouch) {	// First frame screen is touched
  	this->touchPressHeld  	= false;
		this->touchHoldCanceled = false;
  	this->touchPressTimer		= millis();
    this->TouchX0 = map(p.x, this->minXTS, this->maxXTS, 0, DISP_WIDTH	);
    this->TouchY0 = map(p.y, this->minYTS, this->maxYTS, DISP_HEIGHT, 0	);
    this->TouchX  = this->TouchX0;
    this->TouchY  = this->TouchY0;
	} 

	// Cancel touch hold if cursor leaves small circle where touch input was first registered
	if (!this->currTouch || 
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

	if (!this->currTouch && this-prevTouch) {	// Frist frame touch is released from screen
		this->TouchX0 = -1;
		this->TouchY0 = -1;
	}
  if (this->currentScreen > 0){
    this->TouchX  = map(p.x, this->minXTS, this->maxXTS, 0, DISP_WIDTH);
    this->TouchY  = map(p.y, this->minYTS, this->maxYTS, DISP_HEIGHT, 0);
  }
  return;
};

void stateMachine::resetTouch(void){
	this->touchHoldCanceled	= false;
	this->touchPressHeld 		= false;
  this->prevTouch 				= false;
  this->currTouch 				= false;
  this->TouchX    				= 0;
  this->TouchY    				= 0;
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
}

// Enables all available outputs
void stateMachine::enableAllOutputs(void){
	for (uint8_t i = 0; i < MAX_OUTPUTS-1; i++)
		if (this->outputs[i] & 128) this->outputs[i] |= 64;
	return;
}

// Disables all available outputs
void stateMachine::disableAllOutputs(void){
	for (uint8_t i = 0; i < MAX_OUTPUTS-1; i++)
		if (this->outputs[i] & 128) this->outputs[i] &= 191;
	return;
}

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
	if (address > 127) return;
	else {
#ifdef DEBUG
		Serial.print("Enabling key output on address ");
		Serial.println(address);
		Serial.println(this->outputs[address], BIN);
#endif
		this->outputs[address] = this->outputs[address] | 64;
	}
	return;
}

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
}

// Returns true if output as address is enabled for output
const bool stateMachine::isOutputEnabled(const uint8_t address){
	if (address > 127) return false;
	else {
		if (this->outputs[address] & 64) return true;
		else return false;
	}
}

const uint8_t stateMachine::nextOutputAddress(const uint8_t address){
	if (address > 127) return 0;
	uint8_t nextAddress = address + 1;
	while(!((this->outputs[nextAddress] | 127) & 128)){
		nextAddress++;
		if (nextAddress > 127) nextAddress = 0;
	}
	return nextAddress;
}

const uint8_t stateMachine::prevOutputAddress(const uint8_t address){
	if (address > 127) return 0;
	uint8_t prevAddress = address - 1;
	while(!((this->outputs[prevAddress] | 127) & 128)){
		prevAddress--;
		if (prevAddress > 127) prevAddress = 127;
	}
	return prevAddress;
}

void stateMachine::updateInputKeys(void){
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

const uint16_t invert565color(const uint16_t color){
  const uint16_t ir = 31 - (color >> 11);
  const uint16_t ig = 63 - ((color & 2016) >> 5);
  const uint16_t ib = 31 - (color & 31);
  return ((ir << 11) | (ig << 5) | ib);
};
