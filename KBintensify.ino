/*
 * KB-Intensify Firmware
 */
#include "USBHost_t36.h"            // USB Host Library
#define TEENSY64
#include <ILI9341_t3n.h>            // Display Library
#include <ili9341_t3n_font_Arial.h> // Font
#include <XPT2046_Touchscreen.h>    // Touchscreen sensor Library
#include <SPI.h>                    // Communication for SPI devices
#include <SdFat.h>                  // SD card Filesystem
#include <SdFatConfig.h>
#include <sdios.h>
#include <TimeLib.h>                // Real-time clock Library
#include "KButils.h"                // KB-Intensify utilities

#define CS_PIN  8
#define TFT_DC  9
#define TFT_CS  10
#define TFT_RST 24
// MOSI=11, MISO=12, SCK=13
#define TIRQ_PIN  2

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
#endif  // HAS_SDIO_CLASS
//------------------------------------------------------------------------------

//#if SD_FAT_TYPE == 0
SdFat gsd;
File gfile;
File groot;
//#elif SD_FAT_TYPE == 1
//SdFat32 gsd;
//File32 file;
//File32 root;
//#elif SD_FAT_TYPE == 2
//SdExFat gsd;
//ExFile file;
//ExFile root;
//#elif SD_FAT_TYPE == 3
//SdFs gsd;
//FsFile file;
//FsFile root;
//#endif  // SD_FAT_TYPE

// Create a Serial output stream.
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) gsd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------
//stateMachine sm;
stateMachine sm = stateMachine();
XPT2046_Touchscreen gts  = XPT2046_Touchscreen(CS_PIN);
ILI9341_t3n gtft         = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);
EXTMEM char fileNameBuff[MAX_FILEPATH_LENGTH];
EXTMEM uint16_t wallpaper[DISP_WIDTH*DISP_HEIGHT];
EXTMEM char** fileListBuff;

//=============================================================================
// USB Host Ojbects
//=============================================================================
USBHost myusb;
USBHub hub1(myusb);
KeyboardController keyboard1(myusb);
USBHIDParser hid1(myusb); // Needed for parsing hid extras keys
USBHIDParser hid2(myusb); // Needed for parsing hid extras keys
USBDriver *drivers[]                  = {&keyboard1, &hid1, &hid2};
#define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))
const char *driver_names[CNT_DEVICES] = {"KB1", "HID2", "HID2"};
bool driver_active[CNT_DEVICES]       = {false, false, false};

// Array of function pointers, passing only the statemachine
// functions are called by index
void (* SCREENS[4]) (stateMachine*);

void doQuack(stateMachine* sm){
  doHomeScreen(sm);
  doCrosshairDemo(sm);
}

void setup() {
  Serial.begin(115200);

  if (!gsd.begin(SD_CONFIG)) 
    Serial.println("SD card initialization failed");

  // Initialize State Machine variables
  sm.sd   = &gsd;
  sm.tft  = &gtft;
  sm.setTS(&gts);
  sm.setScreen(SCREEN_CALIBRATION);
  
  //sm.currentScreen = SCREEN_CALIBRATION;
  //sm.bgImage = wallpaper;

  // Initialize USB/Keyboard Objects
  myusb.begin();
  keyboard1.attachRawPress(OnRawPress);
  keyboard1.attachRawRelease(OnRawRelease);
  keyboard1.attachExtrasPress(OnHIDExtrasPress);
  keyboard1.attachExtrasRelease(OnHIDExtrasRelease);

  // Initialize function pointers
  SCREENS[SCREEN_CALIBRATION] = doCalibrate;
  SCREENS[SCREEN_HOME]        = doHomeScreen;
  //SCREENS[SCREEN_HOME]        = doQuack;
  SCREENS[SCREEN_IMAGEVIEWER] = doImageViewer;
  SCREENS[SCREEN_PASSWORDMAN] = doPassMan;
  //SCREENS[SCREEN_CROSSHAIRDEMO] = doCrosshairDemo;
   
  // Set RTC, run saftey check
  setSyncProvider(getTeensy3Time);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  // Initialize Display and Touch Sensor
  sm.initDevices();
  if (sm.sd->exists("/userSettings.cfg")){
    sm.loadUserSettings("/userSettings.cfg");
    sm.setScreen(SCREEN_HOME);
  }

  // 
  sm.tft->fillScreen(ILI9341_BLUE);
  sm.tft->updateScreen();
  sm.tft->fillScreen(ILI9341_PINK);
  sm.tft->updateScreen();
  sm.tft->fillScreen(ILI9341_WHITE);
  sm.tft->updateScreen();
  sm.tft->fillScreen(ILI9341_PINK);
  sm.tft->updateScreen();
  sm.tft->fillScreen(ILI9341_BLUE);
  sm.tft->updateScreen();
  while (!Serial && (millis() <= 1000));

  fileListBuff = (char **)malloc(MAX_FILELIST_LENGTH*sizeof(char *));
  for (uint32_t i = 0; i < MAX_FILELIST_LENGTH; i++){
    fileListBuff[i] = (char *)malloc(MAX_FILENAME_LENGTH*sizeof(char));
    memset(fileListBuff[i], '\0', MAX_FILENAME_LENGTH);
  }
}

void loop() {
  //sm.pressedKey = 0;
  //sm.updateInputKeys();
  if (sm.getCurrTouch() && sm.getScreen() != 0 ) {
    Serial.print(sm.getTouchX());
    Serial.print(F(" "));
    Serial.println(sm.getTouchY());
  }
  if (sm.getNumKeysPressed() == 0) sm.clearPressedKeys();
  (* SCREENS[sm.getScreen()])(&sm);
  sm.updateTouchStatus();
  sm.updateSM();
  drawStatusBars(&sm);
  if (sm.drawingEnabled()) sm.tft->updateScreen();
  sm.captureFrameBufferToBackBuffer();
}

void doCrosshairDemo(stateMachine* sm) {
  const uint16_t TouchX = sm->getTouchX();
  const uint16_t TouchY = sm->getTouchY();

  if (sm->getCurrTouch() && sm->getPrevTouch()) {
    // draw new cross
    drawFullCross(TouchX, TouchY, sm->getDetailsColor(), sm->tft);
  }
  return;
}

void OnRawPress(int key){
  const uint8_t modMask = getModMask(key, true);
  if (sm.getKeyStrokePassthrough()) {
    Keyboard.press(HID2ArduKEY(key, false));
    //Keyboard.set_modifier(modMask); // true to enable modifier if pressed
  }
  sm.incNumKeysPressed();
  sm.setModifiers(modMask);
  sm.setPressedKey(key);

  // Use Escape to "unstick" buggersome key inputs
  if (HID2ArduKEY(key) == KEY_ESC){ Keyboard.releaseAll(); Keyboard.set_modifier(0); }

  //Serial.print("HID PRS: ");
  //Serial.println(key);

  return;
}

void OnRawRelease(int key){
  const uint8_t modMask = getModMask(key, false);
  if (sm.getKeyStrokePassthrough()) {
    Keyboard.release(HID2ArduKEY(key, false));
    //Keyboard.set_modifier(modMask); // false to disable modifier if released
  }
  sm.decNumKeysPressed();
  sm.setModifiers(modMask);
  sm.setReleasedKey(key);

  //Serial.print("HID REL: ");
  //Serial.println(key);
  
  return;
}

void OnHIDExtrasPress(uint32_t top, uint16_t key)
{
  const uint8_t modMask = getModMask(key, true);
  if (sm.getKeyStrokePassthrough()) {
    Keyboard.set_modifier(modMask);
    Keyboard.press(key | 0xE400);
  } else {
  sm.incNumKeysPressed();
  sm.setModifiers(modMask);
  sm.setPressedKey(key);
  }
  return;
}

void OnHIDExtrasRelease(uint32_t top, uint16_t key)
{
  const uint8_t modMask = getModMask(key, false);
  if (sm.getKeyStrokePassthrough()) {
    Keyboard.set_modifier(modMask);
    Keyboard.release(key | 0xE400);
  } else {
  sm.decNumKeysPressed();
  sm.setModifiers(modMask);
  sm.setReleasedKey(key);
  }
  return;
}

