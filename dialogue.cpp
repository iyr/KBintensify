#include <stdint.h>
#include <t3nfonts.h>

#include "dialogue.h"
#include "alphabetize.h"
#include "pgmspace.h"

extern char   fileNameBuff[];
extern char** fileListBuff;

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
  sm->tft->drawString1(iconGlyph, 3, offsetX, offsetY);

  sm->tft->setFont(AwesomeF080_12);
  iconGlyph[0] = (char)90;  // Right-pointing arrow glyph
  sm->tft->drawString1(iconGlyph, 3, offsetX+18, offsetY);

  // Draw File dialogue text
  sm->tft->setFont(Arial_12);
  sm->tft->drawString1(Title, (uint16_t)TitleLen+2, offsetX+26, offsetY);

  if (numFiles < 0) { //  avoid unnecessary recomputation
    // Set directory to open
    memset(fileNameBuff, '\0', MAX_FILEPATH_LENGTH);
    strcpy(fileNameBuff, "/");
    File tmpFile = sm->sd->open((const char *)fileNameBuff);
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
  sm->tft->drawString1(printBuff, ti+i+3, DISP_WIDTH-offsetX, offsetY);

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
    sm->tft->drawString1(iconGlyph, 3, offsetX, offsetY-2);
    sm->tft->setFont(LiberationMono_11);
    if (i < numFiles)
      sm->tft->drawString1(
            fileListBuff[i], 
            min(23, strlen(fileListBuff[i])+2),
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
  sm->tft->drawString1(
      iconGlyph, 
      3,
      offsetX+4, 
      DISP_HEIGHT-11*offsetY/4+6
      );

  sm->tft->setFont(LiberationMono_8);
  sm->tft->drawString1(
      fileNameBuff,
      (uint16_t)strlen(fileNameBuff)+2,
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
        sm)){
    if (selectedFile <= 0)  // wrap selection to end if at top
      selectedFile = numFiles-1;
    else 
      selectedFile -= 1;

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
        sm)){
    if (selectedFile+1 >= numFiles) // wrap selection to top if at bottom
      selectedFile = 0;
    else 
      selectedFile += 1;

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
        sm)
      && 
      fileIsValid  ){
    strncat(fileNameBuff, fileListBuff[selectedFile], MAX_FILEPATH_LENGTH);
    memcpy(filePath, fileNameBuff, MAX_FILEPATH_LENGTH);

    // Shave off top file from path
    uint8_t curDirInd = 0,
            prevFNlen = strlen(fileNameBuff);
    for (uint8_t i = 1; i < prevFNlen; i++)
      if (fileNameBuff[i] == '/' && i != prevFNlen-1 ) curDirInd = i;
    for (uint8_t i = curDirInd+1; i < prevFNlen; i++)
      fileNameBuff[i] = '\0';
    //return true;
    selectionMade = true;
  }

  offsetX -= (buttonRadX+1)*2;
  // Enter directory
  if (numFiles > 0  
      &&
      doIconButton(
        offsetX, offsetY,
        buttonRadX, buttonRadY,  0,
        AwesomeF100_20, 
        (char)21,
        primaryColor,
        (lastChar == '/')?accentsColor:averageColor,
        sm)         
      &&
      !selectionMade){

    // Enter (Sub)Folder, rebuild File List
    if (lastChar == '/') {
      strncat(fileNameBuff, fileListBuff[selectedFile], MAX_FILEPATH_LENGTH);
      File tmpFile = sm->sd->open((const char *)fileNameBuff);
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
        sm)         
      &&
      !selectionMade){

    // Shave off top directory from path
    uint8_t curDirInd = 0,
            prevFNlen = strlen(fileNameBuff);
    for (uint8_t i = 1; i < prevFNlen; i++)
      if (fileNameBuff[i] == '/' && i != prevFNlen-1 ) curDirInd = i;
    for (uint8_t i = curDirInd+1; i < prevFNlen; i++)
      fileNameBuff[i] = '\0';

    // Enter (Sub)Folder, rebuild File List
    File tmpFile = sm->sd->open((const char *)fileNameBuff);
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
        sm)         
      &&
      !selectionMade){
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
        sm)         
      &&
      !selectionMade){
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
  if (  sm->getPrevTouch() != sm->getCurrTouch()  &&
        sm->getCurrTouch() == false          ){
    if (  TouchX <= 2*buttonRadX             &&
          TouchX >= 0                        &&
          TouchY <= DISP_HEIGHT-2*buttonRadY &&
          TouchY >= DISP_HEIGHT              ){
      selectedFile  = 0;
      numFiles      = -1;
      lowerLim      = 0;
      upperLim      = numListings;
      memset(fileNameBuff, '\0', MAX_FILEPATH_LENGTH);
      strcpy(fileNameBuff, "/");
    }
  }

  return selectionMade;
}

/*
 * Receives a directory and pointer to pointer to array of pointers
 * to fill in an array of strings representing file names
 * Returns number of files/folders in directory
 */
FLASHMEM uint32_t getDirectoryContents(File dir, char*** filelist){

  uint32_t numFiles   = 0;
  uint32_t fileIndex  = 0;

  // Determine number of files/folders in directory
  while (true){
    File entry = dir.openNextFile();
    if (entry) numFiles++; else break;
  }

  dir.rewind();

  // Iterate through directory files, exit when out of files
  while (fileIndex < MAX_FILELIST_LENGTH){
    File entry = dir.openNextFile();

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
}
