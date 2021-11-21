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
			const uint8_t			titleLen,		// length, in chars, of title text
      ILI9341_t3_font_t symbolFont, // Font for shortcut symbol glyph
      char              symbol,     // shortcut symbol glyph
      const uint16_t    bgColor,    // Button background color
      const uint16_t    dtColor,    // Button details/accents color
      stateMachine*     sm          // Statemachine
      ){

  // Convenience placeholders
  const uint16_t 	TouchX = sm->getTouchX();
  const uint16_t 	TouchY = sm->getTouchY();

  // Draw Button background
  sm->tft->fillRoundRect(
      posX-radX, posY-radY, 
      radX*2, radY*2, 
      min(radX, radY)/3, 
      bgColor
      );

  // Draw Button Text
  sm->tft->setTextColor(dtColor);
  sm->tft->setFont(Arial_8);
  char  subBuff[32] = {'\0'};
  //const uint8_t upperLim = (uint8_t)min(strlen(iconTitle), 32);
  const uint8_t upperLim = titleLen;
  uint8_t lowerLim = 0;
  uint8_t yShift = 4;
  sm->tft->setTextDatum(TC_DATUM);
  for (uint8_t i=lowerLim; i<upperLim; i++){
    if ( iconTitle[i] == '\n' || i == upperLim-1 ){
      for (uint8_t j = lowerLim; j < i+1; j++){
        subBuff[j-lowerLim] = iconTitle[j];
      }
      sm->tft->drawString(
            subBuff,
            (i+1)-lowerLim,
            posX, 
            posY+yShift+Yadjust+5
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
  sm->tft->drawString(tmp, 1, posX, Yadjust+(posY-radY/2));

  // Highlight Icon if cursor over, but touch not released
  if (  //sm->touchEnabled()  			||
        sm->getCurrTouch()  			){
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

	// Check if cursor is inside button area
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

  return false;
}

// Draws a button with two glyph symbols side-by-side
// Variable size
// Returns True if the button has been pressed
bool doDoubleIconButton(
      const uint16_t    posX,       // X-Position to draw button
      const uint16_t    posY,       // Y- Position to draw button
      const uint8_t     radX,       // X-radius of the button
      const uint8_t     radY,       // Y-radius of the button
      const int8_t      Yadjust,    // used for fine-tuning troublesome glyphs
      ILI9341_t3_font_t symbolFont, // Font for shortcut symbol glyph
      const char        symbolA,     // shortcut symbol glyph
      const char        symbolB,     // shortcut symbol glyph
      const uint16_t    bgColor,    // Button background color
      const uint16_t    dtColor,    // Button details/accents color
      const bool        drawBorder, // Draw button border
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
  char tmp[4] = {'\0'};
  tmp[0] = symbolA;
  tmp[1] = (char)127;
  tmp[2] = symbolB;
  sm->tft->drawString(tmp, 3, posX, Yadjust+(posY-radY/2));

  // Draw Button border
  if (drawBorder)
    sm->tft->drawRoundRect(
        posX-radX, posY-radY, 
        radX*2, radY*2, 
        min(radX, radY)/3, 
        dtColor
        );

	// Check if cursor is inside button area
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
      const bool        drawBorder, // Draw button border
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
  sm->tft->drawString(tmp, 1, posX, Yadjust+(posY-radY/2));

  // Draw Button border
  if (drawBorder)
    sm->tft->drawRoundRect(
        posX-radX, posY-radY, 
        radX*2, radY*2, 
        min(radX, radY)/3, 
        dtColor
        );

	// Check if cursor is inside button area
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

  return false;
}

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
  return doIconButton(
      posX,       // X-Position to draw button
      posY,       // Y- Position to draw button
      radX,       // X-radius of the button
      radY,       // Y-radius of the button
      Yadjust,    // used for fine-tuning troublesome glyphs
      symbolFont, // Font for shortcut symbol glyph
      symbol,     // shortcut symbol glyph
      bgColor,    // Button background color
      dtColor,    // Button details/accents color
      true,
      sm          // Statemachine
      );
}

// Draws a button with just text
// Returns True if the button has been pressed
bool doTextButton(
      uint16_t          posX,     // X-Position to draw button
      uint16_t          posY,     // Y- Position to draw button
      const uint8_t     radX,     // X-radius of the button
      const uint8_t     radY,     // Y-radius of the button
      char*             text,			// null-term'd text of button 
      ILI9341_t3_font_t textFont,	// Font for button text
      const uint16_t    bgColor,  // Button background color
      const uint16_t    dtColor,  // Button details/accents color
      stateMachine*     sm        // Statemachine
      ){

  // Convenience placeholders
  const uint16_t 	TouchX = sm->getTouchX();
  const uint16_t 	TouchY = sm->getTouchY();

	// Text Boundary place holders
	int16_t				x1, y1;
	uint16_t			w, h;

  // Draw Button background
  sm->tft->fillRoundRect(
      posX-radX, posY-radY, 
      radX*2, radY*2, 
      min(radX, radY)/3, 
      bgColor
      );

  // Draw Button border
  sm->tft->drawRoundRect(
      posX-radX, posY-radY, 
      radX*2, radY*2, 
      min(radX, radY)/3, 
      dtColor
      );

  // Draw Button Text
  sm->tft->setTextColor(dtColor);
  sm->tft->setFont(textFont);
  sm->tft->setTextDatum(CC_DATUM);
	const char heightTest[] = {"jgypq,QW@#|({[`;'_"};
	sm->tft->getTextBounds(
			//text,
			heightTest,
			160, 120,
			&x1, 	&y1,
			&w, 	&h);
  char  				subBuff[32] 	= {'\0'};
  const uint8_t	upperLim 			= (uint8_t)max(strlen(text), 32);
	uint8_t				numLineBreaks	= 0;
  for (uint8_t i=0; i<upperLim; i++) if ( text[i] == '\n' ) numLineBreaks++;
  uint8_t 			lowerLim 			= 0;
  int8_t				yShift 				= -(3+(h>>4)) - ((h>>1)-1)*(numLineBreaks);

#ifdef DEBUG
	Serial.print("Text Line height: ");
	Serial.print(h);
	Serial.print(", numLineBreaks: ");
	Serial.print(numLineBreaks);
	Serial.print(", yShift: ");
	Serial.print(yShift);
	Serial.println();
#endif

	// Corrects Line spacing based on font size
	int8_t 	tmh =		(((h>>3)-1) | 1);
					tmh *= 	tmh;

  for (uint8_t i=lowerLim; i<upperLim; i++){
    if ( text[i] == '\n' ||
         i == upperLim-1         ){
      for (uint8_t j = lowerLim; j < i+(i==upperLim-1?1:0); j++){
        subBuff[j-lowerLim] = text[j];
      }
      sm->tft->drawString(
            subBuff,
            upperLim,
            posX, 
            (posY+yShift)
            );
      lowerLim = i+1;
			yShift += h-tmh;
			memset(subBuff, '\0', 32);
    }
  }

  if (sm->getCurrTouch()  			){
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

	// Check if cursor is inside button area
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

  return false;
}
