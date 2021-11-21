#include <stdint.h>
#include <t3nfonts.h>

#include "dialogue.h"
#include "alphabetize.h"
#include "pgmspace.h"

extern char   fileNameBuff[];
extern char** fileListBuff;
static bool   dialogueEntered = false;

//EXTMEM char fileListBuff[MAX_FILELIST_LENGTH][MAX_FILENAME_LENGTH];
//EXTMEM char fileNameBuff[MAX_FILEPATH_LENGTH];

// Returns true when (validated) selection has been made
// updates filePath c-str pointer
FLASHMEM bool doFileOpen(
      const char*   Title,
      uint8_t       TitleLen,
      char*         filePath,
      bool (*valid)(char *, stateMachine *),
      stateMachine* sm
      ){

  //if (!dialogueEntered) sm->captureFrameBufferToBackBuffer();
  sm->tft->writeRect(0, 0, DISP_WIDTH, DISP_HEIGHT, sm->getBackBuffer());
  dialogueEntered = true;
  sm->enableTouchInput();
  sm->disableDrawing();

  // Used for drawing individual glyphs
  char iconGlyph[3] = {'\0'};

  // Used for file/directory navigation
  static  uint16_t  selectedFile  = 0;
  static  int16_t   numFiles      = -1;

  // UI element position place-holders
  uint16_t          offsetX       = 24,
                    offsetY       = 24;

  // Used for drawing a finite range of listings
  const   uint8_t   numListings   = 5;
  static  uint16_t  lowerLim      = 0;
  static  uint16_t  upperLim      = numListings;

  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();
  const   uint16_t  averageColor  = sm->getAverageColor();
  const   uint16_t  TouchX        = sm->getTouchX();
  const   uint16_t  TouchY        = sm->getTouchY();

  // Size of UI buttons
  const   uint8_t   buttonRadX    = 27;
  const   uint8_t   buttonRadY    = 19;

  const   bool      screenWasPressed      = sm->screenWasPressed();
  const   bool      screenWasLongPressed  = sm->screenWasLongPressed();
  const   bool      screenWasReleased     = sm->screenWasReleased();
  const   bool      actionRepeat          = sm->getRepeatAction();

  // Return value
  bool              selectionMade = false;

  // Draw Backrop w/border
  sm->tft->fillRect(
      offsetX,
      offsetY,
      DISP_WIDTH-offsetX*2,
      DISP_HEIGHT-offsetY*3,
      primaryColor
      );
  offsetX += 3;
  offsetY += 3;
  sm->tft->drawRoundRect(
      offsetX,
      offsetY,
      DISP_WIDTH-offsetX*2,
      DISP_HEIGHT-offsetY*3+3,
      4,
      accentsColor
      );

  // Draw Dividing lines
  sm->tft->drawLine(
      offsetX, 
      offsetY+3*offsetY/4, 
      DISP_WIDTH-offsetX-1, 
      offsetY+3*offsetY/4,  
      accentsColor
      );
  sm->tft->drawLine(
      offsetX, 
      DISP_HEIGHT-11*offsetY/4+3, 
      DISP_WIDTH-offsetX-1, 
      DISP_HEIGHT-11*offsetY/4+3,  
      accentsColor
      );

  offsetX += 3;
  offsetY += 3;

  // Draw open folder icon
  sm->tft->setTextColor(accentsColor);
  sm->tft->setTextDatum(TL_DATUM);

  sm->tft->setFont(AwesomeF000_12);
  iconGlyph[0] = (char)124; // Open Folder glyph
  sm->tft->drawString(iconGlyph, 1, offsetX, offsetY);

  sm->tft->setFont(AwesomeF080_12);
  iconGlyph[0] = (char)90;  // Right-pointing arrow glyph
  sm->tft->drawString(iconGlyph, 1, offsetX+18, offsetY);

  // Draw File dialogue text
  sm->tft->setFont(Arial_12);
  sm->tft->drawString(Title, (uint16_t)TitleLen, offsetX+26, offsetY);

  if (numFiles < 0) { //  avoid unnecessary recomputation
    // Set directory to open
    memset(fileNameBuff, '\0', MAX_FILEPATH_LENGTH);
    strcpy(fileNameBuff, "/");
    FsFile tmpFile = sm->sd->open((const char *)fileNameBuff);
    numFiles = getDirectoryContents(tmpFile, &fileListBuff);
    alphabetize(fileListBuff, numFiles);
    upperLim = min(min(MAX_FILELIST_LENGTH, numFiles), numListings);
    tmpFile.close();
  }

  // Draw selected file index / number of files
  char num2chars[33] = {'\0'};  // Used to convert number to c-string
  char printBuff[72] = {'\0'};  // Buffer containing '(index/numfiles)'
  // ^^^^ these buffers could be smaller in practice, but
  // are large-ish to accommodate potential bignums in 32-bit ints for safety
  sm->tft->setTextDatum(TR_DATUM);
  printBuff[0] = '(';                       // Start with left brace
  itoa( selectedFile+(int)(numFiles>0), 
        num2chars, 
        10);                                // get selection index to c-str num
  uint8_t i = 1;                            // convenience variable
  for (i = 1; i < strlen(num2chars)+1; i++) // copy index c-str num to printBuff
    printBuff[i] = num2chars[i-1];
  memset(num2chars, '\0', 33);              // reset c-str num buffer
  printBuff[i] = ' '; i++;                  // add space; improves legibility
  printBuff[i] = '/'; i++;                  // visual delineator
  itoa(numFiles, num2chars, 10);            // get number of files to c-str num
  const uint8_t ti = i;                     // convenience variable
  for (i = 0; i < strlen(num2chars)+1; i++) // copy numfiles to printBuff
    printBuff[ti+i] = num2chars[i];
  printBuff[ti+i-1] = ')';                    // Append right brace
  sm->tft->setTextDatum(TR_DATUM);
  sm->tft->drawString(printBuff, ti+i+1, DISP_WIDTH-offsetX, offsetY);

  // Setup vars for printing directory entries
  offsetX = 32;
  offsetY += 4;
  sm->tft->setTextDatum(TL_DATUM);
  sm->tft->setCursor(80, offsetY);
  sm->tft->println();

  // Draw Directory entries w/ symbols
  for (uint32_t i = lowerLim; i < upperLim; i++) {
    offsetY += 18;
    // Use Glyph to indicate folder vs file
    if (fileListBuff[i][strlen(fileListBuff[i])-1] == '/') {
      iconGlyph[0] = (char)123; // Folder Icon
      sm->tft->setFont(AwesomeF000_12);
    } else {
      iconGlyph[0] = (char)118; // File Icon
      sm->tft->setFont(AwesomeF080_12);
    }
    // Indicate end of list
    if (i >= numFiles) {
      iconGlyph[0] = (char)94;  // Up arrow
      sm->tft->setFont(AwesomeF080_12);
    }

    // Highlight current selection
    if (i == selectedFile) {
      sm->tft->setTextColor(primaryColor);
      sm->tft->fillRect(offsetX-2, offsetY-3, 214, 18, accentsColor);
    } else {
      sm->tft->setTextColor(accentsColor);
    }

    // Draw entry
    sm->tft->drawString(iconGlyph, 1, offsetX, offsetY-2);
    sm->tft->setFont(LiberationMono_11);
    if (i < numFiles)
      sm->tft->drawString(
            fileListBuff[i], 
            min(23, strlen(fileListBuff[i])),
            offsetX+18, 
            offsetY
            );
  }

  offsetX=24;
  offsetY=24;
  offsetX += 4;
  offsetY += 2;

  // Draw Mask to partially obscure text/iconography
  sm->tft->fillRect(
      offsetX,
      DISP_HEIGHT-(11*offsetY/4)+1, 
      DISP_WIDTH-offsetX*2-4,
      12,
      primaryColor
      );

  // Draw current directory path
  sm->tft->setTextDatum(TL_DATUM);
  sm->tft->setTextColor(accentsColor);

  sm->tft->setFont(AwesomeF100_8);
  iconGlyph[0] = (char)20; // Folder Icon Glyph
  sm->tft->drawString(
      iconGlyph, 
      1,
      offsetX+4, 
      DISP_HEIGHT-11*offsetY/4+6
      );

  sm->tft->setFont(LiberationMono_8);
  sm->tft->drawString(
      fileNameBuff,
      (uint16_t)strlen(fileNameBuff),
      offsetX+16, 
      DISP_HEIGHT-11*offsetY/4+8
      );

  // Reset positional vars for drawing buttons
  offsetX = 272;
  offsetY = 69;
  const char lastChar = fileListBuff[selectedFile][strlen(fileListBuff[selectedFile])-1];

  // Scroll Up (Decrement file selection index)
  if (doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY,  0,
        AwesomeF000_20, 
        (char)119,  // Up arrow
        primaryColor,
        (numFiles>1)?accentsColor:averageColor,
        sm)             &&
      (screenWasPressed || actionRepeat)  ){
    if (screenWasPressed) {
      if (selectedFile <= 0)  // wrap selection to end if at top
        selectedFile = numFiles-1;
      else 
        selectedFile -= 1;
    } else if (actionRepeat) {
      if (selectedFile <= 0)  // Stop auto-decrement at end of list
        selectedFile = 0;
      else 
        selectedFile -= 1;
    }

    selectedFile *= (int)(numFiles > 0);

    // Recalculate range of entries to draw, if necessary
    if (selectedFile < lowerLim) lowerLim = selectedFile;
    if (selectedFile > upperLim-2) lowerLim = selectedFile-(min(numListings-2, numFiles-1));
    upperLim = lowerLim+min(numListings, numFiles);
  }

  offsetY += (buttonRadY+1)*2;
  // Scroll Down (Increment file selection index)
  if (doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY,  0,
        AwesomeF000_20, 
        (char)120,  // Down arrow
        primaryColor,
        (numFiles>1)?accentsColor:averageColor,
        sm)             &&
      (screenWasPressed || actionRepeat)  ){

    if (screenWasPressed) {
      if (selectedFile+1 >= numFiles) // wrap selection to top if at bottom
        selectedFile = 0;
      else 
        selectedFile += 1;
    } else if (actionRepeat) {
      if (selectedFile+1 >= numFiles) // Stop auto-increment at end of list
        selectedFile = numFiles-1;
      else 
        selectedFile += 1;
    }

    selectedFile *= (int)(numFiles > 0);

    // Recalculate range of entries to draw, if necessary
    if (selectedFile < lowerLim) lowerLim = selectedFile;
    if (selectedFile > upperLim-2) lowerLim = selectedFile-(min(numListings-2, numFiles-1));
    upperLim = lowerLim+min(numListings, numFiles);
  }

  offsetY += (buttonRadY+1)*2;
  // Confirm (validated) selection
  char confBuff[MAX_FILEPATH_LENGTH] = {'\0'};
  memcpy(confBuff, fileNameBuff, MAX_FILEPATH_LENGTH);
  strncat(confBuff, fileListBuff[selectedFile], MAX_FILEPATH_LENGTH);
  const bool fileIsValid = valid(confBuff, sm);
  if (doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY, -2,
        AwesomeF000_24, 
        (char)88,
        primaryColor,
        (fileIsValid)?accentsColor:averageColor,
        sm)             &&
      screenWasReleased &&
      fileIsValid       ){
    strncat(fileNameBuff, fileListBuff[selectedFile], MAX_FILEPATH_LENGTH);
    memcpy(filePath, fileNameBuff, MAX_FILEPATH_LENGTH);

    // Shave off top file from path
    uint8_t curDirInd = 0,
            prevFNlen = strlen(fileNameBuff);
    for (uint8_t i = 1; i < prevFNlen; i++)
      if (fileNameBuff[i] == '/' && i != prevFNlen-1 ) curDirInd = i;
    for (uint8_t i = curDirInd+1; i < prevFNlen; i++)
      fileNameBuff[i] = '\0';
    selectionMade = true;
  }

  offsetX -= (buttonRadX+1)*2;
  // Enter directory
  if (numFiles > 0              &&
      doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY,  0,
        AwesomeF100_20, 
        (char)21,
        primaryColor,
        (lastChar == '/')?accentsColor:averageColor,
        sm)             &&
      screenWasPressed  &&
      !selectionMade    ){

    // Enter (Sub)Folder, rebuild File List
    if (lastChar == '/') {
      strncat(fileNameBuff, fileListBuff[selectedFile], MAX_FILEPATH_LENGTH);
      FsFile tmpFile = sm->sd->open((const char *)fileNameBuff);
      numFiles = getDirectoryContents(tmpFile, &fileListBuff);
      alphabetize(fileListBuff, numFiles);
      lowerLim = 0;
      upperLim = min(min(MAX_FILELIST_LENGTH, numFiles), numListings);
      tmpFile.close();
      selectedFile = 0;
    }
  }

  offsetX -= (buttonRadX+1)*2;
  // Go up one directory level if not at root
  if (doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY,  0,
        AwesomeF100_20, 
        (char)18,
        primaryColor,
        (strlen(fileNameBuff)>1)?accentsColor:averageColor,
        sm)             &&
      screenWasPressed  &&
      !selectionMade    ){

    // Shave off top directory from path
    uint8_t curDirInd = 0,
            prevFNlen = strlen(fileNameBuff);
    for (uint8_t i = 1; i < prevFNlen; i++)
      if (fileNameBuff[i] == '/' && i != prevFNlen-1 ) curDirInd = i;
    for (uint8_t i = curDirInd+1; i < prevFNlen; i++)
      fileNameBuff[i] = '\0';

    // Enter (Sub)Folder, rebuild File List
    FsFile tmpFile = sm->sd->open((const char *)fileNameBuff);
    numFiles = getDirectoryContents(tmpFile, &fileListBuff);
    alphabetize(fileListBuff, numFiles);
    lowerLim = 0;
    upperLim = min(min(MAX_FILELIST_LENGTH, numFiles), numListings);
    tmpFile.close();
    selectedFile = 0;
  }
  
  offsetX -= (buttonRadX+1)*2;
  // Return to root directory
  if (doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY,  0,
        AwesomeF100_20, 
        (char)34,
        primaryColor,
        (strlen(fileNameBuff)>1)?accentsColor:averageColor,
        sm)             &&
      screenWasPressed  &&
      !selectionMade    ){
    selectedFile  = 0;
    numFiles      = -1;
    lowerLim      = 0;
    upperLim      = numListings;
    memset(fileNameBuff, '\0', MAX_FILEPATH_LENGTH);
    strcpy(fileNameBuff, "/");
  }

  offsetX -= (buttonRadX+1)*2;
  // Cancel file open (reset vars)
  if (doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY, -2,
        AwesomeF000_24, 
        (char)87,
        primaryColor,
        accentsColor,
        sm)             &&
      screenWasReleased &&
      !selectionMade    ){
    selectedFile  = 0;
    numFiles      = -1;
    lowerLim      = 0;
    upperLim      = numListings;
    memset(fileNameBuff, '\0', MAX_FILEPATH_LENGTH);
    strcpy(fileNameBuff, "/");
    memset(filePath, '\0', MAX_FILEPATH_LENGTH);
    selectionMade = true;
  }

  // reset vars if user exits/returns to home screen via home button
  if (  screenWasPressed                    &&
        //TouchX <= 2*buttonRadX              &&
        TouchX <= 2*28                      &&
        TouchX >= 0                         &&
        //TouchY <= DISP_HEIGHT-2*buttonRadY  &&
        TouchY <= DISP_HEIGHT-2*24          &&
        TouchY >= DISP_HEIGHT               ){
    selectedFile  = 0;
    numFiles      = -1;
    lowerLim      = 0;
    upperLim      = numListings;
    memset(fileNameBuff, '\0', MAX_FILEPATH_LENGTH);
    strcpy(fileNameBuff, "/");
  }

  if (selectionMade){
    sm->enableTouchInput(); 
    sm->enableDrawing();
    //sm->resetTouch();
    dialogueEntered = false;
  }
  else 
  {
    sm->disableTouchInput();
    sm->tft->updateScreen();
  }

  return selectionMade;
};

/*
 * Receives a directory and pointer to pointer to array of pointers
 * to fill in an array of strings representing file names
 * Returns number of files/folders in directory
 */
FLASHMEM uint32_t getDirectoryContents(FsFile dir, char*** filelist){

  uint32_t numFiles   = 0;
  uint32_t fileIndex  = 0;

  // Determine number of files/folders in directory
  while (true){
    FsFile entry = dir.openNextFile();
    if (entry) numFiles++; else break;
  }

  dir.rewind();

  // Iterate through directory files, exit when out of files
  while (fileIndex < MAX_FILELIST_LENGTH){
    FsFile entry = dir.openNextFile();

    // If entry valid, add filename to filelist, otherwise, break
    if (entry){
      // Make temporary buffer for getting file name
      size_t nameSize = 0;
      char nameBuff[MAX_FILENAME_LENGTH] = {'\0'};
      nameSize = entry.getName((char *)&nameBuff, MAX_FILENAME_LENGTH);
      
      // Append Slash to indicate directory
      if (entry.isDirectory()) nameBuff[strlen(nameBuff)] = '/';

      memset(
            (* filelist)[fileIndex], 
            '\0', 
            MAX_FILENAME_LENGTH
            );
      memcpy(
            (* filelist)[fileIndex], 
            nameBuff, 
            min(MAX_FILENAME_LENGTH, strlen(nameBuff))
            );
      
      fileIndex++;
    } else break;
  }

  return numFiles;
};

FLASHMEM uint8_t doMultiOptMessage(
      const char          Symbol,
      ILI9341_t3_font_t   SymbolFont,
      const char*         Title,
      const uint8_t       TitleLen,
            char*         BodyText,
      const uint16_t      BodyLen,
      const uint8_t       numButtons,
      const char**        buttonTexts,
      const uint8_t       buttonTextLens,
      const uint16_t*     buttonColors,
      ILI9341_t3_font_t*  buttonFonts,
      stateMachine*       sm
    ){
  sm->tft->writeRect(0, 0, DISP_WIDTH, DISP_HEIGHT, sm->getBackBuffer());
  //dialogueEntered = true;
  sm->enableTouchInput();
  sm->disableDrawing();

  // Used for drawing individual glyphs
  char iconGlyph[3] = {'\0'};

  // Used for file/directory navigation
  static  uint16_t  selectedFile  = 0;
  static  int16_t   numFiles      = -1;

  // UI element position place-holders
  uint16_t          offsetX       = 24,
                    offsetY       = 24;

  // Used for drawing a finite range of listings
  const   uint8_t   numListings   = 5;
  static  uint16_t  lowerLim      = 0;
  static  uint16_t  upperLim      = numListings;

  // Convenience placeholders
  const   uint16_t  accentsColor  = sm->getDetailsColor();
  const   uint16_t  primaryColor  = sm->getPrimaryColor();
  const   uint16_t  averageColor  = sm->getAverageColor();
  const   uint16_t  TouchX        = sm->getTouchX();
  const   uint16_t  TouchY        = sm->getTouchY();

  // Size of UI buttons
  const   uint8_t   buttonRadX    = 32;
  const   uint8_t   buttonRadY    = 20;

  const   bool      screenWasPressed      = sm->screenWasPressed();
  const   bool      screenWasLongPressed  = sm->screenWasLongPressed();
  const   bool      screenWasReleased     = sm->screenWasReleased();
  const   bool      repeatAction          = sm->getRepeatAction();

  // Draw Backrop w/border
  sm->tft->fillRect(
      offsetX,
      offsetY,
      DISP_WIDTH-offsetX*2,
      DISP_HEIGHT-offsetY*3,
      primaryColor
      );
  offsetX += 3;
  offsetY += 3;
  sm->tft->drawRoundRect(
      offsetX,
      offsetY,
      DISP_WIDTH-offsetX*2,
      DISP_HEIGHT-offsetY*3+3,
      4,
      accentsColor
      );

  // Draw Dividing lines
  sm->tft->drawLine(
      offsetX, 
      offsetY+3*offsetY/4, 
      DISP_WIDTH-offsetX-1, 
      offsetY+3*offsetY/4,  
      accentsColor
      );
  // Draw dialogue title text
  sm->tft->setTextDatum(TL_DATUM);
  sm->tft->setFont(SymbolFont);
  const char tmc[] = {Symbol};
  sm->tft->drawString(tmc, 1, offsetX+4, offsetY+2);
  sm->tft->setFont(Arial_12);
  sm->tft->drawString(Title, (uint16_t)TitleLen, offsetX+24, offsetY+4);

  static uint16_t startLineInd  = 0;      // Index of where to start drawing
  drawTextField(
      offsetX,
      offsetY,
      0, 0,
      BodyText,
      BodyLen,
      Arial_12,
      startLineInd,
      sm);

  offsetX = 286;
  offsetY = 180 - buttonRadY*3;
  
  if(doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY, 0,
        AwesomeF000_20, 
        (char)120,  // Down arrow
        primaryColor,
        accentsColor,
        sm)){
        //sm)             &&
      //screenWasPressed  ){
    if (screenWasPressed || repeatAction)
      startLineInd = findNext(BodyText, BodyLen, startLineInd, '\n');
  }

  offsetY -= buttonRadY<<1;
  if(doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY, 0,
        AwesomeF000_20, 
        (char)119,  // Up arrow
        primaryColor,
        accentsColor,
        sm)){
        //sm)             &&
      //screenWasPressed  ){
    if (screenWasPressed || repeatAction)
      startLineInd = findPrev(BodyText, BodyLen, startLineInd, '\n');
  }

  offsetX = 286;
  offsetY = 180;
  char tms[32] = {'\0'};
  for (uint8_t i = 0; i < numButtons; i ++) {
    strcpy(tms, buttonTexts[i]);
    if (doTextButton(
          offsetX, offsetY,
          buttonRadX, buttonRadY,
          tms,
          buttonFonts[i], 
          primaryColor,
          buttonColors[i],
          sm)             &&
        screenWasReleased ){
#ifdef DEBUG
      Serial.print("Dialogue Button Pressed: ");
      Serial.println(i+1);
#endif
      return i+1;
    }
    memset(tms, '\0', 32);
    offsetX -= (buttonRadX+2)*2;
  }

  sm->tft->updateScreen();
  return 0;
};

// Finds the index of next byte matching char,
// returns input index if no match is found
const uint16_t findNext(
    const char*     text,
    const uint16_t  textLen,
    const uint16_t  currIndex,
    const char      searchTarget
    ){
#ifdef DEBUG
  Serial.print("Look for next \"");
  Serial.print(searchTarget);
  Serial.print("\", uint8_t: ");
  Serial.print((uint8_t)searchTarget);
  Serial.println();
#endif
  uint16_t i = currIndex;
  if (text[i] == searchTarget) i++;
  for (i = i; i < textLen-1; i++){
#ifdef DEBUG
    Serial.print("Scanning char: ");
    Serial.print(text[i]);
    Serial.print(", uint8_t: ");
    Serial.print((uint8_t)text[i]);
    Serial.print(", (index): ");
    Serial.print(i);
    Serial.println();
#endif
    if (text[i] == searchTarget){
#ifdef DEBUG
      Serial.print("Found next char at index: ");
      Serial.print(i);
      Serial.println();
#endif
      return i;
    }
  }
#ifdef DEBUG
  Serial.print("NO next char found, index: ");
  Serial.print(currIndex);
  Serial.println();
#endif
  return currIndex;
};

// Finds the index of prev byte matching char,
// returns 0 if no match is found
const uint16_t findPrev(
    const char*     text,
    const uint16_t  textLen,
    const uint16_t  currIndex,
    const char      searchTarget
    ){
  if (currIndex >= textLen || currIndex == 0){
    return currIndex;
  }
#ifdef DEBUG
  Serial.print("Look for prev \"");
  Serial.print(searchTarget);
  Serial.print("\", uint8_t: ");
  Serial.print((uint8_t)searchTarget);
  Serial.println();
#endif

  for (uint16_t i = currIndex-1; i >= 0; i--){
    if (i == 0) Serial.println("Search has reached start of array");
    if (text[i] == searchTarget || i == 0){
#ifdef DEBUG
      Serial.print("Found Prev char at index: ");
      Serial.print(i);
      Serial.println();
#endif
      return i;
    }
  }
#ifdef DEBUG
  Serial.print("NO prev char found, index: ");
  Serial.print(currIndex);
  Serial.println();
#endif
  return currIndex;
};

const uint8_t drawTextField(
          uint16_t    posX, 
          uint16_t    posY,
    const uint16_t    fieldWidth,
    const uint16_t    fieldHeight,
          char*       text,
    const uint16_t    textLen,
    ILI9341_t3_font_t textFont,
    const uint16_t    currIndex,
    stateMachine*     sm){

  // Text Area Vars
  uint16_t        lastLineBreak = 0;
  uint16_t        lastSpace     = 0;
  char            subBuff[256]  = {'\0'};

  // Text Boundary place holders
  int16_t       x1=0, y1=0;
  uint16_t      w=0, h=0, h1=0;
  const char heightTest[] = {"jgypq,QW@#|({[`;'_"};

  // Determine max font character height
  sm->tft->getTextBounds(
      heightTest, 18,
      0, 0,
      //posX+24, posY+24,
      &x1, &y1,
      &w, &h1);
  sm->tft->setTextDatum(TL_DATUM);
  sm->tft->setFont(textFont);

  // Replace spaces with line breaks where line length has exceeded text box width
  uint16_t numLineBreaks = 0;
  for (uint16_t i = 0; i < textLen; i++) {
    subBuff[i-lastLineBreak] = text[i];
    sm->tft->getTextBounds(
        subBuff, i-lastLineBreak, 
        8, 28,
        &x1, &y1,
        &w, &h);

    // Length of line has exceeded width of text box
    if (w >= DISP_WIDTH-((posX+24)<<1) || i == textLen-1){
      if (i == textLen-1) i = textLen;
      else i = lastSpace;           // retrograde i to start of last word

      text[i] = '\n';
      numLineBreaks++;
      memset(subBuff, '\0', i-lastLineBreak);   // reset subBuff
      lastLineBreak = lastSpace+1;  // replace last space with a line break (start of last word)
    } 
    if (text[i] == ' ') lastSpace = i;
  }

  // Draw Text with line breaks
  lastLineBreak = currIndex;
  for (uint16_t i = currIndex; i < textLen; i++){
    subBuff[i-lastLineBreak] = text[i];
    if ((text[i] == '\n' && i != currIndex) || i == textLen-1){
      if (i == textLen-1) i = textLen;

      if (posY+28 <= DISP_HEIGHT-100) {
        sm->tft->drawString(subBuff, i-lastLineBreak, posX+8, posY+28);
        posY += h1;
      }

      memset(subBuff, '\0', i-lastLineBreak);   // reset subBuff
      lastLineBreak = i+1;
    } 
  }

  return 0;
};
