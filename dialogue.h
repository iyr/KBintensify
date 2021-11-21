
#include <stdint.h>

//#ifndef _ILI9341_t3NH_
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>
//#endif

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include <SD.h>

#include "smStruct.h"
#include "buttons.h"

#define MAX_FILEPATH_LENGTH 256
#define MAX_FILELIST_LENGTH 256
#define MAX_FILENAME_LENGTH 32

// Returns true when (validated) selection has been made
// updates filePath String pointer
FLASHMEM bool doFileOpen(
      const char*   Title,
      uint8_t       TitleLen,
      char*         filePath,
      bool (*valid)(char *, stateMachine *),
      stateMachine* sm
      );

FLASHMEM uint32_t getDirectoryContents(FsFile dir, char*** filelist);

FLASHMEM uint8_t doMultiOptMessage(
      const char          Symbol,
      ILI9341_t3_font_t   SymbolFont,
      const char*   			Title,
      const uint8_t 			TitleLen,
      			char*   			BodyText,
      const uint16_t			BodyLen,
      const uint8_t 			numButtons,
      const char**  			buttonTexts,
      const uint8_t 			buttonTextLens,
      const uint16_t*     buttonColors,
      ILI9341_t3_font_t*	buttonFonts,
      stateMachine* 			sm
    );


const uint8_t drawTextField(
          uint16_t    posX, 
          uint16_t    posY,
    const uint16_t    fieldWidth,
    const uint16_t    fieldHeight,
          char*       text,
    const uint16_t    textLen,
    ILI9341_t3_font_t textFont,
    const uint16_t    currIndex,
    stateMachine*     sm);

// Finds the index of next byte matching char,
// returns input index if no match is found
const uint16_t findNext(
    const char*     text,
    const uint16_t  textLen,
    const uint16_t  currIndex,
    const char      searchTarget
    );

// Finds the index of prev byte matching char,
// returns input index if no match is found
const uint16_t findPrev(
    const char*     text,
    const uint16_t  textLen,
    const uint16_t  currIndex,
    const char      searchTarget
    );
