
#include <stdint.h>

#ifndef _ILI9341_t3NH_
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>
#endif

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include <TimeLib.h>  // Provide Real-time clock
#include "smStruct.h"

// Draws a button with a glyph symbol and text
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
      );

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
      );

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
      );

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
      );

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
      );
