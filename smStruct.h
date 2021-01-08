// Defines a stateMachine whose address 
// can be passed around to functions
// instead of many disparate function arguments
//

#ifndef _stateMachineClass_
#define _stateMachineClass_

#define DISP_WIDTH 320
#define DISP_HEIGHT 240

#include <stdint.h>

#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

class stateMachine {

  public:
    stateMachine(void);
    ~stateMachine(void);

    ILI9341_t3n *tft;
    SdFat *sd;

    // buffer for storing temporary filepath information
    char* filePathBuff[256];

    void      setTS(XPT2046_Touchscreen *tsPtr);
    void      initDevices(void);
    void      setCalibVars(const uint16_t minXts, const uint16_t minYts, 
                            const uint16_t maxXts, const uint16_t maxYts);

    void      setScreen(const uint8_t screen);
    uint8_t   getScreen(void);

    void      updateTouchStatus(void);
    bool      getCurrTouch(void);
    bool      getPrevTouch(void);
    uint16_t  getRawTouchX(void);
    uint16_t  getRawTouchY(void);
    uint16_t  getTouchX(void);
    uint16_t  getTouchY(void);

    uint16_t  getPrimaryColor(void);
    uint16_t  getDetailsColor(void);
    uint16_t  getAverageColor(void);  // Returns mean of primary and details

    bool      getKeyStrokePassthrough(void);
    void      enableKeyStrokePassthrough(void);
    void      disableKeyStrokePassthrough(void);

  private:
    XPT2046_Touchscreen *ts;
    File *file;
    File *root;
    uint16_t primaryColor = ILI9341_PURPLE; // UI background color
    uint16_t detailsColor = ILI9341_ORANGE; // Text, UI details color
    //uint16_t* bgImage; // background image / wallpaper

    uint8_t currentScreen = 0;

    // Calibration variables
    // for mapping touch-sensor coordinates to 
    // screen-space coordinates and vice/versa

    uint16_t minXTS, minYTS, maxXTS, maxYTS;

    // Whether or not keystrokes are passed to host machine(s)
    bool  keyPassthrough = true;

    // Used for handling touchscreen inputs
    bool      prevTouch = false;  // touch sensor status @end of previous frame
    bool      currTouch = false;  // touch sensor status @start of current frame
    uint16_t  TouchX    = 0;      // calibrated X-coordinate of touch input
    uint16_t  TouchY    = 0;      // calibrated Y-coordinate of touch input
};

#endif
