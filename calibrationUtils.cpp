#include "calibrationUtils.h"

// >:(
uint16_t mean(uint16_t* values, uint16_t numValues) {
  uint16_t sum = values[0];
  for (uint8_t i = 1; i < numValues; i++)
    sum += values[i];
  return sum/numValues;
}

void drawFullCross(
    const uint16_t x, 
    const uint16_t y, 
    const uint16_t color,
    ILI9341_t3n* tft
  ){
  tft->drawFastHLine(0, y, (uint16_t)DISP_WIDTH, color);
  tft->drawFastVLine(x, 0, (uint16_t)DISP_HEIGHT, color);

  return;
}

void drawCalibrate(ILI9341_t3n* tft, const uint8_t step, const uint16_t* calibPoints){
  tft->fillScreen(ILI9341_BLACK);
  tft->setTextColor(ILI9341_YELLOW);
  tft->setFont(Arial_12);
  tft->setCursor(4, 4);
  tft->print("Touch Screen Calibration");
  tft->setCursor(4, 100);
  tft->print("Step: ");
  tft->print(step+1);
  tft->print("/8");
  tft->setCursor(4, 200);
  tft->print("Please tap the cross\n as accurately as you can");
  
  drawFullCross(
        calibPoints[step*2+0], 
        calibPoints[step*2+1], 
        ILI9341_WHITE, 
        tft
        );
}

void doCalibrate(stateMachine* sm){

  // Track which calibration step we're on
  static uint8_t currStep = 0, prevStep = -1;

  // Points in screen-space to draw to the display
  const uint16_t calibPoints[16] = {
    DISP_WIDTH/2,     DISP_HEIGHT/2,    // Theta0
    DISP_WIDTH/4,     (3*DISP_HEIGHT)/4,// P
    DISP_WIDTH/2,     DISP_HEIGHT/2,    // Theta1
    (3*DISP_WIDTH)/4, (3*DISP_HEIGHT)/4,// Q
    DISP_WIDTH/2,     DISP_HEIGHT/2,    // Theta2
    (3*DISP_WIDTH)/4, DISP_HEIGHT/4,    // R
    DISP_WIDTH/2,     DISP_HEIGHT/2,    // Theta3
    DISP_WIDTH/4,     DISP_HEIGHT/4     // S
  };

  // Points in sensor-space corresponding to calibPoints
  static uint16_t measuredPoints[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  // Only update screen if we change step
  if (currStep != prevStep) {
    drawCalibrate(sm->tft, currStep, calibPoints);
    prevStep = currStep;
  }

  // Get, debounce touch input
  if (sm->getCurrTouch() != sm->getPrevTouch()) {
    // Perform action on touch release
    if (!sm->getCurrTouch()){
      const uint16_t tx = sm->getRawTouchX();
      const uint16_t ty = sm->getRawTouchY();
      Serial.println(tx);
      Serial.println(ty);
      measuredPoints[currStep*2+0] = tx;
      measuredPoints[currStep*2+1] = ty;
      //const uint32_t tmp = sm->getRawTouchXY();
      //Serial.println(tmp);
      //measuredPoints[currStep*2+0] = (uint16_t)(tmp >> 16);
      //measuredPoints[currStep*2+1] = (uint16_t)(tmp & 65536);
      //measuredPoints[currStep*2+0] = sm->getTouchX();
      //measuredPoints[currStep*2+1] = sm->getTouchY();
      currStep++;
    }
    delay(50);  //debounce
  }

  // Perform calibration calculations 
  if (currStep == 8) {
    uint16_t thetaMean[2];
    uint16_t thetaX[4];
    uint16_t thetaY[4];

    for (uint8_t i=0; i<4; i++){
      thetaX[i] = measuredPoints[i*4+0];
      thetaY[i] = measuredPoints[i*4+1];
    }

    thetaMean[0] = mean(thetaX, 4);
    thetaMean[1] = mean(thetaY, 4);

    uint16_t minXcalc[4]; // Ax
    uint16_t minYcalc[4]; // By
    uint16_t maxXcalc[4]; // Cx
    uint16_t maxYcalc[4]; // Dy

    uint8_t P=0,Q=1,R=2,S=3;

    // Use P to calculate params
    minXcalc[0] = thetaMean[0]-2*(thetaMean[0]-measuredPoints[P*4+2]);
    minYcalc[0] = thetaMean[1]+2*(measuredPoints[P*4+3]-thetaMean[1]);
    maxXcalc[0] = thetaMean[0]+2*(thetaMean[0]-measuredPoints[P*4+2]);
    maxYcalc[0] = thetaMean[1]-2*(measuredPoints[P*4+3]-thetaMean[1]);

    // Use Q to calculate params
    minXcalc[1] = thetaMean[0]-2*(measuredPoints[Q*4+2]-thetaMean[0]);
    minYcalc[1] = thetaMean[1]+2*(measuredPoints[Q*4+3]-thetaMean[1]);
    maxXcalc[1] = thetaMean[0]+2*(measuredPoints[Q*4+2]-thetaMean[0]);
    maxYcalc[1] = thetaMean[1]-2*(measuredPoints[Q*4+3]-thetaMean[1]);
  
    // Use R to calculate params
    minXcalc[2] = thetaMean[0]-2*(measuredPoints[R*4+2]-thetaMean[0]);
    minYcalc[2] = thetaMean[1]+2*(thetaMean[1]-measuredPoints[R*4+3]);
    maxXcalc[2] = thetaMean[0]+2*(measuredPoints[R*4+2]-thetaMean[0]);
    maxYcalc[2] = thetaMean[1]-2*(thetaMean[1]-measuredPoints[R*4+3]);
  
    // Use S to calculate params
    minXcalc[3] = thetaMean[0]-2*(thetaMean[0]-measuredPoints[S*4+2]);
    minYcalc[3] = thetaMean[1]+2*(thetaMean[1]-measuredPoints[S*4+3]);
    maxXcalc[3] = thetaMean[0]+2*(thetaMean[0]-measuredPoints[S*4+2]);
    maxYcalc[3] = thetaMean[1]-2*(thetaMean[1]-measuredPoints[S*4+3]);

    // Set calibration variables from averages
    sm->setCalibVars(
        mean(minXcalc, 4), 
        mean(minYcalc, 4), 
        mean(maxXcalc, 4), 
        mean(maxYcalc, 4)
        );
    //
    //sm->minXTS = mean(minXcalc, 4);
    //sm->minYTS = mean(minYcalc, 4);
    //sm->maxXTS = mean(maxXcalc, 4);
    //sm->maxYTS = mean(maxYcalc, 4);

    // Update screen, reset steps
    sm->setScreen(1);
    currStep = 0;
    prevStep = -1;

    sm->tft->fillScreen(ILI9341_PURPLE);
    sm->tft->setTextColor(ILI9341_YELLOW);
    sm->tft->setFont(Arial_18);
    sm->tft->setCursor(45, 120);
    sm->tft->print("Calibration Complete");
    delay(1000);
    sm->tft->fillScreen(ILI9341_BLACK);
  }
  delay(10);
}

