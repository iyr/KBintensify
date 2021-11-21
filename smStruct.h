// Defines a stateMachine whose address 
// can be passed around to functions
// instead of many disparate function arguments
//

#ifndef _stateMachineClass_
#define _stateMachineClass_

#define DISP_WIDTH 320
#define DISP_HEIGHT 240

#define PRESSED_KEYS_BUFF_SIZE 10

#include <stdint.h>

#include <Adafruit_VCNL4040.h>      // Proximitiy/Ambient Light Sensor library
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include <SD.h>

#include "USBHost_t36.h"            // USB Host Library
#include <Wire.h>

#include <ArduinoJson.h>
#include <pgmspace.h>

#include "keyLookUp.h"

#define MAX_OUTPUTS 127
#define NUM_OUTPUT_SHORTCUTS 8

// Used for drawing dialogue windows over other controls
// !! back buffer uses ~%30 DMAMEM capacity on Teensy 4.x
//static uint16_t backBuff [DISP_WIDTH*DISP_HEIGHT] EXTMEM;
//static uint16_t wallpaper[DISP_WIDTH*DISP_HEIGHT] EXTMEM;
//static uint16_t frameBuff[DISP_WIDTH*DISP_HEIGHT] DMAMEM;
static uint16_t backBuff [DISP_WIDTH*DISP_HEIGHT] DMAMEM;
static uint16_t wallpaper[DISP_WIDTH*DISP_HEIGHT] DMAMEM;
static uint16_t frameBuff[DISP_WIDTH*DISP_HEIGHT] FASTRUN;

class stateMachine {

  public:
    stateMachine(void);
    ~stateMachine(void);

    ILI9341_t3n *tft            = NULL;
    SdFat *sd                   = NULL;
    KeyboardController *kc      = NULL;
    Adafruit_VCNL4040 *proxlux  = NULL;

    // buffer for storing temporary filepath information
    char* filePathBuff[256];

    // Utilities for statemachine management
    void      saveUserSettings(void);                 // Save settings to default path
    void      loadUserSettings(void);                 // Load settings from default path
    void      loadUserSettings(const char* filepath); // Load from defined path
    void      updateSM(void);                         // Update statemachine
    void      setTS(XPT2046_Touchscreen *tsPtr);      // Set touchscreen ptr
    void      setKC(KeyboardController  *kc);         // Set USBhost keyboard controller
    void      initDevices(void);                      // Init KBi peripherals inc outputs
    void      scanForOutputs(void);                   // Scan i2c bus for output devices
    void      sendKeyStroke(                          // Send keystroke to specified output
      const uint8_t   deviceaddress,  // device i2c address to send the keystroke
      const uint8_t   isPressed,      // differentiate press / release
      const uint8_t   modmask,        // modifier keys mask
      const uint16_t  key             // the actual key code (typ 1 byte for ascii, 2 bytes for hid extras)
      );
    void      passKeyToOutputs(       // Send keystroke to all receiving outputs
      const uint8_t   isPressed,      // differentiate press / release
      const uint8_t   modmask,        // modifier keys mask
      const uint16_t  key             // the actual key code (typ 1 byte for ascii, 2 bytes for hid extras)
      );

    // Set touchscreen calibration variables
    void      setCalibVars(const uint16_t minXts, const uint16_t minYts, 
                            const uint16_t maxXts, const uint16_t maxYts);

    // Get/Set current "applet" (screen) represented by a single byte
    void            setScreen(const uint8_t screen);
    const uint8_t   getScreen(void);

    // Utilities to facilitate screen double-buffering
    void            captureFrameBufferToBackBuffer(void); // saves current fbuff
    uint16_t*       getBackBuffer(void);  // returns a ptr to the back buffer

    // Utilities for getting system colors + complimentary colors
    const uint16_t  getPrimaryColor(void);  //  Mainly background color
    const uint16_t  getDetailsColor(void);  //  Mainly text color
    const uint16_t  getAverageColor(void);  //  mean of primary+secondary
    const uint16_t  getPrimaryColorInverted(void);  // expand color palette
    const uint16_t  getDetailsColorInverted(void);  // expand color palette

    // Utilities for handling and managing touchscreen input
    void            updateTouchStatus(void);  // Refreshes sm vars with ts vars
    void            resetTouch(void);         // Resets sm touch vars
    const bool      getCurrTouch(void);       // Gets this frame's touch status
    const bool      getPrevTouch(void);       // Gets prev frame's touch status
    const uint16_t  getRawTouchX(void);       // Gets raw x-coordinate from ts
    const uint16_t  getRawTouchY(void);       // Gets raw y-coordinate from ts
    const uint16_t  getTouchX(void);          // Gets calibrated x-coordinate
    const uint16_t  getTouchY(void);          // Gets calibrated y-coordinate
    const bool      getTouchPressHeld(void);  // Returns true if screen has been pressed and held at one point
    const bool      getPrevLongPress(void);   // The value of touchPressHeld from the previous frame
    const bool      screenWasPressed(void);   // Returns true for ONE FRAME when touch input is first registered
    const bool      screenWasReleased(void);  // Returns true for ONE FRAME when touch release is registered
    const bool      screenWasLongPressed(void); // Returns true for ONE FRAME when a long press is first registered
    const bool      touchEnabled(void);       // Whether or not touch is enabled
    void            disableTouchInput(void);  //
    void            enableTouchInput(void);   //
    const bool      getRepeatAction(void);    // Used for generating repititious inputs for long presses
    
    // Utilities for managing screen drawing
    const bool      drawingEnabled(void);
    void            disableDrawing(void);
    void            enableDrawing(void);

    // Utilities for setting/getting *general* keystroke passthrough
    const uint16_t  getKPM(void);     // Returns Keystrokes per minute
    void            incKeyEvents(void); // For measuring keystrokes per minute
    const uint8_t   getNumOutputs(void);  // Returns number of available outputs
    const uint8_t   getActiveOutput(void);  // Returns first lowest address of SOLE active outputs, 255 otherwise
    const uint8_t   getNumActOutputs(void);   // Returns number of Active outputs
    void            enableAllOutputs(void);     // Enables all available outputs
    void            disableAllOutputs(void);      // Disables all available outputs
    const bool      getKeyStrokePassthrough(void);  // Returns true if any output is receiving keystrokes
    void            enableKeyStrokePassthrough(void); // Enables passthrough
    void            disableKeyStrokePassthrough(void);  // Disables all passthrough
    void            enableOutput(const uint8_t address);  // Enables output at address
    void            disableOutput(const uint8_t address);   // Disables output at address
    const bool      isOutputEnabled(const uint8_t address);   // Returns true if output as address is enabled for output
    const uint8_t   getOutputCapability(const uint8_t address); // Returns Output capability byte
    void            getEnabledOutputs(uint8_t* byteAddresses);  // Uses 1 bit in each byte to indicate if output at index is enabled
    void            setEnabledOutputs(uint8_t* byteAddresses);  // Uses 1 bit in each byte to indicate if output at index is enabled
    const uint8_t   nextOutputAddress(const uint8_t address);   // Returns next highest address of available output
    const uint8_t   prevOutputAddress(const uint8_t address);   // Returns next lowest address of available output
    const uint8_t   nextOutputAddress(const uint8_t address, const uint8_t capMask);  // Returns next highest address of output filtered by capMask
    const uint8_t   prevOutputAddress(const uint8_t address, const uint8_t capMask);  // Returns next lowest address of output filtered by capMask

    void            assignShortcut(const uint8_t slotIndex, uint8_t* byteAddresses);
    void            applyShortcut(const uint8_t slotIndex); // Applies the shortcut at slot index
    const bool      isShortcutSet(const uint8_t slotIndex); // Returns True if a shortcut is set in a given slot

    // Utilities for handling and managing keystroke inputs
    const bool      keyboardConnected(void);                  // Returns true if a keyboard is connected to KBi
    void            incNumKeysPressed(void);                  // safely increment keys pressed
    void            decNumKeysPressed(void);                  // safely decrement keys pressed
    const uint8_t   getNumKeysPressed(void);                  // number of keys pressed
    uint16_t*       getPressedKeys(void);                     // returns ptr of pressed keys
    void            clearPressedKeys(void);                   // resets pressed key in sm
    void            clearReleasedKey(void);                   // resets released key in sm
    void            setPressedKey(const uint16_t key);        // set sm pressed key
    const uint16_t  getPressedKey(void);                      // gets sm pressed key
    void            setReleasedKey(const uint16_t key);       // set sm released key
    const uint16_t  getReleasedKey(void);                     // gets sm released key
    const bool      getKeyPressHeld(void);                    // returns true if key is held for some time
    void            cancelKeyPressHold(void);                 // resets sm held key
    void            setModifiers(const uint8_t modifierMask); // set sm modifier bools
    const uint8_t   getModifiers(void);                       // get sm modifier

    // Default size for home button icons
    const uint16_t  homeButtonRadX = 28;
    const uint16_t  homeButtonRadY = 24;
    
    // Utilities for controlling display backlight (BL)
    void            setBLrMode(const uint8_t mode);       // Set bitmask representing inputs that affect the BL
    void            setBLlev(const uint16_t lightLevel);  // Input value may exceed min / max
    void            setBLmin(const uint16_t lightLevel);
    void            setBLmax(const uint16_t lightLevel);
    void            setBLpin(const uint8_t pin);          // Set the physical pin the BL is attached to
    const uint16_t  getBLlev(void);                       // Get BL level constrained by min and max

    // Pointer to system wallpaper
    uint16_t*       userBackground  = wallpaper;
    uint16_t*       getWallpaperPtr(void);
    void            drawWallpaper(void);
  private:

    // Buffer of i2c Output devices+capabilities as Bit Masks
    // index 0 is central i2c device
    // Bit Map:
    // 0 (128) Output is available to receive keystrokes (always on for index 0)
    // 1 ( 64) Output is currently enabled for receiving keystrokes
    // 2 ( 32) Output Supports media keys
    // 3 ( 16) Output is a Bluetooth connection
    // 4 (  8) Reserved
    // 5 (  4) Reserved
    // 6 (  2) Reserved
    // 7 (  1) Reserved
    uint8_t outputs[MAX_OUTPUTS] = {0};
    uint8_t numOutputs = 1; // number of outputs picked up from scan

    // Uses 1 bit in a set of 16 bytes to represent enabled outputs
    // Primarily used for setting user shortcuts
    uint8_t enabledOutputSets[NUM_OUTPUT_SHORTCUTS][16] = {{0}};

    // Variables for controlling display backlight (BL)
    uint32_t  BLfuncTimer = millis(); // Used to check if its time to call BL function
    uint32_t  BLfuncRate  = 500;      // How often, in milliseconds, the BL function is called
    uint16_t  BLthresh    = 7;        // if BLfunc is greater than this, BL brightens, else BL darkens
    uint16_t  BLLevel     = 255;      // May exceed min / max range, getter method constrains to min / max
    uint16_t  minBLLevel  = 0;        // Min value getBLlev() will return
    uint16_t  maxBLLevel  = 255;      // Max value getBLlev() will return
    uint8_t   BLpin       = 0;        // The physical pin the BL is attached to
    uint8_t   BLrMode     = 128|64|32;// Bitmask representing which inputs the BL responds to
    // Bit Map:
    // 0 (128) BL responds to hand proximity (requires proximity sensor)
    // 1 ( 64) BL responds to keystrokes
    // 2 ( 32) BL responds to touch input
    // 3 ( 16) Reserved
    // 4 (  8) Reserved
    // 5 (  4) Reserved
    // 6 (  2) Reserved
    // 7 (  1) if set, BL is always on regardless of other bits/inputs

    // Pointer to system screen back buffer
    uint16_t*       backBuffer      = backBuff;

    // Vars related to parsing key inputs
    uint32_t        keyPressTimer   = 0;      // how long a key has been pressed
    uint32_t        kspmTimer       = 0;
    uint32_t        kspmTimer0      = 0;
    uint32_t        kspmTimerLast   = 0;
    uint16_t        kpmBuff[6]      = {0};
    const uint16_t  keyHoldTime     = 500;    // how long to hold a key to reg
    uint16_t        pressedKeys[PRESSED_KEYS_BUFF_SIZE];
    uint16_t        pressedKey      = 0;      // Most recently pressed key
    uint16_t        releasedKey     = 0;      // Most recently released key
    uint16_t        kspm            = 0;      // Number of keystrokes per minute
    uint16_t        numKeyEvents    = 0;      // Increments any time a key is pressed or released
    uint8_t         numKeysPressed  = 0;      // Number of current pressed keys
    uint8_t         modifiers       = 0;      // bitmask of active modifier keys
    bool            keyPressHeld    = false;  // key held long enough to repeat
    bool            keyHoldCanceled = false;  // cancels previous bool if true

    // System colors
    uint16_t        primaryColor    = ILI9341_PURPLE; // UI background color
    uint16_t        detailsColor    = ILI9341_ORANGE; // Text, UI details color
    //uint16_t        primaryColor    = 0x1082; // UI background color
    //uint16_t        detailsColor    = 0x171C; // Text, UI details color
    
    XPT2046_Touchscreen *ts;
    //uint16_t* bgImage; // background image / wallpaper

    uint8_t currentScreen = 1;  // Integer representing the current Screen
    uint8_t prevAppScreen = 1;  // Integer representing the last screen that wasn't home
    uint8_t cycleLastScreen[2] = {1};

    // Calibration variables
    // for mapping touch-sensor coordinates to 
    // screen-space coordinates and vice/versa
    uint16_t minXTS, minYTS, maxXTS, maxYTS;

    // Whether or not keystrokes are passed to host machine(s)
    bool      keyPassthrough = true;

    // Whether or touch input is processed
    bool      touchInputEnabled = true;

    // Whether or framebuffer is updated
    bool      screenDrawingEnabled = true;

    // Used for handling touchscreen inputs
    uint32_t  TouchUpdateRate   = 10;     // How often, in milliseconds, the touch status is updated
    uint32_t  TouchUpdateTimer  = 0;      // Time since last touch update
    uint32_t  touchPressTimer   = 0;      // how long the touch screen has been touched
    uint16_t  touchHoldRadius   = 25;     // Cursor must stay in position, measure in a circle to account for jitter
    uint16_t  touchHoldTime     = 500;    // how long to touch the touchscreen to trigger alternate function
    uint16_t  TouchX            = 0;      // calibrated X-coordinate of touch input
    uint16_t  TouchY            = 0;      // calibrated Y-coordinate of touch input
    uint16_t  TouchX0           = -1;     // calibrated X-coordinate of touch input at first frame of touch press
    uint16_t  TouchY0           = -1;     // calibrated Y-coordinate of touch input at first frame of touch press
    bool      touchPressHeld    = false;  // touch held long enough for alternate function
    bool      prevPressHeld     = false;  // Value of 'touchPressHeld' on previous frame
    bool      touchHoldCanceled = true;   // cancels previous bool if true
    bool      prevTouch         = false;  // touch sensor status @end of previous frame
    bool      currTouch         = false;  // touch sensor status @start of current frame
    
    // Used for making repititious inputs in the touch screen has been long pressed
    uint32_t  repeatActionTimer = 0;
    uint16_t  repeatActionDelay = 59;
    bool      repeatAction      = false;
};

const uint16_t invert565color(const uint16_t color);

#endif
