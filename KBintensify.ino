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
  sm.updateTouchStatus();
  if (sm.getCurrTouch()) {
    Serial.println(sm.getTouchX());
    Serial.println(sm.getTouchY());
  }
  (* SCREENS[sm.getScreen()])(&sm);
  drawStatusBars(&sm);
  sm.tft->updateScreen();
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
  if (sm.getKeyStrokePassthrough()) {
    Keyboard.press(HID2ArduKEY(key));
    Keyboard.set_modifier(setMods(key, true)); // true to enable modifier if pressed
  }

  // Use Escape to "unstick" buggersome key inputs
  if (HID2ArduKEY(key) == KEY_ESC){ Keyboard.releaseAll(); Keyboard.set_modifier(0); }

  //Serial.print("HID PRS: ");
  //Serial.println(key);

  return;
}

void OnRawRelease(int key){
  if (sm.getKeyStrokePassthrough()) {
    Keyboard.release(HID2ArduKEY(key));
    Keyboard.set_modifier(setMods(key, false)); // false to disable modifier if released
  }

  //Serial.print("HID REL: ");
  //Serial.println(key);
  
  return;
}

void OnHIDExtrasPress(uint32_t top, uint16_t key)
{
  //Serial.print(key);
  /*
  if (top == 0xc0000) {
    switch (key) {
      case  0x20 : Serial.println(" - +10"); break;
      case  0x21 : Serial.println(" - +100"); break;
      case  0x22 : Serial.println(" - AM/PM"); break;
      case  0x30 : Serial.println(" - Power"); break;
      case  0x31 : Serial.println(" - Reset"); break;
      case  0x32 : Serial.println(" - Sleep"); break;
      case  0x33 : Serial.println(" - Sleep After"); break;
      case  0x34 : Serial.println(" - Sleep Mode"); break;
      case  0x35 : Serial.println(" - Illumination"); break;
      case  0x36 : Serial.println(" - Function Buttons"); break;
      case  0x40 : Serial.println(" - Menu"); break;
      case  0x41 : Serial.println(" - Menu  Pick"); break;
      case  0x42 : Serial.println(" - Menu Up"); break;
      case  0x43 : Serial.println(" - Menu Down"); break;
      case  0x44 : Serial.println(" - Menu Left"); break;
      case  0x45 : Serial.println(" - Menu Right"); break;
      case  0x46 : Serial.println(" - Menu Escape"); break;
      case  0x47 : Serial.println(" - Menu Value Increase"); break;
      case  0x48 : Serial.println(" - Menu Value Decrease"); break;
      case  0x60 : Serial.println(" - Data On Screen"); break;
      case  0x61 : Serial.println(" - Closed Caption"); break;
      case  0x62 : Serial.println(" - Closed Caption Select"); break;
      case  0x63 : Serial.println(" - VCR/TV"); break;
      case  0x64 : Serial.println(" - Broadcast Mode"); break;
      case  0x65 : Serial.println(" - Snapshot"); break;
      case  0x66 : Serial.println(" - Still"); break;
      case  0x80 : Serial.println(" - Selection"); break;
      case  0x81 : Serial.println(" - Assign Selection"); break;
      case  0x82 : Serial.println(" - Mode Step"); break;
      case  0x83 : Serial.println(" - Recall Last"); break;
      case  0x84 : Serial.println(" - Enter Channel"); break;
      case  0x85 : Serial.println(" - Order Movie"); break;
      case  0x86 : Serial.println(" - Channel"); break;
      case  0x87 : Serial.println(" - Media Selection"); break;
      case  0x88 : Serial.println(" - Media Select Computer"); break;
      case  0x89 : Serial.println(" - Media Select TV"); break;
      case  0x8A : Serial.println(" - Media Select WWW"); break;
      case  0x8B : Serial.println(" - Media Select DVD"); break;
      case  0x8C : Serial.println(" - Media Select Telephone"); break;
      case  0x8D : Serial.println(" - Media Select Program Guide"); break;
      case  0x8E : Serial.println(" - Media Select Video Phone"); break;
      case  0x8F : Serial.println(" - Media Select Games"); break;
      case  0x90 : Serial.println(" - Media Select Messages"); break;
      case  0x91 : Serial.println(" - Media Select CD"); break;
      case  0x92 : Serial.println(" - Media Select VCR"); break;
      case  0x93 : Serial.println(" - Media Select Tuner"); break;
      case  0x94 : Serial.println(" - Quit"); break;
      case  0x95 : Serial.println(" - Help"); break;
      case  0x96 : Serial.println(" - Media Select Tape"); break;
      case  0x97 : Serial.println(" - Media Select Cable"); break;
      case  0x98 : Serial.println(" - Media Select Satellite"); break;
      case  0x99 : Serial.println(" - Media Select Security"); break;
      case  0x9A : Serial.println(" - Media Select Home"); break;
      case  0x9B : Serial.println(" - Media Select Call"); break;
      case  0x9C : Serial.println(" - Channel Increment"); break;
      case  0x9D : Serial.println(" - Channel Decrement"); break;
      case  0x9E : Serial.println(" - Media Select SAP"); break;
      case  0xA0 : Serial.println(" - VCR Plus"); break;
      case  0xA1 : Serial.println(" - Once"); break;
      case  0xA2 : Serial.println(" - Daily"); break;
      case  0xA3 : Serial.println(" - Weekly"); break;
      case  0xA4 : Serial.println(" - Monthly"); break;
      case  0xB0 : Serial.println(" - Play"); break;
      case  0xB1 : Serial.println(" - Pause"); break;
      case  0xB2 : Serial.println(" - Record"); break;
      case  0xB3 : Serial.println(" - Fast Forward"); break;
      case  0xB4 : Serial.println(" - Rewind"); break;
      case  0xB5 : Serial.println(" - Scan Next Track"); break;
      case  0xB6 : Serial.println(" - Scan Previous Track"); break;
      case  0xB7 : Serial.println(" - Stop"); break;
      case  0xB8 : Serial.println(" - Eject"); break;
      case  0xB9 : Serial.println(" - Random Play"); break;
      case  0xBA : Serial.println(" - Select DisC"); break;
      case  0xBB : Serial.println(" - Enter Disc"); break;
      case  0xBC : Serial.println(" - Repeat"); break;
      case  0xBD : Serial.println(" - Tracking"); break;
      case  0xBE : Serial.println(" - Track Normal"); break;
      case  0xBF : Serial.println(" - Slow Tracking"); break;
      case  0xC0 : Serial.println(" - Frame Forward"); break;
      case  0xC1 : Serial.println(" - Frame Back"); break;
      case  0xC2 : Serial.println(" - Mark"); break;
      case  0xC3 : Serial.println(" - Clear Mark"); break;
      case  0xC4 : Serial.println(" - Repeat From Mark"); break;
      case  0xC5 : Serial.println(" - Return To Mark"); break;
      case  0xC6 : Serial.println(" - Search Mark Forward"); break;
      case  0xC7 : Serial.println(" - Search Mark Backwards"); break;
      case  0xC8 : Serial.println(" - Counter Reset"); break;
      case  0xC9 : Serial.println(" - Show Counter"); break;
      case  0xCA : Serial.println(" - Tracking Increment"); break;
      case  0xCB : Serial.println(" - Tracking Decrement"); break;
      case  0xCD : Serial.println(" - Pause/Continue"); break;
      case  0xE0 : Serial.println(" - Volume"); break;
      case  0xE1 : Serial.println(" - Balance"); break;
      case  0xE2 : Serial.println(" - Mute"); break;
      case  0xE3 : Serial.println(" - Bass"); break;
      case  0xE4 : Serial.println(" - Treble"); break;
      case  0xE5 : Serial.println(" - Bass Boost"); break;
      case  0xE6 : Serial.println(" - Surround Mode"); break;
      case  0xE7 : Serial.println(" - Loudness"); break;
      case  0xE8 : Serial.println(" - MPX"); break;
      case  0xE9 : Serial.println(" - Volume Up"); break;
      case  0xEA : Serial.println(" - Volume Down"); break;
      case  0xF0 : Serial.println(" - Speed Select"); break;
      case  0xF1 : Serial.println(" - Playback Speed"); break;
      case  0xF2 : Serial.println(" - Standard Play"); break;
      case  0xF3 : Serial.println(" - Long Play"); break;
      case  0xF4 : Serial.println(" - Extended Play"); break;
      case  0xF5 : Serial.println(" - Slow"); break;
      case  0x100: Serial.println(" - Fan Enable"); break;
      case  0x101: Serial.println(" - Fan Speed"); break;
      case  0x102: Serial.println(" - Light"); break;
      case  0x103: Serial.println(" - Light Illumination Level"); break;
      case  0x104: Serial.println(" - Climate Control Enable"); break;
      case  0x105: Serial.println(" - Room Temperature"); break;
      case  0x106: Serial.println(" - Security Enable"); break;
      case  0x107: Serial.println(" - Fire Alarm"); break;
      case  0x108: Serial.println(" - Police Alarm"); break;
      case  0x150: Serial.println(" - Balance Right"); break;
      case  0x151: Serial.println(" - Balance Left"); break;
      case  0x152: Serial.println(" - Bass Increment"); break;
      case  0x153: Serial.println(" - Bass Decrement"); break;
      case  0x154: Serial.println(" - Treble Increment"); break;
      case  0x155: Serial.println(" - Treble Decrement"); break;
      case  0x160: Serial.println(" - Speaker System"); break;
      case  0x161: Serial.println(" - Channel Left"); break;
      case  0x162: Serial.println(" - Channel Right"); break;
      case  0x163: Serial.println(" - Channel Center"); break;
      case  0x164: Serial.println(" - Channel Front"); break;
      case  0x165: Serial.println(" - Channel Center Front"); break;
      case  0x166: Serial.println(" - Channel Side"); break;
      case  0x167: Serial.println(" - Channel Surround"); break;
      case  0x168: Serial.println(" - Channel Low Frequency Enhancement"); break;
      case  0x169: Serial.println(" - Channel Top"); break;
      case  0x16A: Serial.println(" - Channel Unknown"); break;
      case  0x170: Serial.println(" - Sub-channel"); break;
      case  0x171: Serial.println(" - Sub-channel Increment"); break;
      case  0x172: Serial.println(" - Sub-channel Decrement"); break;
      case  0x173: Serial.println(" - Alternate Audio Increment"); break;
      case  0x174: Serial.println(" - Alternate Audio Decrement"); break;
      case  0x180: Serial.println(" - Application Launch Buttons"); break;
      case  0x181: Serial.println(" - AL Launch Button Configuration Tool"); break;
      case  0x182: Serial.println(" - AL Programmable Button Configuration"); break;
      case  0x183: Serial.println(" - AL Consumer Control Configuration"); break;
      case  0x184: Serial.println(" - AL Word Processor"); break;
      case  0x185: Serial.println(" - AL Text Editor"); break;
      case  0x186: Serial.println(" - AL Spreadsheet"); break;
      case  0x187: Serial.println(" - AL Graphics Editor"); break;
      case  0x188: Serial.println(" - AL Presentation App"); break;
      case  0x189: Serial.println(" - AL Database App"); break;
      case  0x18A: Serial.println(" - AL Email Reader"); break;
      case  0x18B: Serial.println(" - AL Newsreader"); break;
      case  0x18C: Serial.println(" - AL Voicemail"); break;
      case  0x18D: Serial.println(" - AL Contacts/Address Book"); break;
      case  0x18E: Serial.println(" - AL Calendar/Schedule"); break;
      case  0x18F: Serial.println(" - AL Task/Project Manager"); break;
      case  0x190: Serial.println(" - AL Log/Journal/Timecard"); break;
      case  0x191: Serial.println(" - AL Checkbook/Finance"); break;
      case  0x192: Serial.println(" - AL Calculator"); break;
      case  0x193: Serial.println(" - AL A/V Capture/Playback"); break;
      case  0x194: Serial.println(" - AL Local Machine Browser"); break;
      case  0x195: Serial.println(" - AL LAN/WAN Browser"); break;
      case  0x196: Serial.println(" - AL Internet Browser"); break;
      case  0x197: Serial.println(" - AL Remote Networking/ISP Connect"); break;
      case  0x198: Serial.println(" - AL Network Conference"); break;
      case  0x199: Serial.println(" - AL Network Chat"); break;
      case  0x19A: Serial.println(" - AL Telephony/Dialer"); break;
      case  0x19B: Serial.println(" - AL Logon"); break;
      case  0x19C: Serial.println(" - AL Logoff"); break;
      case  0x19D: Serial.println(" - AL Logon/Logoff"); break;
      case  0x19E: Serial.println(" - AL Terminal Lock/Screensaver"); break;
      case  0x19F: Serial.println(" - AL Control Panel"); break;
      case  0x1A0: Serial.println(" - AL Command Line Processor/Run"); break;
      case  0x1A1: Serial.println(" - AL Process/Task Manager"); break;
      case  0x1A2: Serial.println(" - AL Select Tast/Application"); break;
      case  0x1A3: Serial.println(" - AL Next Task/Application"); break;
      case  0x1A4: Serial.println(" - AL Previous Task/Application"); break;
      case  0x1A5: Serial.println(" - AL Preemptive Halt Task/Application"); break;
      case  0x200: Serial.println(" - Generic GUI Application Controls"); break;
      case  0x201: Serial.println(" - AC New"); break;
      case  0x202: Serial.println(" - AC Open"); break;
      case  0x203: Serial.println(" - AC Close"); break;
      case  0x204: Serial.println(" - AC Exit"); break;
      case  0x205: Serial.println(" - AC Maximize"); break;
      case  0x206: Serial.println(" - AC Minimize"); break;
      case  0x207: Serial.println(" - AC Save"); break;
      case  0x208: Serial.println(" - AC Print"); break;
      case  0x209: Serial.println(" - AC Properties"); break;
      case  0x21A: Serial.println(" - AC Undo"); break;
      case  0x21B: Serial.println(" - AC Copy"); break;
      case  0x21C: Serial.println(" - AC Cut"); break;
      case  0x21D: Serial.println(" - AC Paste"); break;
      case  0x21E: Serial.println(" - AC Select All"); break;
      case  0x21F: Serial.println(" - AC Find"); break;
      case  0x220: Serial.println(" - AC Find and Replace"); break;
      case  0x221: Serial.println(" - AC Search"); break;
      case  0x222: Serial.println(" - AC Go To"); break;
      case  0x223: Serial.println(" - AC Home"); break;
      case  0x224: Serial.println(" - AC Back"); break;
      case  0x225: Serial.println(" - AC Forward"); break;
      case  0x226: Serial.println(" - AC Stop"); break;
      case  0x227: Serial.println(" - AC Refresh"); break;
      case  0x228: Serial.println(" - AC Previous Link"); break;
      case  0x229: Serial.println(" - AC Next Link"); break;
      case  0x22A: Serial.println(" - AC Bookmarks"); break;
      case  0x22B: Serial.println(" - AC History"); break;
      case  0x22C: Serial.println(" - AC Subscriptions"); break;
      case  0x22D: Serial.println(" - AC Zoom In"); break;
      case  0x22E: Serial.println(" - AC Zoom Out"); break;
      case  0x22F: Serial.println(" - AC Zoom"); break;
      case  0x230: Serial.println(" - AC Full Screen View"); break;
      case  0x231: Serial.println(" - AC Normal View"); break;
      case  0x232: Serial.println(" - AC View Toggle"); break;
      case  0x233: Serial.println(" - AC Scroll Up"); break;
      case  0x234: Serial.println(" - AC Scroll Down"); break;
      case  0x235: Serial.println(" - AC Scroll"); break;
      case  0x236: Serial.println(" - AC Pan Left"); break;
      case  0x237: Serial.println(" - AC Pan Right"); break;
      case  0x238: Serial.println(" - AC Pan"); break;
      case  0x239: Serial.println(" - AC New Window"); break;
      case  0x23A: Serial.println(" - AC Tile Horizontally"); break;
      case  0x23B: Serial.println(" - AC Tile Vertically"); break;
      case  0x23C: Serial.println(" - AC Format"); break;
    } 
  }
  */

  if (sm.getKeyStrokePassthrough()) {
    Keyboard.set_modifier(setMods(key, true));
    Keyboard.press(key | 0xE400);
  }
  return;
}

void OnHIDExtrasRelease(uint32_t top, uint16_t key)
{
  if (sm.getKeyStrokePassthrough()) {
    Keyboard.set_modifier(setMods(key, false));
    Keyboard.release(key | 0xE400);
  }
  return;
}

