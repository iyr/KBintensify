#include "passwordManager.h"
#include "textInputField.h"
#include <t3nfonts.h>

#define PM_STATE_OPENDB 0
#define PM_STATE_EDITDB 1
#define PM_STATE_DISPDB 2
#define PM_STATE_EDITLT 3

#define FIELD_LENGTH 256

char passwordField[FIELD_LENGTH] = {"The Quick Brown Fox Jumps Over The Lazy Dog."};
char databaseField[FIELD_LENGTH] = {"some filepath idk"};
char key_fileField[FIELD_LENGTH] = {"/path/to/file.key"};
//     lol

// Determines whether the filepath leads to a proper database file
bool databaseValid(const char* filePath, stateMachine* sm){
  return true;
};

// Any key file is valid
bool key_fileValid(const char* filePath, stateMachine* sm){
  return true;
};

FLASHMEM void doPassMan(stateMachine* sm){

  static uint8_t  currentSubScreen = 0;
  char            loremIpsum[FIELD_LENGTH] = {'\0'};

  uint16_t        offsetX = 2,
                  offsetY = 20;

  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();
  const   uint16_t  averageColor  = sm->getAverageColor();
  const   uint16_t  primaryColorI = sm->getPrimaryColorInverted();

  const   uint16_t  pressedKey    = sm->getPressedKey();

  // Size of UI buttons
  const   uint8_t   buttonRadX    = 27;
  const   uint8_t   buttonRadY    = 21;
  
  //sm->tft->fillScreen(ILI9341_BLACK);
  //sm->tft->fillScreenHGradient(averageColor, primaryColor);
  sm->tft->fillScreenHGradient(primaryColor-1, averageColor);
  switch(currentSubScreen) {

    case PM_STATE_OPENDB:
      static uint16_t passCursor  = 0;
      static uint16_t dbfpCursor  = 0;
      static uint16_t kyfpCursor  = 0;
      static bool     passActive  = true;
      static bool     dbfpActive  = false;
      static bool     kyfpActive  = false;
      static bool     hidePass    = true;

      sm->disableKeyStrokePassthrough();

      //
      //  Draw Password input field with visibility toggle button
      //
      sm->tft->setFont(Arial_12);
      sm->tft->setTextDatum(TL_DATUM);
      sm->tft->setTextColor(accentsColor);
      strcpy(loremIpsum, "Database Password:");
      sm->tft->drawString1(loremIpsum, strlen(loremIpsum)+2, offsetX, offsetY);
      offsetY += 22;
      // Parse user input into text field
      doTextInputField(
          passwordField,
          FIELD_LENGTH,           // buffer size, not strlen
          &passCursor,
          sm->getPressedKeys(),
          sm->getModifiers(),
          passActive,
          sm->getKeyPressHeld(),
          sm->getNumKeysPressed(),
          sm->getReleasedKey()
          );
      // Draw text field
      const uint8_t obscureBuffSize = 32;
      char obscureBuff[obscureBuffSize] = {'\0'};
      for (uint8_t i = 0; i < min(obscureBuffSize, strlen(passwordField)); i++) 
        obscureBuff[i] = '*';
      if (drawTextInputField(
          offsetX, offsetY,
          DISP_WIDTH-64, 15,
          LiberationMono_11,
          TL_DATUM,
          hidePass?&obscureBuff:passwordField,
          hidePass?min(obscureBuffSize, strlen(passwordField)+1):FIELD_LENGTH,
          &passCursor,
          passActive,
          sm
          )){
        passActive = true;
        dbfpActive = false;
        kyfpActive = false;
      }
      // Draw Show/Hide password toggle
      if (doIconButton(
            DISP_WIDTH-buttonRadX-1, offsetY,
            buttonRadX, buttonRadY, 0,
            AwesomeF000_20,
            (char)112,  // Crossed eye; obscure visibility
            primaryColor,
            hidePass?accentsColor:primaryColorI,
            hidePass,
            sm)){
        hidePass = !hidePass;
      }

      //
      //  Draw database filepath field with file open button
      //
      offsetY += 32;
      sm->tft->setFont(Arial_12);
      sm->tft->setTextDatum(TL_DATUM);
      sm->tft->setTextColor(accentsColor);
      memset(loremIpsum, '\0', FIELD_LENGTH);
      strcpy(loremIpsum, "Database Filepath:");
      sm->tft->drawString1(loremIpsum, strlen(loremIpsum)+2, offsetX, offsetY);
      offsetY += 22;
      // Parse user input into text field
      doTextInputField(
          databaseField,
          FIELD_LENGTH,           // buffer size, not strlen
          &dbfpCursor,
          sm->getPressedKeys(),
          sm->getModifiers(),
          dbfpActive,
          sm->getKeyPressHeld(),
          sm->getNumKeysPressed(),
          sm->getReleasedKey()
          );
      // Draw text field
      if (drawTextInputField(
          offsetX, offsetY,
          DISP_WIDTH-64, 15,
          LiberationMono_11,
          TL_DATUM,
          databaseField,
          FIELD_LENGTH,
          &dbfpCursor,
          dbfpActive,
          sm
          )){
        passActive = false;
        dbfpActive = true;
        kyfpActive = false;
      }

      char        Title[20]   = "Select Database";
      uint8_t     TitleLen    = 20;
      static bool selectingDB = false;
      static bool DBselected  = false;
      // Draw file open button
      if (doIconButton(
            DISP_WIDTH-buttonRadX-1, offsetY,
            buttonRadX, buttonRadY, 0,
            AwesomeF000_20,
            (char)124,  // Crossed eye; obscure visibility
            primaryColor,
            accentsColor,
            sm)){
        selectingDB = true;
        passActive  = false;
        dbfpActive  = true;
        kyfpActive  = false;
      }
      if (selectingDB)
        DBselected = doFileOpen(
          (char*)Title, 
          TitleLen, 
          databaseField,
          databaseValid,
          sm
          );
      if (DBselected){
        selectingDB = false;
        DBselected  = false;
      }

      //
      //  Draw keyfile filepath field with file open button
      //
      offsetY += 32;
      sm->tft->setFont(Arial_12);
      sm->tft->setTextDatum(TL_DATUM);
      sm->tft->setTextColor(accentsColor);
      memset(loremIpsum, '\0', FIELD_LENGTH);
      strcpy(loremIpsum, "Key Filepath:");
      sm->tft->drawString1(loremIpsum, strlen(loremIpsum)+2, offsetX, offsetY);
      offsetY += 22;

      // Parse user input into text field
      doTextInputField(
          key_fileField,
          FIELD_LENGTH,           // buffer size, not strlen
          &kyfpCursor,
          sm->getPressedKeys(),
          sm->getModifiers(),
          kyfpActive,
          sm->getKeyPressHeld(),
          sm->getNumKeysPressed(),
          sm->getReleasedKey()
          );
      // Draw text field
      if (drawTextInputField(
          offsetX, offsetY,
          DISP_WIDTH-64, 15,
          LiberationMono_11,
          TL_DATUM,
          key_fileField,
          FIELD_LENGTH,
          &kyfpCursor,
          kyfpActive,
          sm
          )){
        passActive = false;
        dbfpActive = false;
        kyfpActive = true;
      }

      static bool selectingKF = false;
      static bool KFselected  = false;
      // Draw file open button
      if (doIconButton(
            DISP_WIDTH-buttonRadX-1, offsetY,
            buttonRadX, buttonRadY, 0,
            AwesomeF000_20,
            (char)124,  // Crossed eye; obscure visibility
            primaryColor,
            accentsColor,
            sm)){
        selectingKF = true;
        passActive  = false;
        dbfpActive  = true;
        kyfpActive  = false;
      }
      if (selectingKF)
        KFselected = doFileOpen(
          (char*)Title, 
          TitleLen, 
          key_fileField,
          key_fileValid,
          sm
          );
      if (KFselected){
        selectingKF = false;
        KFselected  = false;
      }
      break;

    case PM_STATE_EDITDB:

      break;

    case PM_STATE_DISPDB:

      break;

    case PM_STATE_EDITLT:

      break;

  }
  //sm->tft->setTextDatum(TC_DATUM);
  //sm->tft->setFont(AwesomeF080_96);
  char tmp[4] = {'\0'};
  tmp[0] = (char)4;
  tmp[1] = (char)28;
  //sm->tft->drawString1(tmp, 4, DISP_WIDTH/2, DISP_HEIGHT/5);

  return;
};

