
#include <stdint.h>

//#ifndef _ILI9341_t3NH_
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>
//#endif

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

#include "smStruct.h"
#include "buttons.h"

#define MAX_FILEPATH_LENGTH 256
#define MAX_FILELIST_LENGTH 256
#define MAX_FILENAME_LENGTH 32

FLASHMEM uint32_t getDirectoryContents(File dir, char*** filelist);

// Returns true when (validated) selection has been made
// updates filePath String pointer
FLASHMEM bool doFileOpen(
      const char*   Title,
      uint8_t       TitleLen,
      char*         filePath,
      bool (*valid)(char *, stateMachine *),
      stateMachine* sm
      );
