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
#include "dialogue.h"
#include "textInputFieldClass.h"

FLASHMEM void doPassMan(stateMachine* sm);
