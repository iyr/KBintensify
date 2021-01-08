#include "buttons.h"

// Draws a button with a glyph symbol and text
// Constant Size, not scalable
// Returns True if the button has been pressed
bool doIconTextButton(
      uint16_t          posX,       // X-Position to draw button
      uint16_t          posY,       // Y- Position to draw button
      const uint8_t     radX,       // X-radius of the button
      const uint8_t     radY,       // Y-radius of the button
      const int8_t      Yadjust,    // used for fine-tuning troublesome glyphs
      char*             iconTitle,  // null-term'd Title text of shortcut
      ILI9341_t3_font_t symbolFont, // Font for shortcut symbol glyph
      char              symbol,     // shortcut symbol glyph
      const uint16_t    bgColor,    // Button background color
      const uint16_t    dtColor,    // Button details/accents color
      stateMachine*     sm          // Statemachine
      ){

  // Convenience placeholders
  const uint16_t TouchX = sm->getTouchX();
  const uint16_t TouchY = sm->getTouchY();

  // Draw Button background
  sm->tft->fillRoundRect(
      posX-radX, posY-radY, 
      radX*2, radY*2, 
      min(radX, radY)/3, 
      bgColor
      );

  // Draw Button Text, replace spaces with line-breaks
  sm->tft->setTextColor(dtColor);
  sm->tft->setFont(Arial_8);
  char  subBuff[32] = {'\0'};
  const uint8_t upperLim = (uint8_t)max(strlen(iconTitle), 32);
  uint8_t lowerLim = 0;
  uint8_t yShift = 4;
  sm->tft->setTextDatum(TC_DATUM);
  for (uint8_t i=lowerLim; i<upperLim; i++){
    if ( iconTitle[i] == ' ' ||
         i == upperLim-1         ){
      for (uint8_t j = lowerLim; j < i+(i==upperLim-1?1:0); j++){
        subBuff[j-lowerLim] = iconTitle[j];
      }
      sm->tft->drawString1(
            subBuff,
            upperLim+2,
            posX, 
            posY+yShift
            );
      lowerLim = i+1;
      yShift += 9;
    }
  }

  // Draw Button Icon
  sm->tft->setFont(symbolFont);
  sm->tft->setTextDatum(BC_DATUM);
  char tmp[3] = {'\0'};
  tmp[0] = symbol;
  sm->tft->drawString1(tmp, 3, posX, Yadjust+(posY-radY/2));

  // Highlight Icon if cursor over, but touch not released
  if (sm->getCurrTouch()){
    if (  TouchX <= posX+radX &&
          TouchX >= posX-radX &&
          TouchY <= posY+radY &&
          TouchY >= posY-radY ){
      //sm->tft->drawRoundRect(posX-28, posY-24, 28*2, 24*2, 8, dtColor);
      sm->tft->drawRoundRect(
          posX-radX, posY-radY, 
          radX*2, radY*2, 
          min(radX, radY)/3, 
          dtColor
          );
    }
  }

  // Touch released
  if (  sm->getPrevTouch() != sm->getCurrTouch()  &&
        sm->getCurrTouch() == false          ){
    if (  TouchX <= posX+radX &&
          TouchX >= posX-radX &&
          TouchY <= posY+radY &&
          TouchY >= posY-radY ){
      // Draw Button background
      sm->tft->fillRoundRect(
          posX-radX, posY-radY, 
          radX*2, radY*2, 
          min(radX, radY)/3, 
          dtColor
          );
      //sm->tft->fillRoundRect(posX-28, posY-24, 28*2, 24*2, 8, dtColor);
      return true;
    }
  }

  return false;
}

// Draws a button with just a glyph symbol
// Variable size
// Returns True if the button has been pressed
bool doIconButton(
      const uint16_t    posX,       // X-Position to draw button
      const uint16_t    posY,       // Y- Position to draw button
      const uint8_t     radX,       // X-radius of the button
      const uint8_t     radY,       // Y-radius of the button
      const int8_t      Yadjust,    // used for fine-tuning troublesome glyphs
      ILI9341_t3_font_t symbolFont, // Font for shortcut symbol glyph
      const char        symbol,     // shortcut symbol glyph
      const uint16_t    bgColor,    // Button background color
      const uint16_t    dtColor,    // Button details/accents color
      stateMachine*     sm          // Statemachine
      ){

  // Convenience placeholders
  const uint16_t TouchX = sm->getTouchX();
  const uint16_t TouchY = sm->getTouchY();

  // Draw Button background
  sm->tft->fillRoundRect(
      posX-radX, posY-radY, 
      radX*2, radY*2, 
      min(radX, radY)/3, 
      bgColor
      );

  // Draw Button Icon
  sm->tft->setFont(symbolFont);
  sm->tft->setTextColor(dtColor);
  sm->tft->setTextDatum(BC_DATUM);
  char tmp[3] = {'\0'};
  tmp[0] = symbol;
  sm->tft->drawString1(tmp, 3, posX, Yadjust+(posY-radY/2));

  // Highlight Icon if cursor over, but touch not released
  //if (sm->getCurrTouch()){
    //if (  TouchX <= posX+radX &&
          //TouchX >= posX-radX &&
          //TouchY <= posY+radY &&
          //TouchY >= posY-radY ){
      sm->tft->drawRoundRect(
          posX-radX, posY-radY, 
          radX*2, radY*2, 
          min(radX, radY)/3, 
          dtColor
          );
    //}
  //}

  // Touch released
  if (  sm->getPrevTouch() != sm->getCurrTouch()  &&
        sm->getCurrTouch() == false          ){
    if (  TouchX <= posX+radX &&
          TouchX >= posX-radX &&
          TouchY <= posY+radY &&
          TouchY >= posY-radY ){
      // Draw Button background
      sm->tft->fillRoundRect(
          posX-radX, posY-radY, 
          radX*2, radY*2, 
          min(radX, radY)/3, 
          dtColor
          );
      return true;
    }
  }

  return false;
}
