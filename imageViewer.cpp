#include "imageViewer.h"
extern uint16_t wallpaper[];
void loadBmp(const char *filename, uint16_t *imgBuffer, stateMachine* sm);

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.
uint16_t read16(FsFile &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(FsFile &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

// Determines whether the filepath leads to a proper bmp file
bool imageValid(const char* filePath, stateMachine* sm){
  FsFile bmpFile;
  uint8_t bmpDepth;

  if (!(bmpFile = sm->sd->open(filePath)))
    return false;

  if (read16(bmpFile) == 0x4D42) {
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
        bmpFile.close();
        return true;
      }
    }
  } else {
    bmpFile.close();
    return false;
  }
}

void doImageViewer(stateMachine* sm){
  char            Title[]           = "Selecting an Image";
  uint8_t         TitleLen          = 18;
  const char      helpMessage[]     = "Use the file browser to select an image file (.bmp supported only). After an image has been opened, tap the screen to select different file.";
  const uint16_t  helpMessageLen    = 140;
  const char*     options[]         = {"Ok"};
  uint8_t         optionLengths[]   = {3};
  ILI9341_t3_font_t optionFonts[]   = {Arial_18};
  /*
  const char      helpMessage[]     = "Use the file browser to select an image file (.bmp supported only). After an image has been opened, tap the screen to select another one. The quick brown fox jumps over the lazy dog. Voluptas quia dolorum velit hic illo ut. Animi impedit quis temporibus eaque voluptas molestiae. Id est eum rerum officia ipsum unde. Tempora praesentium veniam animi nostrum labore nisi quia voluptatem. Et id ex delectus culpa totam deleniti. Deleniti consequuntur et eligendi.";
  const uint16_t  helpMessageLen    = 461;
  const char*     options[]         = {"Ok", "Direc\nHeck", "Top\nQuack", "I\nUnderstand\n(proceed)"};
  uint8_t         optionLengths[]   = {3, 3, 3, 3};
  ILI9341_t3_font_t optionFonts[]   = {Arial_18, Arial_12, Arial_12, Arial_8};
  */
  static bool     selectionMade     = false;
  static bool     imageSelected     = false;
  static uint8_t  helpMessageAck    = 0;

	const		bool			screenWasPressed			= sm->screenWasPressed();
	const		bool			screenWasLongPressed	= sm->screenWasLongPressed();
	const		bool			screenWasReleased			= sm->screenWasReleased();
  
  // Convenience placeholders
  const uint16_t  accentsColor      = sm->getDetailsColor();
  const uint16_t  primaryColor      = sm->getPrimaryColor();
  const uint16_t  averageColor      = sm->getAverageColor();
  const uint16_t  accentsColorI     = sm->getDetailsColorInverted();
  const uint16_t  primaryColorI     = sm->getPrimaryColorInverted();
  const uint16_t  TouchX            = sm->getTouchX();
  const uint16_t  TouchY            = sm->getTouchY();
  const uint16_t  optionColors[]    = {accentsColor, accentsColor, accentsColor, accentsColor};

  // Inform user how to display an image
  if (helpMessageAck == 0) {
    helpMessageAck = doMultiOptMessage(
        (char)90,      // i in a circle ~ information symbol
        AwesomeF000_12,
        (char *)Title,
        TitleLen,
        helpMessage,
        helpMessageLen,
        1,
        options,
        optionLengths,
        optionColors,
        optionFonts,
        sm
        );
    if (//helpMessageAck == 1 ||
        helpMessageAck == 2 ||
        helpMessageAck == 3 || 
        helpMessageAck == 4){
			Serial.print("Button Pressed: ");
			Serial.println(helpMessageAck);
      helpMessageAck = 0;
    }
    //if (helpMessageAck) helpMessageAck = 0;
    return;
  }

  if (selectionMade) {
    // User canceled file-open, return to home screen
    if (strlen((char *)&sm->filePathBuff) == 0) {
      selectionMade = false;
      imageSelected = false;
      //sm->setScreen(SCREEN_HOME);
      sm->setScreen(1);
    } 
    // User selected a file path to image
    else {
      if (!imageSelected) { // only load image once
        //loadBmp((char *)&sm->filePathBuff, wallpaper, sm);
        loadBmp((char *)&sm->filePathBuff, sm->userBackground, sm);
        
        imageSelected = true;
      } else {  // draw loaded image
        sm->tft->writeRect(0, 0, DISP_WIDTH, DISP_HEIGHT, sm->userBackground);
      }

      // Exit on touch
      if (screenWasPressed) {
        selectionMade = false;
        imageSelected = false;
      }
    }
  } else {
    memset(Title, '\0', 18);
    strcpy("Select Image File:", Title);
    selectionMade = doFileOpen(
        (char*)Title, 
        TitleLen, 
        (char *)&sm->filePathBuff, 
        imageValid,
        sm
        );
  }

  // Draw crosshair
  //if (sm->getCurrTouch() && sm->getPrevTouch()) {
    //sm->tft->drawFastHLine(0, TouchY, (uint16_t)DISP_WIDTH, accentsColor);
    //sm->tft->drawFastVLine(TouchX, 0, (uint16_t)DISP_HEIGHT, accentsColor);
  //}

  // reset vars if user exits/returns to home screen via home button
  if (  screenWasPressed                    &&
        //TouchX <= 2*buttonRadX              &&
        TouchX <= 2*28                      &&
        TouchX >= 0                         &&
        //TouchY <= DISP_HEIGHT-2*buttonRadY  &&
        TouchY <= DISP_HEIGHT-2*24          &&
        TouchY >= DISP_HEIGHT               ){
    selectionMade     = false;
    imageSelected     = false;
  }

  return;
}

// This function opens a Windows Bitmap (BMP) file to a buffer
#define BUFFPIXEL 80
void loadBmp(const char *filename, uint16_t *imgBuffer, stateMachine* sm){
  uint16_t  x=0,y=0;
  FsFile    bmpFile;
  int       bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t   bmpDepth;              // Bit depth (currently must be 24)
  uint32_t  bmpImageoffset;        // Start of image data in file
  uint32_t  rowSize;               // Not always = bmpWidth; may have padding
  uint8_t   sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint16_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean   goodBmp = false;       // Set to true on valid header parse
  boolean   flip    = true;        // BMP is stored bottom-to-top
  int       w, h, row, col;
  uint8_t   r, g, b;
  uint32_t  pos = 0, startTime = millis();

  if((x >= DISP_WIDTH) || (y >= DISP_HEIGHT)) return;

  // Open requested file on SD card
  if (!(bmpFile = sm->sd->open(filename))) {
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    read32(bmpFile);
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    // Read DIB header
    read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= DISP_WIDTH)  w = DISP_WIDTH  - x;
        if((y+h-1) >= DISP_HEIGHT) h = DISP_HEIGHT - y;

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            imgBuffer[row*w+col] = sm->tft->color565(r,g,b);
          } // end pixel
        } // end scanline
      } // end goodBmp
    }
  }

  bmpFile.close();
  return;
}
