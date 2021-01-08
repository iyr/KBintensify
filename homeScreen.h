#include <stdint.h>

#ifndef _ILI9341_t3NH_
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>
#endif

#ifndef _XPT2046_Touchscreen_h_
#include <XPT2046_Touchscreen.h>
#endif

#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

#include <TimeLib.h>  // Provide Real-time clock
#include "smStruct.h"
#include "buttons.h"

time_t getTeensy3Time(void);

void drawStatusBars(stateMachine* sm);

void doHomeScreen(stateMachine* sm);

/*
 * Builds a neatly formatted date c-string
 * Returns length of formatted string
 */
uint8_t buildDateStr(
      char* output,
      int   weekday,
      int   month,
      int   day
      );

/*
 * Builds a neatly formatted time c-string, without seconds
 * Returns length of formatted string
 */
uint8_t buildTimeStr(
      char* output, 
      const bool useMeridiem, 
      uint8_t t_hour,
      const uint8_t t_minute
      );

/*
 * Builds a neatly formatted time c-string, including seconds
 */
uint8_t buildTimeStr(
      char* output, 
      const bool useMeridiem, 
      uint8_t t_hour,
      const uint8_t t_minute,
      const uint8_t t_second
      );
