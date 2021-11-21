#include <stdint.h>

#ifndef _ILI9341_t3NH_
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>
#endif

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include "smStruct.h"
#include "extraSymbols.h"
#include "dialogue.h"
#include "buttons.h"
#include <t3nfonts.h>

void doOutputMultiplexor(stateMachine* sm);
const short rollover(short input, const short lowerLimit, const short upperLimit);
