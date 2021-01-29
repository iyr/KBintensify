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

#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

#include <ArduinoJson.h>
#include <pgmspace.h>

#include "keyLookUp.h"

// Used for drawing dialogue windows over other controls
// !! back buffer uses ~%30 DMAMEM capacity on Teensy 4.x
static uint16_t backBuff[DISP_WIDTH*DISP_HEIGHT*2] DMAMEM;

class stateMachine {

  public:
    stateMachine(void);
    ~stateMachine(void);

    ILI9341_t3n *tft;
    SdFat *sd;

    // buffer for storing temporary filepath information
    char* filePathBuff[256];

    void      saveUserSettings(void);
    void      loadUserSettings(void);
    void      loadUserSettings(const char* filepath);
    //void      saveUserSettings(const char* filepath);
    //void      loadUserSettings(const char* filepath);
    void      updateSM(void);
    void      setTS(XPT2046_Touchscreen *tsPtr);
    void      initDevices(void);
    void      setCalibVars(const uint16_t minXts, const uint16_t minYts, 
                            const uint16_t maxXts, const uint16_t maxYts);

    void            setScreen(const uint8_t screen);
    const uint8_t   getScreen(void);

    void            captureFrameBufferToBackBuffer(void);
    uint16_t*       getBackBuffer(void);

    const uint16_t  getPrimaryColor(void);
    const uint16_t  getDetailsColor(void);
    const uint16_t  getAverageColor(void);
    const uint16_t  getPrimaryColorInverted(void);
    const uint16_t  getDetailsColorInverted(void);

    void            updateTouchStatus(void);
    void            resetTouch(void);
    const bool      getCurrTouch(void);
    const bool      getPrevTouch(void);
    const uint16_t  getRawTouchX(void);
    const uint16_t  getRawTouchY(void);
    const uint16_t  getTouchX(void);
    const uint16_t  getTouchY(void);
    
    const bool      touchEnabled(void);
    void            disableTouchInput(void);
    void            enableTouchInput(void);

    const bool      drawingEnabled(void);
    void            disableDrawing(void);
    void            enableDrawing(void);

    const bool      getKeyStrokePassthrough(void);
    void            enableKeyStrokePassthrough(void);
    void            disableKeyStrokePassthrough(void);

    void            updateInputKeys(void);
    void            incNumKeysPressed(void);
    void            decNumKeysPressed(void);
    const uint8_t   getNumKeysPressed(void);
    uint16_t*       getPressedKeys(void);
    void            clearPressedKeys(void);
    void            clearReleasedKey(void);
    void            setPressedKey(const uint16_t key);
    const uint16_t  getPressedKey(void);
    void            setReleasedKey(const uint16_t key);
    const uint16_t  getReleasedKey(void);
    const bool      getKeyPressHeld(void);
    void            cancelKeyPressHold(void);
    void            setModifiers(const uint8_t modifierMask);
    const uint8_t   getModifiers(void);

    const uint16_t  homeButtonRadX = 28;
    const uint16_t  homeButtonRadY = 24;
  private:

    // 
    uint16_t*       backBuffer      = backBuff;

    // Vars related to parsing key inputs
    uint16_t        pressedKeys[PRESSED_KEYS_BUFF_SIZE];
    uint16_t        pressedKey      = 0;      // Most recently pressed key
    uint16_t        releasedKey     = 0;      // Most recently released key
    uint8_t         numKeysPressed  = 0;      // Number of current pressed keys
    const uint16_t  keyHoldTime     = 500;    // how long to hold a key to reg
    uint32_t        keyPressTimer   = 0;      // how long a key has been pressed
    bool            keyPressHeld    = false;  // key held long enough to repeat
    bool            keyHoldCanceled = false;  // cancels previous bool if true
    uint8_t         modifiers       = 0;      // bitmask of active modifier keys

    // System colors
    uint16_t        primaryColor    = ILI9341_PURPLE; // UI background color
    uint16_t        detailsColor    = ILI9341_ORANGE; // Text, UI details color

    XPT2046_Touchscreen *ts;
    //uint16_t* bgImage; // background image / wallpaper

    uint8_t currentScreen = 0;

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
    bool      prevTouch = false;  // touch sensor status @end of previous frame
    bool      currTouch = false;  // touch sensor status @start of current frame
    uint16_t  TouchX    = 0;      // calibrated X-coordinate of touch input
    uint16_t  TouchY    = 0;      // calibrated Y-coordinate of touch input
};

const uint16_t invert565color(const uint16_t color);

#endif
